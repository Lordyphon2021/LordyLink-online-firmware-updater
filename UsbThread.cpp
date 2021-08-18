#include "UsbThread.h"
#include <QTextStream>
#include <qdebug.h>


using namespace std;


void Worker::update()
{
    SerialHandler usb;
    usb.find_lordyphon_port();
    usb.open_lordyphon_port();
    usb.set_buffer_size(4);

    QString checksum_status_message;
    QByteArray header = "#";
    QByteArray tx_data;
    size_t index = 0;
    int bad_checksum_ctr = 0;
    int rx_error_ctr = 0;
    bool carry_on_flag = true;

    sram_content.setFileName("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/sram.txt");
    sram_content.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&sram_content);
    HexToSerialParser parser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt");

    if (parser.parse()) {

        int hexfilesize = parser.get_hexfile_size();

        if (hexfilesize == 0) {
            emit setLabel("error: no file found");
        }
        emit ProgressBar_setMax(hexfilesize);

        while (index < hexfilesize && hexfilesize != 0) {

            if (carry_on_flag == true) {
                tx_data = parser.get_record(index);
                usb.write_serial_data(tx_data);
                out << "record nr. " << index << "  " << (tx_data.toHex()) << '\n';
            }
            usb.wait_for_ready_read(1000);
            checksum_status_message = usb.getInputBuffer();

            if (checksum_status_message == "er") {
                ++bad_checksum_ctr;
                out << "--------------CHECKSUM ERROR AT INDEX " << index << "  " << (tx_data.toHex()) << '\n';
                emit setLabel("checksum error...sending record again");
                carry_on_flag = true;
                QThread::sleep(3);
            }
            else if (checksum_status_message == "ok") {
                emit ProgressBar_valueChanged(static_cast<int>(index));
                emit setLabel("sending file ");
                rx_error_ctr = 0;
                carry_on_flag = true;
                ++index;
            }
            else {
                tx_data.clear();
                tx_data = "?";
                usb.write_serial_data(tx_data);
                qDebug() << checksum_status_message;
                tx_data.clear();
                emit setLabel("rx error, checking status... ");
                out << "---------------RX ERROR AT INDEX---------------------- " << index << "  " << (tx_data.toHex()) << '\n';
                ++rx_error_ctr;
                QThread::sleep(1);
                carry_on_flag = false;
            }
            if (bad_checksum_ctr > 8) {
                emit setLabel("file corrupted...");
                break;
            }
            if (rx_error_ctr > 8) {
                emit setLabel("connection lost...");
                break;
            }
        }
        
       
        sram_content.close();
        emit ProgressBar_valueChanged(static_cast<int>(index + 1));
        emit setLabel("transfer complete");
        tx_data = "*fl";
        usb.write_serial_data(tx_data);
        
        usb.close_usb_port();

    }
    else {
        emit setLabel("file not found, check internet connection");
        emit ProgressBar_valueChanged(0);
    }

    
    
    emit finished();

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
    
    file.open("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/atmega_sram.txt", ios_base::binary);
    QByteArray sram;
    QByteArray tx_data = "$ram";
    usb_port.write_serial_data(tx_data);
    usb_port.set_buffer_size(16);
    emit setLabel("reading SRAM");
    emit ProgressBar_setMax(128000);
    size_t progress_bar_ctr = 0;
    size_t rec_ctr = 0;

    while (sram.size() < 128000) {
        tx_data = "+";
        usb_port.write_serial_data(tx_data);
        usb_port.wait_for_ready_read(2000);
        progress_bar_ctr += 16;
        tx_data = "-";
        usb_port.write_serial_data(tx_data);
        sram += usb_port.getInputBuffer();
        emit ProgressBar_valueChanged(progress_bar_ctr);
          
    }
    emit setLabel("writing file");
    
    for (int i = 0; i < sram.size(); ++i) {
        int temp = static_cast<unsigned char>(sram.at(i)) ;
            
        if (i % 16 == 0) {
            file << endl << dec << "record nr. " << rec_ctr << " :";
            rec_ctr++;
        }
        file << hex << temp << ":";
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