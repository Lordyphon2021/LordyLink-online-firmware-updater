#include "UsbThread.h"
#include "ChecksumValidator.h"
#include <QTextStream>
#include <qdebug.h>
#include <iomanip>
#include <QMutex>


using namespace std;

// THIS IS THE FIRMWARE UPDATE PROCESS, A BLOCKING MASTER DESIGN
void Worker::update()
{
    
    //don't allow erratic user input
    emit deactivateButtons();
    //safety
    QMutex mutex;
    mutex.lock();
    //OPEN USB PORT
    
    usb_port_update_thread = new SerialHandler; //not in ctor, only one serial port can be active
    
    
    usb_port_update_thread->find_lordyphon_port();
    usb_port_update_thread->open_lordyphon_port();
    usb_port_update_thread->lordyphon_handshake();
    //SET BUFFER FOR "OK" or "ER" + '\0' CHARACTER
    usb_port_update_thread->set_buffer_size(3);
   
   
    //SET VARIABLES
   
    carry_on_flag = true;  //START CONDITION TO GET FIRST RECORD

    //OPEN LOGFILE
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("dd_MM_yyyy___h_m_s");
    eeprom_content_logfile.setFileName(QDir::homePath() + "/LordyLink/log/firmware_transfer_log_" + timestamp + ".txt");
    eeprom_content_logfile.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&eeprom_content_logfile);
    
    //create parser object
    hex_parser = new Parser(selected_firmware);

    //PARSE DOWNLOADED HEXFILE
    emit setLabel("validating hexfile     ");
    
    if (hex_parser->parse_hex()) {  //BOOL METHOD, PARSER IS RESPONSIBLE FOR VALIDATING HEXFILE
                                   //IF TRUE, HEXFILE IS VALID
        int hexfilesize = hex_parser->get_hexfile_size();   //GET SIZE FOR PROGRESS BAR AND LOOP

        if (hexfilesize == 0) {    //DOUBLE CHECK
            emit setLabel("error: no file found");
            carry_on_flag = false;
        
        }
        emit ProgressBar_setMax(hexfilesize);  //SET PROGRESSBAR MAXIMUM

        delay(1000);            //ALLOW FOR SETTLING TIME
        
        //THIS LOOP SENDS PARSED HEX RECORDS TO CONTROLLER AND WAITS FOR CHECKSUM CONFIRMATION BEFORE SENDING THE
        //NEXT RECORD: IF CALCULATED CHECKSUM ON CONTROLLER IS "ok",  "index" IS INCREMENTED
        
        while (index < hexfilesize && hexfilesize != 0) {  //TRIPLE CHECK HEXFILE SIZE

            if (carry_on_flag == true) {  //INITIAL STATE == TRUE
                tx_data = hex_parser->get_hex_record(index);
                usb_port_update_thread->write_serial_data(tx_data); //SEND RECORD TO USB
                out << "record nr. " << index << "  " << (tx_data.toHex()) << '\n';  //LOGFILE OUTPUT
            }
            usb_port_update_thread->wait_for_ready_read(1000);  //THREAD BLOCKS UNTIL INCOMING CONFIRMATION MESSAGE OR TIMEOUT 1000ms
            checksum_status_message = usb_port_update_thread->getInputBuffer(); //GET INCOMING CONFIRMATION MESSAGE

            if (checksum_status_message == "er") {  //CHECKSUM CALCULATION HAS GONE WRONG
                ++bad_checksum_ctr;                 //EXIT CONDITION, IF HEXFILE IS CORRUPTED, CTR WILL GO UP TO 8 BEFORE ABORTING
                out << "--------------CHECKSUM ERROR AT INDEX " << index << "  " << (tx_data.toHex()) << '\n'; //LOGFILE OUTPUT
                emit setLabel("checksum error...sending record again");
                carry_on_flag = true;              //READ RECORD AGAIN AT SAME VECTOR INDEX
                delay(1000);                 //ALLOW TIME FOR USER FEEDBACK
            }
            else if (checksum_status_message == "ok") {      //CHECKSUM IS CORRECT   
                emit ProgressBar_valueChanged(static_cast<int>(index)); //UPDATE PROGRESS BAR
                emit setLabel("sending file ");
                rx_error_ctr = 0;
                carry_on_flag = true;  //ALLOW RECORD READ
                ++index;               //FROM NEXT VECTOR INDEX
            }
            else {
                tx_data.clear();        // INCOMING CONFIRMATION MESSAGE IS CORRUPTED
                tx_data = "?";
                usb_port_update_thread->write_serial_data(tx_data);     //ASK AGAIN FOR CHECKSUM STATUS, CONTROLLER WILL SEND EITHER "ok" or "er"
                qDebug() << checksum_status_message;
                tx_data.clear();
                emit setLabel("rx error, checking status... ");
                out << "---------------RX ERROR AT INDEX---------------------- " << index << "  " << (tx_data.toHex()) << '\n'; //LOGFILE
                ++rx_error_ctr;         //IF MESSAGE ISNT READABLE FOR MORE THAN 8 TIMES, CONNECTION MUST BE LOST
                delay(1000);
                carry_on_flag = false;
            }
            if (bad_checksum_ctr > 8) {  //EXIT CONDITION: CHECKSUM ERROR
                emit setLabel("file corrupted...");
                carry_on_flag = false;
                break;
            }
            if (rx_error_ctr > 8) {     //EXIT CONDITION: CONNECTION ERROR
                emit setLabel("rx error, check connection ");
                carry_on_flag = false;
                break;
            }
        }
        
        delay(1000); //THIS IS NECESSARY FOR THE NEXT MESSAGE TO BE RECEIVED CORRECTLY
        eeprom_content_logfile.close(); // LOGFILE DONE
        emit ProgressBar_valueChanged(hexfilesize); //SET PROGRESSBAR TO MAX
      
        //IF carry_on_flag IS TRUE AFTER EVERYTHING HAS BEEN SENT, EVERYTHING HAS BEEN TRANSMITTED CORRECTLY
        if (carry_on_flag == true) {   
            emit setLabel("transfer complete");
            usb_port_update_thread->write_serial_data(burn_flash); //CALLS BURN FUNCTION ON LORDYPHON
            delay(1000);
            emit setLabel("burning flash, don't turn off");
            delay(10000);
            emit setLabel("restart lordyphon");
            delay(2000);
            
           
        }
        else
            emit setLabel("transfer aborted");
        
        usb_port_update_thread->close_usb_port();
       

    }//end: if (hex_parser.parse_hex())
    else {
        //delete usb_port_update_thread; // SerialHandler is a child of QObject, no delete necessary
        emit remoteMessageBox("file not found, check internet connection");
        emit ProgressBar_valueChanged(0);
       
    }
    
    
    
    
    
    //delete usb_port_update_thread;
    delete hex_parser; //no child of QObject
    emit activateButtons();
    mutex.unlock();
    emit finished();  //THREAD CLOSED, QT WILL DESTROY LATER VIA SIGNALS

}



//THIS IS THE GET SET METHOD, READS ALL 128kB AT ONCE, 
//CHECKSUM IS CALCULATED AFTER LAST BYTE IS READ.


void Worker::get_eeprom_content()
{
    
    //GUI/THREAD SAFETY MEASURES
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();
   
    usb_port_get_thread = new SerialHandler;
    
    
    //check if lordyphon is still connected
    if (!usb_port_get_thread->find_lordyphon_port()) {
        emit setLabel("lordyphon disconnected!");
        mutex.unlock();
        emit activateButtons();
        emit finished();
    }
    //open port
    if (!usb_port_get_thread->lordyphon_port_is_open())
        usb_port_get_thread->open_lordyphon_port();
    //ask for ID
    if (!usb_port_get_thread->lordyphon_handshake()){
        emit setLabel("connection error");
        emit activateButtons();
        mutex.unlock();
        emit finished();
    }
    
    //create timestamp
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("dd_MM_yyyy___h_m_s");
    //add to path and file name
    QString new_filepath_qstring = QDir::homePath() + "/LordyLink/Sets/saved_set_" + timestamp + ".txt";
    QString new_filename = "saved_set_" + timestamp + ".txt";
    
    //open new file with timestamp in name
    ofstream set_file;
    set_file.open(new_filepath_qstring.toStdString());
    
    if (set_file.is_open()) {
        uint16_t checksum_lordyphon = 0;
       
        //GUI feedback
        emit setLabel("reading set data");
        emit ProgressBar_setMax(128000);
        progress_bar_ctr = 0;
        rec_ctr = 0;
        tx_data = "r"; //init dump
        usb_port_get_thread->write_serial_data(tx_data); //send dump request
        //incoming message size known, set buffer size
        usb_port_get_thread->set_buffer_size(4);
        usb_port_get_thread->wait_for_ready_read(20);
        //waits
        usb_port_get_thread->set_buffer_size(1000);
        usb_port_get_thread->clear_buffer(); //flush out old data
        //read all in a loop, add input buffer to eeprom byte array
        while (eeprom.size() < 128000) {
            usb_port_get_thread->wait_for_ready_read(20);
            eeprom += usb_port_get_thread->getInputBuffer();
            //update progress bar
            emit ProgressBar_valueChanged(eeprom.size());
        }
        //checksum verification

        usb_port_get_thread->set_buffer_size(2); //set buffer size for 16bit checksum from lordyphon
        tx_data = "s"; //request uint16 checksum from lordyphon
        usb_port_get_thread->write_serial_data(tx_data);
        usb_port_get_thread->wait_for_ready_read(2000);

        QByteArray checksum_from_lordyphon = usb_port_get_thread->getInputBuffer();
        uint8_t msb = checksum_from_lordyphon.at(0); //big endian
        uint8_t lsb = checksum_from_lordyphon.at(1);

        checksum_lordyphon = (msb << 8) | lsb;  //assemble uint16_t checksum
        ChecksumValidator eeprom_checker;
        eeprom_checker.set_Data(eeprom, checksum_lordyphon);

        if (eeprom_checker.is_valid()) {
            
            emit setLabel("writing file");
            delay(1000);

            for (int i = 0; i < eeprom.size(); ++i) {
                //create readable format (nice for debugging)
                int temp = static_cast<unsigned char>(eeprom.at(i));
                //set format for parser
                if (i != 0 && i % 16 == 0) {
                    set_file << endl;
                }
                set_file << setw(2) << setfill('0') << hex << temp;
            }
            set_file.close();
            //add new set to model/QTableView
            emit newItem(new_filename);
            emit ProgressBar_valueChanged(128000);
            emit setLabel("done");
        }
        else {
            
            emit remoteMessageBox("checksum error, try again");
            set_file.close();
            //delete temp file
            QDir sets = new_filepath_qstring;
            sets.remove(new_filepath_qstring);
        }
        
    }//end:  if (set_file.is_open())
    else
        emit remoteMessageBox("file not found!");
   
    
    //clean up, reactivate GUI buttons
    usb_port_get_thread->clear_buffer();
    usb_port_get_thread->close_usb_port();
    emit activateButtons();
    mutex.unlock();
    emit finished();
    
}


//SEND SET TO LORDYPHON METHOD
//COMMUNICATION LORDYLINK TO LORDYPHON ONLY WORKS PROPERLY IF DATA CHUNKS ARE RATHER SMALL,
//OTHERWISE THE MICROCONTROLLER GETS OVERWHELMED AND RESETS ITSELF.(TRIED ALL AVAILABLE BAUD RATES)
//THUS, I DECIDED TO SEND 16 BYTE CHUNKS AND HAVE THE TRANSFER CONFIRMED BEFORE CARRYING ON.
//THIS SEMI-SYNCHRONIZED TRANSFER WORKS BEST SO FAR.


void Worker::send_eeprom_content()
{
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();
    
    usb_port_send_thread = new SerialHandler;
    
    emit setLabel("sending set");
    
    if (!usb_port_send_thread->find_lordyphon_port()) {
        emit remoteMessageBox("lordyphon disconnected!");
        emit activateButtons();
        mutex.unlock();
        emit finished();
    }
    if (!usb_port_send_thread->lordyphon_port_is_open())
        usb_port_send_thread->open_lordyphon_port();

   
    delay(500);
    usb_port_send_thread->set_buffer_size(100);
    if(!usb_port_send_thread->clear_buffer())
        qDebug() <<"buffer not empty";

    qDebug() << "in send thread: " << selected_set;
    
    set_dir = QDir::homePath() + "/LordyLink/Sets/";
    //hand path to set to parser
    eeprom_parser = new Parser(set_dir + selected_set);
  
    if (eeprom_parser->parse_eeprom()) {
        //get size
        int setfilesize = eeprom_parser->get_eeprom_size();
        //set progress bar max
        emit ProgressBar_setMax(setfilesize);
        emit setLabel("sending set");
        
        //prepare lordyphon for incoming transmission
        usb_port_send_thread->set_buffer_size(5);
        QByteArray call_lordyphon = "R";
       
        usb_port_send_thread->write_serial_data(call_lordyphon);
        //set buffer size for expected response
        usb_port_send_thread->wait_for_ready_read(2000); //allow up to 2 sec to confirm

        QString response = usb_port_send_thread->getInputBuffer();
        QByteArray temp_record;
        
        
        if (response == "doit") {  //play mode is off
            size_t index = 0;
            call_lordyphon = "//";  //start transfer
            usb_port_send_thread->write_serial_data(call_lordyphon);
            QByteArray checksum_vec;

            //SEND DATA
            while (index < setfilesize && setfilesize != 0) {
                temp_record = eeprom_parser->get_eeprom_record(index); //get record
                checksum_vec += temp_record; // sum up all bytes for checksum calculation
                usb_port_send_thread->write_serial_data(temp_record);  //send record
                usb_port_send_thread->set_buffer_size(3);   //expected confirmation mnessage size
                usb_port_send_thread->wait_for_ready_read(1000);      //wait for confirmation
                //any 3 byte message could be used here, this is just for synchronizing
                usb_port_send_thread->getInputBuffer(); //dummy read

                index++;

                emit ProgressBar_valueChanged(static_cast<int>(index));
            }
           
            usb_port_send_thread->clear_buffer();
            //expecting lordyphon checksum
            delay(500);

            tx_data = "s"; //request uint16 checksum from lordyphon
            usb_port_send_thread->write_serial_data(tx_data);
            emit setLabel("waiting for checksum");
            usb_port_send_thread->wait_for_ready_read(-1); // no timeout
            
            
            QByteArray checksum_from_lordyphon = usb_port_send_thread->getInputBuffer();
            uint8_t msb = checksum_from_lordyphon.at(0); //big endian
            uint8_t lsb = checksum_from_lordyphon.at(1);

            
            uint16_t checksum_lordyphon = (msb << 8) | lsb;  //assemble uint16_t checksum
            uint16_t checksum_local = 0;

            ChecksumValidator eeprom_checker2;
            eeprom_checker2.set_Data(checksum_vec, checksum_lordyphon);

            

            if (eeprom_checker2.is_valid()) {
                emit setLabel("checksum ok  ");
                delay(1000);
                QByteArray call_lordyphon = "ß";  //DATA CORRECT, SEND BURN EEPROM MESSAGE
                usb_port_send_thread->write_serial_data(call_lordyphon);
                emit setLabel("burning eeprom, don't turn off  ");
                usb_port_send_thread->set_buffer_size(2);
                
                //drive progress bar during eeprom burn
                int ctr = 0;
                emit ProgressBar_setMax(500);
                emit ProgressBar_valueChanged(0);
                
                for(int i = 0; i < 500; ++i){
                    usb_port_send_thread->wait_for_ready_read(100); //get pulse from lordyphone
                    emit ProgressBar_valueChanged(i);

                
                }
                
                
               
                emit setLabel("done ");
            }
            else {
                emit setLabel("checksum error");
                emit remoteMessageBox("data transfer incomplete, please try again");

            }


        }//end: if (response == "doit") {  //play mode is off
        else if (response == "DONT") {  //play mode is on
            emit remoteMessageBox("lordyphon memory busy, press stop button");
           

        }
        else {
            qDebug() << "error, message not recognized";
            
        }
        usb_port_send_thread->clear_buffer();
        usb_port_send_thread->close_usb_port();
        mutex.unlock();
        emit activateButtons();
        delete eeprom_parser; //not child of QObject
        emit finished();
    } //end: if (eeprom_parser->parse_eeprom()) {
    else
    {
        emit remoteMessageBox("file not found");
        usb_port_send_thread->close_usb_port();
        mutex.unlock();
        emit activateButtons();
        delete eeprom_parser; //not child of QObject
        emit finished();

    }




}

void USBThread::run()
{
    
   //nothing to do here....
}