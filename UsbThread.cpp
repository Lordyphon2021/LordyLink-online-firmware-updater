#include "UsbThread.h"
#include <QTextStream>

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
            usb.wait_for_ready_read();
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

                tx_data.clear();
                emit setLabel("rx error, checking status... ");
                out << "---------------RX ERROR AT INDEX " << index << "  " << (tx_data.toHex()) << '\n';
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
        tx_data = "&";
        usb.write_serial_data(tx_data);
        usb.close_usb_port();
        emit get_sram();
        emit finished();

    }
    else {
        emit setLabel("file not found, check internet connection");
        emit ProgressBar_valueChanged(0);
       
    }


}
void Worker::get_sram_content()
{
    SerialHandler usb_port;
    QFile sram_content_from_uc;
    QTextStream out(&sram_content_from_uc);
    sram_content_from_uc.setFileName("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/atmega_sram.txt");
    sram_content_from_uc.open(QIODevice::ReadWrite | QIODevice::Text);
    
    
    if (usb_port.find_lordyphon_port() && usb_port.open_lordyphon_port() && usb_port.lordyphon_handshake()) {
        usb_port.set_buffer_size(2);
        QByteArray tx_data = "$ram";
        usb_port.write_serial_data(tx_data);
        
        
        usb_port.wait_for_ready_read();

        
        QByteArray message_size_array = usb_port.getInputBuffer();
        quint16 message_size = ((message_size_array.at(0) << 8) & 0xff00) | (message_size_array.at(1) & 0x00ff);
        
        usb_port.set_buffer_size(message_size);
        usb_port.wait_for_ready_read();
        
        QByteArray buffer = usb_port.getInputBuffer();
        
        emit setLabel("loading sram");
        emit ProgressBar_setMax(buffer.size());
    
        out << buffer.toHex(':');
           
            
            

    
  


    sram_content_from_uc.close();
    emit finished();
    }
    else
  
        emit setLabel("error: no usb connection");


}

void USBThread::run()
{
    
   
}