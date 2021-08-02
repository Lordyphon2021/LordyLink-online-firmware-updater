#include "LordyLink.h"


#include <qstring.h>
#include<qlistwidget.h>
#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIODevice>
#include <QDialog>




using namespace std;






LordyLink::LordyLink(QWidget *parent)
    : QMainWindow(parent)
{
    
    
    ui.setupUi(this);
    //QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), SLOT(download_wrapper()));
	QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(usb_action_wrapper()));
    ui.QInstallLabel->hide();
    ui.QInstallProgressBar->hide();
	 
    filehandler = new Filehandler(ui.Q_UpdateFeedback);
     
}

void LordyLink::error_message_box(const char* message)
{
    QMessageBox error_message;
    error_message.setText(message);
    error_message.exec();
}





void LordyLink::download(Filehandler* filehandler)
{
    QString location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/lordyphon_proto.txt";
    QString path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt"; 
    filehandler->download(location, path);
    
}


void LordyLink::download_wrapper()
{
    download(filehandler);
    
    
}

void LordyLink::usb_action_wrapper()
{
    
    //scans ports for manufacturer ID "FTDI", 
    //( I have no vendor ID yet, so final identification has to be done via handshake 
    if(!usb.find_lordyphon_port())
        error_message_box("Port not found!");
    
    if (usb.open_lordyphon_port()) {
        //lordylink sends tx_passphrase "c++ is a great language", if USB response is "YES INDEED"
        //lordyphon is successfully identified and ready for communication
        
        if (usb.lordyphon_handshake() == true) {
            ui.QUsbStatus->addItem("Handshake complete, Lordyphon connected!");
            
            
            update.show();
            ui.QInstallLabel->hide();
            ui.QInstallProgressBar->hide();
            
            int dialog_code = update.exec();
            
            
            
            
            if (dialog_code == QDialog::Accepted) {
                
                ui.QInstallLabel->show();
                ui.QInstallProgressBar->show();
                
                parser = new HexToSerialParser("C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt");
                parser->parse();
                parser->send_hex_file();
             
              

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





