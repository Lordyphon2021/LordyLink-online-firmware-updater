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
    // model erzeugen
    model = new QStandardItemModel();

    
    QDir sets(QDir::homePath() + "/LordyLink/Sets");
    if (!sets.exists())
        sets.mkpath(".");
    
    // read files from directory
    home = QDir::homePath() + "/LordyLink/Sets";
    
    
    QDir directory(home);
    QStringList txtfiles = directory.entryList(QStringList() << "*.txt", QDir::Files);
    
    foreach(QString filename, txtfiles) {
        qDebug() << filename;
        QStandardItem* itemname = new QStandardItem(filename);
        itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
        model->appendRow(QList<QStandardItem*>() << itemname);
    }
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("set name: "));
    
    // Verbindung des models mit der View
    ui.dirView->setModel(model);
    
    for (int col = 0; col < model->rowCount(); col++)
    {
        ui.dirView->setColumnWidth(col, 300);
    }
    
    // Slots verbinden
    QObject::connect(ui.dirView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(renameStart(const QModelIndex)));
   
    QObject::connect(ui.dirView->model(), SIGNAL(itemChanged(QStandardItem*)), SLOT(renameEnd(QStandardItem*)));
	QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(on_update_button()));
    QObject::connect(ui.saveSetButton, SIGNAL(clicked()), this, SLOT(OnGetSetButton()));
    QObject::connect(ui.sendSetButton, SIGNAL(clicked()), this, SLOT(OnSendSetButton()));
    
    
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
                    if (usb_port->clear_buffer())
                        usb_port->close_usb_port();
                }
            }

        }

            
    }
    else if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && usb_port->lordyphon_handshake()) {
        ui.hardware_connected_label->setText("Lordyphon connected");
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
    }
    
}




void LordyLink::on_update_button()
{
    usb_port->find_lordyphon_port();
    usb_port->open_lordyphon_port();
    usb_port->lordyphon_update_call();
   
        
    if (usb_port->lordyphon_update_call()) {
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
        
    

        update.show();
        ui.QInstallLabel->hide();
        ui.QInstallProgressBar->hide();

        int dialog_code = update.exec();

        if (dialog_code == QDialog::Accepted) {
            Worker* update_worker = new Worker;
            USBThread* update_thread = new USBThread;
            ui.QInstallLabel->show();

            ui.QInstallProgressBar->show();
            ui.QInstallProgressBar->valueChanged(0);
            update_worker->moveToThread(update_thread);

            connect(update_thread, &QThread::started, update_worker, &Worker::update);
            connect(update_worker, &Worker::finished, update_thread, &QThread::quit);
            connect(update_worker, &Worker::finished, update_worker, &Worker::deleteLater);
            connect(update_thread, &QThread::finished, update_thread, &QThread::deleteLater);
            connect(update_worker, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
            connect(update_worker, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
            connect(update_worker, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));
            connect(update_worker, SIGNAL(remoteMessageBox(QString)), this, SLOT(OnRemoteMessageBox(QString)));
            connect(update_worker, SIGNAL(activateButtons()), this, SLOT(OnActivateButtons()));
            connect(update_worker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
            update_thread->start();
        }
    }
   
    else {
        QMessageBox info;
        info.setText("please activate update mode (power on with rec button pressed)");
        info.exec();
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
    }
}

void LordyLink::OnGetSetButton()
{
    
    if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && !usb_port->lordyphon_update_call()) {

        if (usb_port->clear_buffer())
            usb_port->close_usb_port();




        ui.QInstallProgressBar->reset();

        Worker* getSetWorker = new Worker;;

        USBThread* getSetThread = new USBThread;
        ui.QInstallLabel->setText("reading file");
        ui.QInstallLabel->show();
        ui.QInstallProgressBar->show();

        getSetWorker->moveToThread(getSetThread);
        connect(getSetWorker, SIGNAL(newItem(QString)), this, SLOT(refresh(QString)));
        connect(getSetThread, &QThread::started, getSetWorker, &Worker::get_eeprom_content);
        connect(getSetWorker, &Worker::finished, getSetThread, &QThread::quit);
        connect(getSetWorker, &Worker::finished, getSetWorker, &Worker::deleteLater);
        connect(getSetThread, &QThread::finished, getSetThread, &QThread::deleteLater);
        connect(getSetWorker, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
        connect(getSetWorker, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
        connect(getSetWorker, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));
        connect(getSetWorker, SIGNAL(remoteMessageBox(QString)), this, SLOT(OnRemoteMessageBox(QString)));
        connect(getSetWorker, SIGNAL(activateButtons()), this, SLOT(OnActivateButtons()));
        connect(getSetWorker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
        getSetThread->start();
        
        
       
    }
    else {

        QMessageBox info;
        info.setText("not possible in update mode");
        info.exec();



    }

}


void LordyLink::OnSendSetButton()
{

    if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && !usb_port->lordyphon_update_call()) {
        if(usb_port->clear_buffer())
            usb_port->close_usb_port();




        ui.QInstallProgressBar->reset();

        Worker* sendSetWorker = new Worker;

        USBThread* sendSetThread = new USBThread;
        ui.QInstallLabel->setText("sending file");
        ui.QInstallLabel->show();
        ui.QInstallProgressBar->show();

        sendSetWorker->moveToThread(sendSetThread);

        connect(sendSetThread, &QThread::started, sendSetWorker, &Worker::send_eeprom_content);
        connect(sendSetWorker, &Worker::finished, sendSetThread, &QThread::quit);
        connect(sendSetWorker, &Worker::finished, sendSetWorker, &Worker::deleteLater);
        connect(sendSetThread, &QThread::finished, sendSetThread, &QThread::deleteLater);
        connect(sendSetWorker, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
        connect(sendSetWorker, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
        connect(sendSetWorker, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));
        connect(sendSetWorker, SIGNAL(remoteMessageBox(QString)), this, SLOT(OnRemoteMessageBox(QString)));
        connect(sendSetWorker, SIGNAL(activateButtons()), this, SLOT(OnActivateButtons()));
        connect(sendSetWorker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
        sendSetThread->start();
    }
    else {

        QMessageBox info;
        info.setText("not possible in update mode");
        info.exec();



    }

}
void LordyLink::OnRemoteMessageBox(QString message)
{
    QMessageBox fromRemote;
    fromRemote.setText(message);
    fromRemote.exec();

}
void LordyLink::OnActivateButtons()
{
    bool ok = true;
    
    if(!ui.Q_UpdateLordyphonButton->isEnabled())
        ui.Q_UpdateLordyphonButton->setEnabled(ok);
    if (!ui.saveSetButton->isEnabled())
        ui.saveSetButton->setEnabled(ok);
    if (!ui.sendSetButton->isEnabled())
        ui.sendSetButton->setEnabled(ok);

    ui.QInstallProgressBar->hide();
    ui.QInstallLabel->hide();

}
void LordyLink::OnDeactivateButtons()
{
    bool ok = true;

    if (ui.QInstallProgressBar->isHidden())
        ui.QInstallProgressBar->show();
    if (ui.QInstallLabel->isHidden())
        ui.QInstallLabel->show();
       
    if (ui.Q_UpdateLordyphonButton->isEnabled())
        ui.Q_UpdateLordyphonButton->setDisabled(ok);
    if (ui.saveSetButton->isEnabled())
        ui.saveSetButton->setDisabled(ok);
    if (ui.sendSetButton->isEnabled())
        ui.sendSetButton->setDisabled(ok);



}
