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
    
    usb_port = new SerialHandler;
    thread = new USBThread;
    ui.setupUi(this);
   
	QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(on_update_button()));
    
    
    ui.hardware_connected_label->setText("       ");
    ui.QInstallLabel->hide();
    ui.QInstallProgressBar->hide();
	
    
    



   
    
   
    if (!usb_port->find_lordyphon_port()) {
        QNoHardwareDialog* no_hardware = new QNoHardwareDialog;

        
        int ctr = 0;
        
        while (!usb_port->find_lordyphon_port()) {
            ui.hardware_connected_label->setText("Lordyphon disconnected");
            no_hardware->setWindowTitle("Lordyphon not found!");
            no_hardware->show();
            
            int hardware_dialog_code = no_hardware->exec();

            if (hardware_dialog_code == QDialog::Rejected)
                exit(1);

            ctr++;

            if (ctr > 0 && !usb_port->find_lordyphon_port()) {
                QMessageBox error;
                error.setText("please connect Lordyphone or check power and usb connections.");
                error.exec();
            }

        }
            
    }
    if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && usb_port->lordyphon_handshake()) {
        ui.hardware_connected_label->setText("Lordyphon connected");
        usb_port->close_usb_port();
    }
}

void LordyLink::error_message_box(const char* message)
{
    QMessageBox* error_message = new QMessageBox;
    error_message->setText(message);
    error_message->exec();

    
}













void LordyLink::on_update_button()
{
            update.show();
            ui.QInstallLabel->hide();
            ui.QInstallProgressBar->hide();
            
            int dialog_code = update.exec();
            
            if (dialog_code == QDialog::Accepted) {
                
                ui.QInstallLabel->show();
                ui.QInstallProgressBar->show();
               
                QObject::connect(thread, SIGNAL(finished()), thread, SLOT(quit()));
                QObject::connect(thread, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
                QObject::connect(thread, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
                QObject::connect(thread, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));
                
                thread->start();
            }
         
    
           
    
   
}





