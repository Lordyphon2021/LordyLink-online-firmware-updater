#include "UsbThread.h"
#include <QTextStream>
#include <qdebug.h>


using namespace std;


void Worker::update()
{
    
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
    HexToSerialParser parser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt");

    //PARSE DOWNLOADED HEXFILE
    
    if (parser.parse()) {  //BOOL METHOD, PARSER'S RESPOONSIBLE FOR HEXFILE ERRORS

        int hexfilesize = parser.get_hexfile_size();   //GET SIZE FOR PROGRESS BAR AND LOOP

        if (hexfilesize == 0) {
            emit setLabel("error: no file found");
            carry_on_flag = false;
        
        }
        emit ProgressBar_setMax(hexfilesize);  //SET PROGRESSBAR MAXIMUM

        usb.write_serial_data("u");    //lordyphon jump to bootloader
        QThread::sleep(1);            //ALLOW FOR SETTLING TIME
        usb.write_serial_data("§");  //RESET ADDRESS VARIABLES AND COUNTERS ON CONTROLLER
        QThread::sleep(1);            //ALLOW FOR SETTLING TIME
        
        
        //THIS LOOP SENDS PARSED HEX RECORDS TO CONTROLLER AND WAITS FOR CHECKSUM CONFIRMATION BEFORE SENDING THE
        //NEXT RECORD: IF CALCULATED CHECKSUM ON CONTROLLER IS "ok",  "index" IS INCREMENTED
        //
        
        while (index < hexfilesize && hexfilesize != 0) {

            if (carry_on_flag == true) {
                tx_data = parser.get_record(index);
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
                QThread::sleep(3);                 //ALLOW TIME FOR USER FEEDBACK
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
                QThread::sleep(1);
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
        
        QThread::sleep(1); //THIS IS NECESSARY FOR THE LAST MESSAGE TO BE RECEIVED CORRECTLY, DON'T ASK ME WHY :)
        sram_content.close();
        emit ProgressBar_valueChanged(hexfilesize);
      
        //IF carry_on_flag IS TRUE AFTER EVERYTHING HAS BEEN SENT, EVERYTHING HAS BEEN TRANSMITTED CORRECTLY
        if (carry_on_flag == true) {   
            emit setLabel("transfer complete");
            usb.write_serial_data(burn_flash); //CALLS BURN FUNCTION ON CONTROLLER
            QThread::sleep(1);
        }
        else
            emit setLabel("transfer aborted");
        
        usb.close_usb_port();  
       

    }
    else {
        emit setLabel("file not found, check internet connection");
        emit ProgressBar_valueChanged(0);
    }

    
    
    emit finished();  //THREAD CLOSED, QT WILL DESTROY LATER VIA SIGNALS

}



void Worker::get_sram_content()
{
    SerialHandler usb_port;
    ofstream file;
    
    
    
    if (!usb_port.find_lordyphon_port()) {
        emit setLabel("lordyphon disconnected!");
        emit finished();
    }
    
    
    
    
    if (!usb_port.lordyphon_port_is_open())
        usb_port.open_lordyphon_port(); 
    
    if (!usb_port.lordyphon_handshake()){
        emit setLabel("connection error");
        emit finished();
    }
    
    file.open("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_eeprom.txt", ios_base::binary);
    QByteArray eeprom;
   
    QByteArray temp_rec;
    
    emit setLabel("reading SRAM");
    emit ProgressBar_setMax(128000);
    size_t progress_bar_ctr = 0;
    size_t rec_ctr = 0;
    //usb_port.dump_baud_rate();
    QByteArray tx_data = "r"; //init dump
    usb_port.write_serial_data(tx_data);
    usb_port.set_buffer_size(4);
    usb_port.wait_for_ready_read(20);
    usb_port.set_buffer_size(1000);
    
    while (eeprom.size() < 128000) {
        
        //tx_data = "+";
        //usb_port.write_serial_data(tx_data);
        usb_port.wait_for_ready_read(20);
       
        //tx_data = "-";
        //usb_port.write_serial_data(tx_data);
        
        eeprom += usb_port.getInputBuffer();
       

        
        emit ProgressBar_valueChanged(eeprom.size());

        //eeprom_record_sum = std::accumulate(eeprom_record.begin(), eeprom_record.end(), eeprom_record_sum);  // get sum of last record

    }
  
    
        
       
    
   
    emit setLabel("writing file");
    rec_ctr = 0;
    for (int i = 0; i < eeprom.size(); ++i) {
        int temp = static_cast<unsigned char>(eeprom.at(i)) ;
            
        if (i % 16 == 0) {
            file << endl << dec << "record nr. " << rec_ctr << " :";
            rec_ctr++;
        }
        file << hex << temp << " ";
    }
    
    emit ProgressBar_valueChanged(128000);
    emit setLabel("done");
    usb_port.close_usb_port();
    file.close();
    emit finished();
    
}

void USBThread::run()
{
    
   
}