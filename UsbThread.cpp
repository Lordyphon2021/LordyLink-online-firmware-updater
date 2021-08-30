#include "UsbThread.h"
#include <QTextStream>
#include <qdebug.h>
#include <iomanip>
#include <QMutex>


using namespace std;


void Worker::update()
{
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();
    //OPEN USB PORT
    SerialHandler usb;
    usb.find_lordyphon_port();
    usb.open_lordyphon_port();
    
    usb.lordyphon_handshake();
    
    usb.set_buffer_size(3);
   
   
    //SET VARIABLES
    QString checksum_status_message;
   
    QByteArray header = "#";
    QByteArray burn_flash = "w";
    QByteArray tx_data;
    size_t index = 0;
    int bad_checksum_ctr = 0;
    int rx_error_ctr = 0;
    bool carry_on_flag = true;  //START CONDITION TO GET FIRST RECORD

    //OPEN DOWNLOADED FILE FOR PARSER
    
    sram_content.setFileName("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/TX_HEX_LOG.txt");
    sram_content.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&sram_content);
    Parser hex_parser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt");

    //PARSE DOWNLOADED HEXFILE
    
    if (hex_parser.parse_hex()) {  //BOOL METHOD, PARSER'S RESPOONSIBLE FOR HEXFILE ERRORS

        int hexfilesize = hex_parser.get_hexfile_size();   //GET SIZE FOR PROGRESS BAR AND LOOP

        if (hexfilesize == 0) {
            emit setLabel("error: no file found");
            carry_on_flag = false;
        
        }
        emit ProgressBar_setMax(hexfilesize);  //SET PROGRESSBAR MAXIMUM

        //usb.write_serial_data("u");    //lordyphon jump to bootloader
        delay(1000);            //ALLOW FOR SETTLING TIME
        usb.write_serial_data("§");  //RESET ADDRESS VARIABLES AND COUNTERS ON CONTROLLER
        delay(1000);           //ALLOW FOR SETTLING TIME
        
        
        //THIS LOOP SENDS PARSED HEX RECORDS TO CONTROLLER AND WAITS FOR CHECKSUM CONFIRMATION BEFORE SENDING THE
        //NEXT RECORD: IF CALCULATED CHECKSUM ON CONTROLLER IS "ok",  "index" IS INCREMENTED
        //
        
        while (index < hexfilesize && hexfilesize != 0) {

            if (carry_on_flag == true) {
                tx_data = hex_parser.get_hex_record(index);
                usb.write_serial_data(tx_data);
                out << "record nr. " << index << "  " << (tx_data.toHex()) << '\n';  //LOGFILE OUTPUT
            }
            usb.wait_for_ready_read(1000);  //THREAD BLOCKS UNTIL INCOMING CONFIRMATION MESSAGE OR TIMEOUT 1000ms
            checksum_status_message = usb.getInputBuffer(); //GET INCOMING CONFIRMATION MESSAGE

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
                usb.write_serial_data(tx_data);     //ASK AGAIN FOR CHECKSUM STATUS, CONTROLLER WILL SEND EITHER "ok" or "er"
                qDebug() << checksum_status_message;
                tx_data.clear();
                emit setLabel("rx error, checking status... ");
                out << "---------------RX ERROR AT INDEX---------------------- " << index << "  " << (tx_data.toHex()) << '\n'; //LOGFILE
                ++rx_error_ctr;         //IF MESSAGE ISNT READABLE FOR MORE THAN 8 TIMES, CONNECTION MUST BE LOST
                delay(1000);
                carry_on_flag = false;
            }
            if (bad_checksum_ctr > 100) {  //EXIT CONDITION: CHECKSUM ERROR
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
        
        delay(1000); //THIS IS NECESSARY FOR THE LAST MESSAGE TO BE RECEIVED CORRECTLY
        sram_content.close();
        emit ProgressBar_valueChanged(hexfilesize);
      
        //IF carry_on_flag IS TRUE AFTER EVERYTHING HAS BEEN SENT, EVERYTHING HAS BEEN TRANSMITTED CORRECTLY
        if (carry_on_flag == true) {   
            emit setLabel("transfer complete");
            usb.write_serial_data(burn_flash); //CALLS BURN FUNCTION ON CONTROLLER
            delay(1000);
        }
        else
            emit setLabel("transfer aborted");
        
        usb.close_usb_port();  
       

    }
    else {
        emit remoteMessageBox("file not found, check internet connection");
        
        emit ProgressBar_valueChanged(0);
        mutex.unlock();
        emit activateButtons();
        emit finished();
    }

    emit activateButtons();
    mutex.unlock();
    emit finished();  //THREAD CLOSED, QT WILL DESTROY LATER VIA SIGNALS

}



void Worker::get_eeprom_content()
{
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();
    SerialHandler usb_port;
    ofstream file;
    
    qDebug() << "entering get data thread";
    
    if (!usb_port.find_lordyphon_port()) {
        emit setLabel("lordyphon disconnected!");
        mutex.unlock();
        emit activateButtons();
        emit finished();
    }
    
    
    
    
    if (!usb_port.lordyphon_port_is_open())
        usb_port.open_lordyphon_port(); 
    
    if (!usb_port.lordyphon_handshake()){
        emit setLabel("connection error");
        emit activateButtons();
        mutex.unlock();
        emit finished();
    }
    
    QDir sets(QDir::homePath() + "/LordyLink/Sets");
    if (!sets.exists())
        sets.mkpath(".");
   
    QString sets_path = QDir::homePath();
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("dd_MM_yyyy__h_m_s");

   
    
   
    
    QString new_filepath = sets_path + "/LordyLink/Sets/saved_set_" + timestamp + ".txt";
    QString new_filename = "saved_set_" + timestamp + ".txt";
    
    
    file.open(new_filepath.toStdString());
    
    if (file.is_open()) {

                // update GUI
        




        QByteArray eeprom;
        QByteArray temp_rec;
        uint16_t checksum_uc = 0;
        uint16_t eeprom_checksum = 0;
        int error_ctr = 0;


        emit setLabel("reading set data");
        emit ProgressBar_setMax(128000);
        size_t progress_bar_ctr = 0;
        size_t rec_ctr = 0;
        //usb_port.dump_baud_rate();
        QByteArray tx_data = "r"; //init dump
        usb_port.write_serial_data(tx_data);
        usb_port.set_buffer_size(4);
        usb_port.wait_for_ready_read(20);
        usb_port.set_buffer_size(1000);
        usb_port.clear_buffer();
        
        while (eeprom.size() < 128000) {

           
            usb_port.wait_for_ready_read(20);

           

            eeprom += usb_port.getInputBuffer();



            emit ProgressBar_valueChanged(eeprom.size());



        }
        //checksum verification

        usb_port.set_buffer_size(2); //set buffer size for 16bit checksum from lordyphon



        for (auto i : eeprom)
            eeprom_checksum += static_cast<unsigned char>(i);  //qbytearray returns signed char, need unsigned for correct value

        tx_data = "s"; //request uint16 checksum from lordyphon
        usb_port.write_serial_data(tx_data);
        usb_port.wait_for_ready_read(2000);

        QByteArray checksum_from_lordyphon = usb_port.getInputBuffer();
        uint8_t msb = checksum_from_lordyphon.at(0);
        uint8_t lsb = checksum_from_lordyphon.at(1);

        checksum_uc = (msb << 8) | lsb;  //assemble 16bit checksum

        qDebug() << "checksum from uc: " << checksum_uc;
        qDebug() << "local checksum : " << eeprom_checksum;

        if (checksum_uc != eeprom_checksum) {
            emit remoteMessageBox("checksum error, try again");
            usb_port.clear_buffer();
            usb_port.close_usb_port();
            file.close();
            
            sets.remove(new_filepath);

            emit activateButtons();
            mutex.unlock();
            emit finished(); // exit thread

        }






        eeprom_checksum = 0;

        emit setLabel("writing file");
        delay(1000);






        for (int i = 0; i < eeprom.size(); ++i) {
            int temp = static_cast<unsigned char>(eeprom.at(i));
            //char temp = eeprom.at(i);




            if (i != 0 && i % 16 == 0) {
                file << endl;

            }
            file << setw(2) << setfill('0') << hex << temp;
        }
        file.close();
        emit newItem(new_filename);


        emit ProgressBar_valueChanged(128000);
        emit setLabel("done");
        usb_port.clear_buffer();
        usb_port.close_usb_port();
    }
    else
        emit remoteMessageBox("file not found!");
   
    
    usb_port.close_usb_port();
    emit activateButtons();
    mutex.unlock();
    emit finished();
    
}



















void Worker::send_eeprom_content()
{
    emit deactivateButtons();
    QMutex mutex;
    mutex.lock();
    SerialHandler usb_port2;
   
    emit setLabel("sending set");


    if (!usb_port2.find_lordyphon_port()) {
       
        emit remoteMessageBox("lordyphon disconnected!");
        emit activateButtons();
        mutex.unlock();
        emit finished();
    }




    if (!usb_port2.lordyphon_port_is_open())
        usb_port2.open_lordyphon_port();

    //if (!usb_port2.lordyphon_handshake()) {
    //    emit setLabel("connection error");
     //   emit finished();
    //}
    delay(500);
    usb_port2.set_buffer_size(100);
    if(!usb_port2.clear_buffer())
        qDebug() <<"buffer not empty";
   

    Parser eeprom_parser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_eeprom.txt");
   
   
    if (eeprom_parser.parse_eeprom()) {
        
        qDebug() << "entering send data thread";

        int setfilesize = eeprom_parser.get_eeprom_size();
        emit ProgressBar_setMax(setfilesize);
        emit setLabel("sending set");
        
        //prepare lordyphon for incoming transmission
        usb_port2.set_buffer_size(5);
        QByteArray call_lordyphon = "R";
       
        usb_port2.write_serial_data(call_lordyphon);
        //set buffer size for expected response
       
        
        usb_port2.wait_for_ready_read(2000); //allow up to 2 sec to confirm

        QString response = usb_port2.getInputBuffer();
        qDebug() << response;
        QByteArray temp_record;
        QString status;
        
        if (response == "doit") {
            size_t index = 0;
           
            call_lordyphon = "//";  //start transfer
            usb_port2.write_serial_data(call_lordyphon);
           
            
            while (index < setfilesize && setfilesize != 0) {
                
                
                temp_record = eeprom_parser.get_eeprom_record(index);
               
                
                usb_port2.write_serial_data(temp_record);  //send record


                usb_port2.set_buffer_size(3);
               
                usb_port2.wait_for_ready_read(1000);        //wait for eeprom burn 

                status = usb_port2.getInputBuffer();
                //qDebug() << status;
                index++;

                emit ProgressBar_valueChanged(static_cast<int>(index));


            }
            
            
            usb_port2.set_buffer_size(2);
            QByteArray checksum_bigendian = usb_port2.getInputBuffer();
            qDebug() << "checksum from lordyphon: " << checksum_bigendian;
            uint8_t msb = checksum_bigendian.at(0);
            uint8_t lsb = checksum_bigendian.at(1);

            uint16_t checksum_lordyphon = (msb << 8) | lsb;  //assemble 16bit checksum
            //if (checksum_lordyphon == eeprom_parser.get_eeprom_checksum()) {
              
                //if(usb_port2.clear_buffer())
                  
                delay(1000);
                QByteArray call_lordyphon = "ß";
                usb_port2.write_serial_data(call_lordyphon);
                


           // }


           
           
        }
        else if (response == "DONT") {

           
            emit remoteMessageBox("lordyphon memory busy, press stop button");
            emit activateButtons();
            mutex.unlock();
            emit finished();

        }
        else {
            qDebug() << "error, message not recognized";
            
        }
        usb_port2.clear_buffer();
        usb_port2.close_usb_port();
        mutex.unlock();
        emit activateButtons();
        emit finished();
    }
    else
    {
        emit remoteMessageBox("file not found");
        
        usb_port2.close_usb_port();
        mutex.unlock();
       emit activateButtons();
        emit finished();

    }




}

void USBThread::run()
{
    
   
}