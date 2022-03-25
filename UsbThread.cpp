#include "UsbThread.h"
#include "ChecksumValidator.h"
#include <QTextStream>
#include <qdebug.h>
#include <iomanip>
#include <QMutex>
#include <qexception.h>


using namespace std;

//FIRMWARE UPDATE PROCESS
void Worker::update()
{
    //DEACTIVATE ALL BUTTONS DURING THREAD
    emit deactivateButtons();
    emit deactivateAbortButton();
    //GUI...
    emit setLabel("initializing....     ");
    emit ProgressBar_valueChanged(0);
    //THREAD SAFETY
    QMutex mutex;
    mutex.lock(); 
   
    //OPEN USB PORT
    SerialHandler usb_port_update_thread;
    usb_port_update_thread.find_lordyphon_port();  //hotplug timer deactivated in threads, check if lordyphon is still connected
    usb_port_update_thread.open_lordyphon_port();
    usb_port_update_thread.lordyphon_handshake();
    
    //SET BUFFER FOR "OK" or "ER" + '\0' CHARACTER
    usb_port_update_thread.set_buffer_size(3);
   
    //SET VARIABLES
    size_t index = 0;
    int checksum_error_ctr = 0;
    int rx_error_ctr = 0;
    bool carry_on_flag = true;  //START CONDITION TO GET FIRST RECORD
    QByteArray tx_data;
    ready_read_timeout_ctr = 0;
    
    //CREATE TIMESTAMP & OPEN LOGFILE
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("dd_MM_yyyy___h_m_s");
    QFile firmware_transfer_logfile;
    firmware_transfer_logfile.setFileName(QDir::homePath() + "/LordyLink/log/firmware_transfer_log_" + timestamp + ".txt");
    firmware_transfer_logfile.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&firmware_transfer_logfile);
   
    //CREATE PARSER OBJECT
    Parser hex_parser(selected_firmware);
    
    emit setLabel("validating hexfile     ");
    
    //PARSE DOWNLOADED HEXFILE
    if (hex_parser.parse_hex()) {  //BOOL METHOD, PARSER IS RESPONSIBLE FOR VALIDATING HEXFILE
                                   //IF TRUE, HEXFILE IS VALID
        int hexfilesize = hex_parser.get_hexfile_size();   //GET SIZE FOR PROGRESS BAR AND LOOP

        if (hexfilesize == 0) {    //DOUBLE CHECK
            emit setLabel("error: no file found");
            carry_on_flag = false;
        }
        emit ProgressBar_setMax(hexfilesize);  //SET PROGRESSBAR MAXIMUM
        delay(1000);            
        
        //THIS LOOP SENDS PARSED HEX RECORDS TO MICROCONTROLLER AND WAITS FOR CHECKSUM CONFIRMATION BEFORE SENDING THE
        //NEXT RECORD: IF CALCULATED CHECKSUM ON CONTROLLER IS "ok",  "index" IS INCREMENTED
        while (index < hexfilesize && hexfilesize != 0) { 

            if (QThread::currentThread()->isInterruptionRequested()) {
                emit setLabel("aborting....");
                delay(1000);
                firmware_transfer_logfile.close();
                firmware_transfer_logfile.remove();
                usb_port_update_thread.write_serial_data(call_lordyphon.abort);
                usb_port_update_thread.close_usb_port();
                emit activateButtons();
                mutex.unlock();
                emit finished();
                return;
            }
            if (carry_on_flag == true) {  //INITIAL STATE == TRUE
                tx_data = hex_parser.get_hex_record(index);  //QByteArray txdata IS USED TO SEND DATA TO THE MICROCONTROLLER
                usb_port_update_thread.write_serial_data(tx_data); //SEND RECORD TO USB
                out << "record nr. " << index << "  " << (tx_data.toHex()) << '\n';  //LOGFILE OUTPUT
            }
            if (!usb_port_update_thread.wait_for_ready_read(1000)) //THREAD BLOCKS UNTIL INCOMING CONFIRMATION MESSAGE OR TIMEOUT 1000ms
                ready_read_timeout_ctr++;  //THIS IS USED INSTEAD OF THE HOTPLUG TIMER 
            
            QString checksum_status = usb_port_update_thread.getInputBuffer(); //CATCH INCOMING CONFIRMATION MESSAGE

            if (checksum_status == lordyphon_response.is_checksum_error) {  //CHECKSUM CALCULATION HAS GONE WRONG
                ++checksum_error_ctr;                 //EXIT CONDITION, IF HEXFILE IS CORRUPTED, CTR WILL GO UP TO 8 BEFORE ABORTING
                out << "--------------CHECKSUM ERROR AT INDEX " << index << "  " << (tx_data.toHex()) << '\n'; //LOGFILE OUTPUT
                emit setLabel("checksum error...sending record again");
                carry_on_flag = true;              //READ RECORD AGAIN AT SAME VECTOR INDEX
                delay(1000);                 //ALLOW TIME FOR USER FEEDBACK
            }
            else if (checksum_status == lordyphon_response.checksum_ok) {      //CHECKSUM IS CORRECT   
                emit ProgressBar_valueChanged(static_cast<int>(index));        //UPDATE PROGRESS BAR
                emit setLabel("sending file ");
                rx_error_ctr = 0;
                carry_on_flag = true;  //ALLOW RECORD READ
                ++index;               //FROM NEXT VECTOR INDEX
            }
            else {
                // INCOMING CONFIRMATION MESSAGE IS CORRUPTED
                usb_port_update_thread.write_serial_data(call_lordyphon.get_checksum_status);     //ASK AGAIN FOR CHECKSUM STATUS, CONTROLLER WILL SEND EITHER "ok" or "er"
                //qDebug() << checksum_status;
                emit setLabel("rx error, checking status... ");
                out << "---------------RX ERROR AT INDEX---------------------- " << index << '\n'; //LOGFILE
                ++rx_error_ctr;         //IF MESSAGE ISNT READABLE FOR MORE THAN 8 TIMES, CONNECTION MUST BE LOST
                delay(1000);
                carry_on_flag = false;
            }
            if (checksum_error_ctr > 8) {  //EXIT CONDITION: CHECKSUM ERROR
                emit setLabel("file corrupted...");
                carry_on_flag = false;
                break;
            }
            if (rx_error_ctr > 8 || ready_read_timeout_ctr > 8) {  //EXIT CONDITION: CONNECTION ERROR
                emit setLabel("rx error, check connection ");
                carry_on_flag = false;
                break;
            }
        }
        
        delay(1000); //THIS IS NECESSARY FOR THE NEXT MESSAGE TO BE RECEIVED CORRECTLY
        firmware_transfer_logfile.close(); // LOGFILE DONE
        emit ProgressBar_valueChanged(hexfilesize); //SET PROGRESSBAR TO MAX
      
        //DATA TRANSFER COMPLETE HERE, BURN FLASH ON MICROCONTROLLER
        if (carry_on_flag == true) {   
            emit setLabel("transfer complete");
            usb_port_update_thread.write_serial_data(call_lordyphon.hexfile_send_complete); //CALLS BURN FUNCTION ON LORDYPHON
            delay(1000);
            emit setLabel("burning flash, don't turn off");
            delay(18000);       //NO PROGRESS BAR ANIMATION DURING FLASH BURN
            emit setLabel("restart lordyphon");
            delay(2000);
        }
        else
            emit setLabel("transfer aborted");
        
        usb_port_update_thread.close_usb_port();
    }//end: if (hex_parser.parse_hex())
    else {
        emit remoteMessageBox("no file found, check internet connection");  
        emit ProgressBar_valueChanged(0);
    }
   
    emit activateButtons();
    mutex.unlock();
    emit finished();  //THREAD CLOSED
}


//GET SET METHOD, READS ALL 128kB FROM LORDYPHON-EEPROM AT ONCE, 
//CHECKSUM IS CALCULATED AFTER LAST BYTE IS READ.
void Worker::get_eeprom_content(){
    
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();

    SerialHandler usb_port_get_thread;

    //CREATE DATA CONTAINER
    QByteArray eeprom;

    ready_read_timeout_ctr = 0;
    
    //CHECK IF LORDYPHON IS STILL CONNECTED
    if (!usb_port_get_thread.find_lordyphon_port()) {
        emit setLabel("lordyphon disconnected!");
        mutex.unlock();
        emit activateButtons();
        emit finished();
    }
    //OPEN USB PORT
    if (!usb_port_get_thread.lordyphon_port_is_open())
        usb_port_get_thread.open_lordyphon_port();
    
    if (!usb_port_get_thread.lordyphon_handshake()){
        emit setLabel("connection error");
        emit activateButtons();
        mutex.unlock();
        emit finished();
    }
    
    //PREPARE LORDYPHON FOR DATA TRANSFER
    usb_port_get_thread.set_buffer_size(5);
    usb_port_get_thread.write_serial_data(call_lordyphon.transfer_request);
    //SET BUFFER FOR EXPECTED RESPONSE SIZE
    usb_port_get_thread.wait_for_ready_read(2000); //allow up to 2 sec to confirm

    QString response = usb_port_get_thread.getInputBuffer();
   

    if (response == lordyphon_response.play_mode_is_on) {
        
        emit remoteMessageBox("lordyphon memory busy, press stop button");
        usb_port_get_thread.clear_buffer();
        usb_port_get_thread.close_usb_port();
        emit activateButtons();
        mutex.unlock();
        emit finished();
        return;


    }
    else if (response == lordyphon_response.play_mode_is_off) {


        //CREATE TIMESTAMP
        QDateTime now = QDateTime::currentDateTime();
        QString timestamp = now.toString("dd_MM_yyyy___h_m_s");
        //ADD TIMESTAMP TO PATH AND FILE NAME
        QString new_filepath_qstring = QDir::homePath() + "/LordyLink/Sets/saved_set_" + timestamp + ".txt";
        QString new_filename = "saved_set_" + timestamp + ".txt";

        //OPEN NEW FILE WITH TIMESTAMP IN NAME
        ofstream set_file;
        set_file.open(new_filepath_qstring.toStdString());

        if (set_file.is_open()) {

            size_t progress_bar_ctr = 0;
            size_t rec_ctr = 0;

            //GUI FEEDBACK
            emit setLabel("reading set data");
            emit ProgressBar_setMax(128000);
            //SEND DUMP REQUEST TO LORDYPHON
            usb_port_get_thread.write_serial_data(call_lordyphon.dump_request);
            delay(10);
            usb_port_get_thread.set_buffer_size(1000);
            usb_port_get_thread.clear_buffer();

            //READ LORDYPHON DATA IN LOOP, ADD INPUT BUFFER TO eeprom QByteArray
            while (eeprom.size() < 128000) {  //128kB TOTAL EEPROM SIZE

                //clean exit if user aborts...
                if (QThread::currentThread()->isInterruptionRequested()) {
                    emit setLabel("aborting....");
                    delay(1000);
                    set_file.close();
                    usb_port_get_thread.write_serial_data(call_lordyphon.abort);
                    delay(200);
                    usb_port_get_thread.write_serial_data(call_lordyphon.abort);  //sending twice works better...
                    usb_port_get_thread.close_usb_port();
                    QDir sets = new_filepath_qstring;
                    sets.remove(new_filepath_qstring);
                    emit activateButtons();
                    mutex.unlock();
                    emit finished();
                    return;
                }
                if (ready_read_timeout_ctr > 0)  //DEBUGGING
                    qDebug() << "timeout counter: " << ready_read_timeout_ctr;

                if (!usb_port_get_thread.wait_for_ready_read(300)) //HOT PLUG DETECTION INSTEAD OF TIMER IN THREAD
                    ready_read_timeout_ctr++;

                if (ready_read_timeout_ctr > 5) {
                    emit remoteMessageBox("connection error      ");
                    break;
                }
                eeprom += usb_port_get_thread.getInputBuffer();  //ADD RECEIVED DATA TO CONTAINER
                //UPDATE PROGRESS BAR
                emit ProgressBar_valueChanged(eeprom.size());
            }
            //CHECKSUM VERIFICATION, ALL BYTES ARE SUMMED UP INTO AN uint16_t ON BOTH SYSTEMS
            usb_port_get_thread.clear_buffer();
            usb_port_get_thread.set_buffer_size(2); //SET BUFFER FOR EXPECTED 16BIT CHECKSUM FROM LORDYPHON
            usb_port_get_thread.write_serial_data(call_lordyphon.request_checksum);
            usb_port_get_thread.wait_for_ready_read(2000);

            QByteArray checksum16bit_from_lordyphon = usb_port_get_thread.getInputBuffer();
            uint8_t lsb = checksum16bit_from_lordyphon.at(1);
            uint8_t msb = checksum16bit_from_lordyphon.at(0);

            uint16_t checksum_lordyphon = ((msb << 8) | lsb);  //ASSEMBLE 16BIT CHECKSUM

            ChecksumValidator checksum_checker;  //CREATE CHECKSUMVALIDATOR OBJECT
            checksum_checker.set_Data(eeprom, checksum_lordyphon);

            if (checksum_checker.is_valid()) {  //BOOL METHOD COMPARES BOTH CHECKSUMS
                emit setLabel("checksum ok ");
                delay(500);
                emit setLabel("writing file");
                delay(1000);

                for (int i = 0; i < eeprom.size(); ++i) {
                    //CREATE (HUMAN-)READABLE FORMAT FOR PARSER, 16 DATABYTES PER RECORD(LINE)
                    int temp = static_cast<unsigned char>(eeprom.at(i));

                    if (i != 0 && i % 16 == 0) {
                        set_file << endl;
                    }
                    set_file << setw(2) << setfill('0') << hex << temp;
                }
                set_file.close();
                //ADD NEW SET TO MODEL / QTableView
                emit newItem(new_filename);
                emit ProgressBar_valueChanged(128000);
                emit setLabel("done");
            }
            else {
                emit remoteMessageBox("checksum error, try again");
                set_file.close();
                //DELETE TEMP FILE
                QDir sets = new_filepath_qstring;
                sets.remove(new_filepath_qstring);
            }

        }//end:  if (set_file.is_open())
        else
            emit remoteMessageBox("file not found!");

        
    }//END: else if (response == lordyphon_response.play_mode_is_on)
    else {
        emit remoteMessageBox("communication error, please try again!   ");
        qDebug() << "error, message not recognized";


    }
    //END THREAD
    usb_port_get_thread.clear_buffer();
    usb_port_get_thread.close_usb_port();
    emit activateButtons();
    mutex.unlock();
    emit finished();
}


//SEND SET TO LORDYPHON METHOD
//COMMUNICATION LORDYLINK TO LORDYPHON ONLY WORKS PROPERLY IF DATA CHUNKS ARE RATHER SMALL,
//OTHERWISE THE MICROCONTROLLER GETS OVERWHELMED AND RESETS ITSELF. 
//THUS, I DECIDED TO SEND 16 BYTE CHUNKS AND HAVE TRANSMISSION CONFIRMED BEFORE CARRYING ON.
//THIS SEMI-SYNCHRONIZED TRANSFER WORKS BEST SO FAR.

void Worker::send_eeprom_content(){
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();

    SerialHandler usb_port_send_thread;
    ready_read_timeout_ctr = 0;
    
    emit setLabel("sending set");
    
    if (!usb_port_send_thread.find_lordyphon_port()) {
        emit remoteMessageBox("lordyphon disconnected!");
        emit activateButtons();
        mutex.unlock();
        emit finished();
    }
    
    if (!usb_port_send_thread.lordyphon_port_is_open())
        usb_port_send_thread.open_lordyphon_port();

    usb_port_send_thread.set_buffer_size(100);
    
    if(!usb_port_send_thread.clear_buffer())
        qDebug() <<"buffer not empty";  //DEBUGGING

    qDebug() << "in send set thread: " << selected_set;
    QString set_dir;
    set_dir = QDir::homePath() + "/LordyLink/Sets/";
    
    //HAND PATH TO PARSER
    Parser eeprom_parser(set_dir + selected_set);
  
    if (eeprom_parser.parse_eeprom()) {
        //GET SIZE 
        int setfilesize = eeprom_parser.get_eeprom_size();
        //GUI FEEDBACK
        emit ProgressBar_setMax(setfilesize);
        emit setLabel("sending set");
        
        //PREPARE LORDYPHON FOR INCOMING DATA
        usb_port_send_thread.set_buffer_size(5);
        usb_port_send_thread.write_serial_data(call_lordyphon.transfer_request);
        //SET BUFFER FOR EXPECTED RESPONSE SIZE
        usb_port_send_thread.wait_for_ready_read(2000); //allow up to 2 sec to confirm

        QString response = usb_port_send_thread.getInputBuffer();
        QByteArray temp_record;
        bool abortflag = false;
        
        if (response == lordyphon_response.play_mode_is_off) {  //LORDYPHON PLAY MODE IS OFF
            qDebug() << " lordyphon confirmes playmode off";
            size_t index = 0;
            delay(200);
            usb_port_send_thread.write_serial_data(call_lordyphon.begin_tansfer); 
            QByteArray checksum_vec;
           
            //SEND DATA
            while (index < setfilesize && setfilesize != 0) {
                //set clean exit if user aborts...
                if (QThread::currentThread()->isInterruptionRequested()) 
                    abortflag = true;

                temp_record = eeprom_parser.get_eeprom_record(index); //GET EEPROM RECORD (16 BYTES)
                checksum_vec += temp_record; // SUM UP ALL BYTES FOR CHECKSUM CALCULATION
                usb_port_send_thread.write_serial_data(temp_record);  //SEND RECORD TO LORDYPHON
                usb_port_send_thread.set_buffer_size(3);   //EXPECTED CONFIRMATION MESSAGE SIZE
                
                if (ready_read_timeout_ctr > 0)
                    qDebug() << "timeout counter: " << ready_read_timeout_ctr;
                
                if (!usb_port_send_thread.wait_for_ready_read(2000))
                    ready_read_timeout_ctr++;

                if (ready_read_timeout_ctr > 5) {
                    emit remoteMessageBox("connection error, try again");
                    emit setLabel("connection error");
                    for (int i = 0; i < 5; ++i)
                        usb_port_send_thread.write_serial_data(call_lordyphon.abort2);
                    break;
                }

                //ANY 3-BYTE MESSAGE CAN BE USED HERE, THIS IS JUST FOR SYNCHRONIZING
                usb_port_send_thread.getInputBuffer(); //DUMMY READ
               
                if (abortflag == false) {
                    QByteArray dummy;
                    dummy.append(0xff);
                    usb_port_send_thread.write_serial_data(dummy);
                }
                else {
                   for(int i = 0; i < 5; ++i)
                     usb_port_send_thread.write_serial_data(call_lordyphon.abort2);
                   
                   usb_port_send_thread.close_usb_port();
                   emit setLabel("aborting....");
                   delay(1000);
                  
                   emit activateButtons();
                   mutex.unlock();
                   emit finished();
                   return;
               }
                index++;
                emit ProgressBar_valueChanged(static_cast<int>(index));
            }//END: while(index < setfilesize && setfilesize != 0)
           
            usb_port_send_thread.clear_buffer();
            //EXPECTING LORDYPHON CHECKSUM
            delay(500);
            usb_port_send_thread.write_serial_data(call_lordyphon.request_checksum);
            emit setLabel("waiting for checksum");
            usb_port_send_thread.wait_for_ready_read(1000); 
            
            QByteArray checksum_from_lordyphon = usb_port_send_thread.getInputBuffer();
            uint8_t msb = checksum_from_lordyphon.at(0); //big endian
            uint8_t lsb = checksum_from_lordyphon.at(1);
            uint16_t checksum_lordyphon = (msb << 8) | lsb;  //assemble uint16_t checksum
            
            //SAME PROCEDURE AS IN GET SET THREAD
            ChecksumValidator checksum_checker;
            checksum_checker.set_Data(checksum_vec, checksum_lordyphon);

            if (checksum_checker.is_valid()) {
                emit setLabel("checksum ok  ");
                delay(1000);
                usb_port_send_thread.write_serial_data(call_lordyphon.burn_eeprom); //CHECKSUM CORRECT, CALL EEPROM BURN FUNCTION ON LORDAPHON
                emit setLabel("burning eeprom, don't turn off  ");
                usb_port_send_thread.set_buffer_size(2);
                
                //SYNCHRONIZE PROGRESS BAR DURING EEPROM BURN
                int ctr = 0;
                emit ProgressBar_setMax(500); 
                emit ProgressBar_valueChanged(0);
                
                for(int i = 0; i < 500; ++i){
                    usb_port_send_thread.wait_for_ready_read(100); //GET PULSE FROM LORDYPHON
                    emit ProgressBar_valueChanged(i);
                }
                emit setLabel("done ");
            }
            else {
                emit setLabel("checksum error");
                emit remoteMessageBox("data transfer incomplete, please try again");

            }
        }//end: if (response == "doit") {  //PLAY MODE IS OFF
        else if (response == lordyphon_response.play_mode_is_on) {  //PLAY MODE IS ON, PROMPT USER
            emit remoteMessageBox("lordyphon memory busy, press stop button");
        }
        else {
            emit remoteMessageBox("communication error, please try again!   ");
            qDebug() << "error, message not recognized";
        }
        
        usb_port_send_thread.clear_buffer();
        usb_port_send_thread.close_usb_port();
        mutex.unlock();
        emit activateButtons();
        emit finished();
    } //end: if (eeprom_parser->parse_eeprom()) {
    else{
        emit remoteMessageBox("file not found");
        usb_port_send_thread.close_usb_port();
        mutex.unlock();
        emit activateButtons();
        emit finished();
    }
}

void USBThread::run()
{
    //nothing to do here....
}