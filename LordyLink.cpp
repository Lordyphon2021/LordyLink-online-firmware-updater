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
    
    ui.setupUi(this);
   
	QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(on_update_button()));
    QObject::connect(ui.saveSetButton, SIGNAL(clicked()), this, SLOT(OnGetSetButton()));
   
    
    
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
                error.setText("please connect Lordyphone and activate update mode.");
                error.exec();
            }
            else {
                
                usb_port->open_lordyphon_port();
                if (usb_port->lordyphon_handshake()) {
                    ui.hardware_connected_label->setText("Lordyphon connected");
                    usb_port->close_usb_port();
                }
            }

        }

            
    }
    else if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && usb_port->lordyphon_handshake()) {
        ui.hardware_connected_label->setText("Lordyphon connected");
        usb_port->close_usb_port();
    }
    /*
    if (!usb_port->lordyphon_handshake()) {
        usb_port->find_lordyphon_port();
        if(! usb_port->lordyphon_port_is_open())
            usb_port->open_lordyphon_port();
        bool handshake_ok = false;
       
        
        while (!usb_port->lordyphon_handshake()) {

            QMessageBox error;
            error.setText("please set lordyphon to update mode (power on + rec button)");
            error.exec();
                usb_port->close_usb_port();
                usb_port->find_lordyphon_port();
                usb_port->open_lordyphon_port();
                
                if (usb_port->lordyphon_handshake())
                    break;
            
           

        }
        ui.hardware_connected_label->setText("Lordyphon connected");
        usb_port->close_usb_port();
    }*/
}
/*
void LordyLink::error_message_box(const char* message)
{
    QMessageBox* error_message = new QMessageBox;
    error_message->setText(message);
    error_message->exec();

    
}

*/

void LordyLink::on_update_button()
{
    usb_port->find_lordyphon_port();
    usb_port->open_lordyphon_port();
    usb_port->lordyphon_update_call();
   
        
    if (usb_port->lordyphon_update_call()) {
        usb_port->close_usb_port();
        
    

        update.show();
        ui.QInstallLabel->hide();
        ui.QInstallProgressBar->hide();

        int dialog_code = update.exec();

        if (dialog_code == QDialog::Accepted) {
            Worker* worker = new Worker;
            USBThread* thread = new USBThread;
            ui.QInstallLabel->show();

            ui.QInstallProgressBar->show();
            ui.QInstallProgressBar->valueChanged(0);
            worker->moveToThread(thread);

            connect(thread, &QThread::started, worker, &Worker::update);
            connect(worker, &Worker::finished, thread, &QThread::quit);
            connect(worker, &Worker::finished, worker, &Worker::deleteLater);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            connect(worker, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
            connect(worker, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
            connect(worker, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));

            thread->start();
        }
    }
   
    else {
        QMessageBox info;
        info.setText("please activate update mode (power on with rec button pressed)");
        info.exec();
        usb_port->close_usb_port();
    }
}

void LordyLink::OnGetSetButton()
{
    
    if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && !usb_port->lordyphon_update_call()) {

        usb_port->close_usb_port();




        ui.QInstallProgressBar->reset();

        Worker* worker2 = new Worker;;

        USBThread* thread2 = new USBThread;
        ui.QInstallLabel->show();
        ui.QInstallProgressBar->show();

        worker2->moveToThread(thread2);

        connect(thread2, &QThread::started, worker2, &Worker::get_sram_content);
        connect(worker2, &Worker::finished, thread2, &QThread::quit);
        connect(worker2, &Worker::finished, worker2, &Worker::deleteLater);
        connect(thread2, &QThread::finished, thread2, &QThread::deleteLater);
        connect(worker2, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
        connect(worker2, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
        connect(worker2, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));

        thread2->start();
    }
    else {

        QMessageBox info;
        info.setText("not possible in update mode");
        info.exec();



    }

}




