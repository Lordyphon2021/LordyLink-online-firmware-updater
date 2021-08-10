#include "UsbThread.h"
#include <QTextStream>



void USBThread::run()
{
   
        
        SerialHandler usb;
        
        usb.find_lordyphon_port();
        usb.open_lordyphon_port();
        usb.set_buffer_size(3);

        QString checksum_status_message;
        QByteArray header = "#";
        QByteArray tx_data;
        size_t index = 0;
        int timeout_ctr = 0;
        
        sram_content.setFileName("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/sram.txt");
        sram_content.open(QIODevice::ReadWrite | QIODevice::Text);
        QTextStream out(&sram_content);
        parser = new HexToSerialParser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt");
       
       



        if (parser->parse()) {
           
            
           
            if (parser->get_hexfile_size() == 0) {

                emit setLabel("error: no file found");
            }

            emit setMax(parser->get_hexfile_size());

            

            while (index < parser->get_hexfile_size()) {

                QByteArray tx_data = parser->get_record(index);
                usb.write_serial_data(tx_data);
                out <<(tx_data.toHex()) << '\n';
                
                usb.wait_for_ready_read();
                checksum_status_message = usb.getInputBuffer();
               

                if (checksum_status_message == "er") {
                    ++timeout_ctr;

                    emit setLabel("checksum error...sending record again");
                    QThread::sleep(3);
                   
                }
                else if (checksum_status_message == "ok") {

                    emit valueChanged(static_cast<int>(index));
                    
                    emit setLabel("sending file ");

                    ++index;
                }


                if (timeout_ctr > 8) {
                    emit setLabel("file corrupted");
                    break;
                }

            }


            sram_content.close();
            delete parser;

            emit setLabel("transfer complete");
           
        }


}