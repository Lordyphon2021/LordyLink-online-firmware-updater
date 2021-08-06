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
                
                parser = new HexToSerialParser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt", ui.QUsbStatus);
                size_t index = 0;
                
               
                
                if (parser->parse()) {
                    ui.QUsbStatus->addItem("parsing...");
                    QByteArray& tx_data = parser->get_record(index);
                    ui.QUsbStatus->addItem(tx_data); 
                    usb->write_serial_data(tx_data);

                }
                
                
                
                
                
                
                
                
                else
                    ui.QUsbStatus->addItem("parser error");
                
               
               
               // ui.QInstallProgressBar->setMaximum(parser->get_hexfile_size());
               // ui.QInstallProgressBar->
                
               

                   
               
                   

                
            }
            else if(dialog_code == QDialog::Rejected)
            {
              
               // QMessageBox hurra;
               // hurra.setText("cancel pressed");
               // hurra.exec();
                    

               

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





