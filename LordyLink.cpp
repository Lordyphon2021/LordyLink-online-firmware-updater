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
    
    
   
    //setup gui
    ui.setupUi(this);
    //create model
    model = new QStandardItemModel();

    //create directories for sets and firmware if they don't exist already
    QDir sets(QDir::homePath() + "/LordyLink/Sets");
    if (!sets.exists())
        sets.mkpath(".");
    
    QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
    if (!firmware.exists())
        firmware.mkpath(".");
    
    
    
    // read sets from directory
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
    
    //connect qtableview with model
    ui.dirView->setModel(model);
    // set column size
    for (int col = 0; col < model->rowCount(); col++)
    {
        ui.dirView->setColumnWidth(col, 320);
    }
    
   //connect slots
    
    //double click to rename
    QObject::connect(ui.dirView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(renameStart(const QModelIndex)));
    QObject::connect(ui.dirView->model(), SIGNAL(itemChanged(QStandardItem*)), SLOT(renameEnd(QStandardItem*)));
    //single click to select item
    QObject::connect(ui.dirView, SIGNAL(clicked(const QModelIndex&)), SLOT(selectItemToSend(const QModelIndex)));
    //connect gui buttons
   
    QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(OnUpdateButton()));
   
    QObject::connect(ui.saveSetButton, SIGNAL(clicked()), this, SLOT(OnGetSetButton()));
    QObject::connect(ui.sendSetButton, SIGNAL(clicked()), this, SLOT(OnSendSetButton()));
   
    
    ui.hardware_connected_label->setText("       ");
    ui.QInstallLabel->hide();
    ui.QInstallProgressBar->hide();
	
    
    //GUI is only created if lordyphon is detected
    //without lordyphon user is forced to quit 
    
    //create serial port
    usb_port = new SerialHandler;
    
    if (!usb_port->find_lordyphon_port()) {
        dialog_no_hardware_found = new QNoHardwareDialog;

        int ctr = 0;  // alternates through different message boxes
        
        while (!usb_port->find_lordyphon_port()) {
            ui.hardware_connected_label->setText("Lordyphon disconnected");
            dialog_no_hardware_found->setWindowTitle("Lordyphon not found!");
            dialog_no_hardware_found->show();
            // open dialog
            int hardware_dialog_code = dialog_no_hardware_found->exec();

            //no lordyphon => quit
            if (hardware_dialog_code == QDialog::Rejected)
                exit(1);

            ctr++;

            if (ctr > 0 && !usb_port->find_lordyphon_port()) {
                QMessageBox error;
                error.setText("please connect Lordyphone");
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
    
    //get 10 files from server
    for (size_t version = 0; version < 10; ++version) {


        Filehandler* filehandler = new Filehandler;
        QString location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/firmware_versions/lordyphon_firmware_V1.0" + QString::number(version) + ".hex";
        QString file_path = QDir::homePath() + "/LordyLink/Firmware/lordyphon_firmware_V1.0";
        QString path = file_path + QString::number(version) + ".txt";



        filehandler->download(location, path);


    }
    
    
    
}




void LordyLink::OnUpdateButton()
{
    
    
  
    
    
    
    
    
    
    
    
    usb_port->find_lordyphon_port();
    usb_port->open_lordyphon_port();
  
   if (usb_port->lordyphon_update_call()) {  //lordyphon has to be in update mode for this
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
        
        update_dialog = new QUpdateDialog;
        update_dialog->show();
        ui.QInstallLabel->hide();
        ui.QInstallProgressBar->hide();

        connect(update_dialog, SIGNAL(selected(QString)), this, SLOT(get_path(QString)));
        
        
        int update_dialog_code = update_dialog->exec();




        if (update_dialog_code == QDialog::Accepted) {  //if update is confirmed, new thread is created
            qDebug() << "path in lordylink "  << QDir::homePath() +"/LordyLink/Firmware/" + update_dialog->get_firmware_path();
            QString path_to_selected_firmware = QDir::homePath() +"/LordyLink/Firmware/" + update_dialog->get_firmware_path();
            Worker* update_worker = new Worker(path_to_selected_firmware, true);
            USBThread* update_thread = new USBThread;
            //setup gui feedback
            ui.QInstallLabel->show();
            ui.QInstallProgressBar->show();
            ui.QInstallProgressBar->valueChanged(0); //progress bar to zero
            
            update_worker->moveToThread(update_thread);

            connect(update_thread, &QThread::started, update_worker, &Worker::update);
            connect(update_worker, &Worker::finished, update_thread, &QThread::quit);
            connect(update_worker, &Worker::finished, update_worker, &Worker::deleteLater);
            connect(update_thread, &QThread::finished, update_thread, &QThread::deleteLater);
            //no gui operations in thread, worker signals send values to local slots
            connect(update_worker, SIGNAL(ProgressBar_setMax(int)), this, SLOT(ProgressBar_OnsetMax(int)));
            connect(update_worker, SIGNAL(setLabel(QString)), this, SLOT(OnsetLabel(QString)));
            connect(update_worker, SIGNAL(ProgressBar_valueChanged(int)), this, SLOT(ProgressBar_OnValueChanged(int)));
            //one message box to serve all threads
            connect(update_worker, SIGNAL(remoteMessageBox(QString)), this, SLOT(OnRemoteMessageBox(QString)));
            //during threads "update", "getSet" and "sendSet" all pushbuttons are deactivated
            connect(update_worker, SIGNAL(activateButtons()), this, SLOT(OnActivateButtons()));
            //and reactivated when thread is finished
            connect(update_worker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
            
            update_thread->start(); 
        }
    } //end: if(usb_port->lordyphon_update_call()) 
   
    else { //lordyphon is not in update mode
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
        connect(getSetWorker, SIGNAL(newItem(QString)), this, SLOT(addNewSet(QString)));
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

        
       

       

       

        if (selected_set != "") {
            QtSendDialog* send_dialog = new QtSendDialog;
            send_dialog->show();
            int send_dialog_code = send_dialog->exec();
            
            if (send_dialog_code == QDialog::Accepted) {
                ui.QInstallProgressBar->reset();
                qDebug() << "constructor path: " << selected_set;
                Worker* sendSetWorker = new Worker(selected_set);  //hand path to selected item to constructor

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

        }
        else {
            QMessageBox info;
            info.setText("select a set");
            info.exec();
        }

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
void LordyLink::addNewSet(QString filename)
{


    QStandardItem* itemname = new QStandardItem(filename);
    itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);

    model->appendRow(QList<QStandardItem*>() << itemname);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("saved sets: "));

    for (int col = 0; col < model->rowCount(); col++)
    {
        ui.dirView->setColumnWidth(col, 320);
    }

}

void LordyLink::renameStart(const QModelIndex mindex)
{
    // alter Filename bestimmen
    oldName = ui.dirView->model()->index(mindex.row(), 0).data().toString();
    qDebug() << "DoubleClicked: " << oldName;



}
void LordyLink::renameEnd(QStandardItem* item) 
{
    
   
    // neuen Filenamen bestimmen
    qDebug() << item->text();
    // Datei umbenennen
    QString newname = QDir::homePath() + "/LordyLink/Sets/" + item->text();
    
    if (!newname.contains(".txt"))
        newname += ".txt";
    
    
    QFile::rename(QDir::homePath() + "/LordyLink/Sets/" + oldName , newname);  // neuen Filenamen bestimmen
    



}

void LordyLink::selectItemToSend(const QModelIndex mindex)
{


    selected_set = ui.dirView->model()->index(mindex.row(), 0).data().toString();
 
    


}



