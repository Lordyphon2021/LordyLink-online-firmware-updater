#include "LordyLink.h"


#include <qstring.h>
#include<qlistwidget.h>
#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIODevice>
#include <QDialog>
#include "QNoHardwareDialog.h"







using namespace std;






LordyLink::LordyLink(QWidget *parent)
    : QMainWindow(parent)
{
    
   

    ui.setupUi(this);
   
	QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(usb_action_wrapper()));
    ui.QInstallLabel->hide();
    ui.QInstallProgressBar->hide();
	
    
    



   
    usb = new SerialHandler;
    if (!usb->find_lordyphon_port()) {
        QNoHardwareDialog* no_hardware = new QNoHardwareDialog;

        
        int ctr = 0;
        
        while (!usb->find_lordyphon_port()) {
            
            no_hardware->setWindowTitle("Lordyphon not found!");
            no_hardware->show();
            
            int hardware_dialog_code = no_hardware->exec();

            if (hardware_dialog_code == QDialog::Rejected)
                exit(1);

            ctr++;

            if (ctr > 0 && !usb->find_lordyphon_port()) {
                QMessageBox error;
                error.setText("please connect Lordyphone or check power and usb connections.");
                error.exec();
            }

        }
            
    }
    if (usb->find_lordyphon_port() && usb->open_lordyphon_port() && usb->lordyphon_handshake()) {
        ui.QUsbStatus->addItem("Lordyphon connected");
    }
}

void LordyLink::error_message_box(const char* message)
{
    QMessageBox error_message;
    error_message.setText(message);
    error_message.exec();
}


bool LordyLink::send_hex_to_usb()
{
    parser = new HexToSerialParser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt", ui.QUsbStatus);
    size_t index = 0;
    size_t timeout_ctr = 0;

    if (parser->parse()) {

        if (parser->get_hexfile_size() == 0) {
            error_message_box("file empty");
            return false;
        }
        
        bool checksum = false;
        QString checksum_status_message;
        QByteArray header = "#";
        QByteArray tx_data;
        
        while (index <= parser->get_hexfile_size()-1) {
            tx_data.clear();
            QByteArray tx_data = parser->get_record(index);
            usb->write_serial_data(tx_data);
           
            ui.QUsbStatus->addItem("it");
            
            
            checksum_status_message = usb->read_fixed_size(5);
         
            
        
            if (checksum_status_message == "error") {
                ++timeout_ctr;
                
                ui.QUsbStatus->addItem("checksum error!");
                checksum_status_message.clear();
                checksum = false;
                continue;

            }
            else if (checksum_status_message == "valid") {
                timeout_ctr = 0;
                ++index;
                ui.QUsbStatus->addItem("checksum ok");
                checksum_status_message.clear();
                checksum = true;

            }
            else {
                ++timeout_ctr;
                checksum_status_message.clear();
                checksum = false;
                continue;
            }

            if (timeout_ctr == 5) {
                error_message_box("file corrupted");
                usb->write_serial_data(header);
                return false;
            }
            
        }
        
        usb->write_serial_data(header);
        ui.QUsbStatus->addItem("hexfile sent!");
        delete parser;
        return true;

    }

}










void LordyLink::usb_action_wrapper()
{
    
    //scans ports for manufacturer ID "FTDI", 
    //( I have no vendor ID yet, so final identification has to be done via handshake 
    if(!usb->find_lordyphon_port())
        error_message_box("Port not found!");
    
    if (usb->open_lordyphon_port()) {
        //lordylink sends tx_passphrase "c++ is a great language", if USB response is "YES INDEED"
        //lordyphon is successfully identified and ready for communication
        
        if (usb->lordyphon_handshake() == true) {
            ui.QUsbStatus->addItem("Handshake complete, Lordyphon connected!");
            update.show();
            ui.QInstallLabel->hide();
            ui.QInstallProgressBar->hide();
            
            int dialog_code = update.exec();
            
            if (dialog_code == QDialog::Accepted) {
                
                ui.QInstallLabel->show();
                ui.QInstallProgressBar->show();
                
                if (send_hex_to_usb() == true)
                    error_message_box("file sent");
                 else
                    ui.QUsbStatus->addItem("error: update unsuccessful");
            }
         
        }
        else {
            error_message_box("handshake failed. set Lordyphon to update mode");
            return;
        }
    }
    else {
        error_message_box("Lordyphon not connected.");
        return;
    }
}





