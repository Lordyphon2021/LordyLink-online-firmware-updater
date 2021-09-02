#include "LordyLink.h"


#include <qstring.h>
#include<qlistwidget.h>
#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIODevice>
#include <QDialog>



using namespace std;


//GUI constructor, sets up file system, downloads all available firmware files
// and checks for lordyphon on the usb ports

LordyLink::LordyLink(QWidget *parent)
    : QMainWindow(parent)
{
    
    //setup gui
    ui.setupUi(this);
    ui.Q_UpdateLordyphonButton->setDisabled(true);
    //create model
    model = new QStandardItemModel();

    //create directories for sets, logs and firmware if they don't exist already
    QDir sets(QDir::homePath() + "/LordyLink/Sets");
    if (!sets.exists())
        sets.mkpath(".");
    
    QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
    if (!firmware.exists())
        firmware.mkpath(".");
    
    QDir versioncount(QDir::homePath() + "/LordyLink/versioncount");
    if (!versioncount.exists())
        versioncount.mkpath(".");
    
    QDir log(QDir::homePath() + "/LordyLink/log");
    if (!log.exists())
        log.mkpath(".");
    
    
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
    //connect qtableview with model
    ui.dirView->setModel(model);
    // set column size
    for (int col = 0; col < model->rowCount(); col++)
    {
        ui.dirView->setColumnWidth(col, 320);
    }
    
    //connect slots
    
    //double click to rename, item changed for rename end
    QObject::connect(ui.dirView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(renameStart(const QModelIndex)));
    QObject::connect(ui.dirView->model(), SIGNAL(itemChanged(QStandardItem*)), SLOT(renameEnd(QStandardItem*)));
    //single click to select item and copy path
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
                ctr = 0;
                QMessageBox error;
                error.setText("please connect Lordyphone");
                error.exec();
            }
            else {
                //open GUI on Lordyphon ID
                usb_port->open_lordyphon_port();
                if (usb_port->lordyphon_handshake()) {
                    ui.hardware_connected_label->setText("Lordyphon connected");
                    if (usb_port->clear_buffer())  //not sure if clear_buffer does exactly what it should
                        usb_port->close_usb_port(); //but the whole thing does seem less glitchy
                }
            }

        }//end: while (!usb_port->find_lordyphon_port())

            
    }//lordyphon was connected from the start
    else if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && usb_port->lordyphon_handshake()) {
        ui.hardware_connected_label->setText("Lordyphon connected");
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
    }
    
    
    //assume that there are 10 different versions on the ftp server, try and load all, filehandler will delete empty files
    //this might seem a bit clumsy but I couldn't find a way to get the ftp directory list with QT
    //(QFtp class is now obsolete)
    for (size_t i = 0; i < 10; ++i) {
        Downloader* download_from_ftp = new Downloader;
        connect(download_from_ftp, SIGNAL(download_finished()), this, SLOT(activate_install_button())); 
        //set up proper paths for downloader
        QString location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/firmware_versions/lordyphon_firmware_V1.0" + QString::number(i) + ".hex";
        QString path_first_part = QDir::homePath() + "/LordyLink/Firmware/lordyphon_firmware_V1.0"; //this is only part of the name
        QString path_complete = path_first_part + QString::number(i) + ".txt"; //version number added here...
        download_from_ftp->download(location, path_complete);  //pass to method
    }// it's rather slow but it works.
 }



 //"update firmware" button opens dialog, where all available releases are displayed in a QTableView window.
 // path to selected version will be passed to the parser class (in update_thread)
 //where it will be checked for validity and checksum errors (a line of a hexfile is called record)
 //and then stored in a container of plain record data vectors (std::vector<QByteArray>)
 //from which the data section will be transfered to lordyphon, one record at a time.
 //if lordyphon confirms correct checksum, next record will be sent until EOF.
 // -detailed error handling explained in parser class / update_worker...
 //when transmission is ok, lordyphon burns the data into the application section of its flash memory.
 //user is prompted to restart lordyphon
void LordyLink::OnUpdateButton()
{
    usb_port->find_lordyphon_port();
    usb_port->open_lordyphon_port();
  
   if (usb_port->lordyphon_update_call()) {  //lordyphon has to be in update mode for this, different response
        if (usb_port->clear_buffer())       //than regular handshake
            usb_port->close_usb_port();
        
        update_dialog = new QUpdateDialog;
        update_dialog->show();
        ui.QInstallLabel->hide();
        ui.QInstallProgressBar->hide();
        //pass path to firmware version as QString from update dialog
        connect(update_dialog, SIGNAL(selected_version(QString)), this, SLOT(set_firmware_path_from_dialog(QString)));
        int update_dialog_code = update_dialog->exec(); //evaluate dialog result
        
        if (update_dialog_code == QDialog::Accepted) {  //if update is confirmed, new thread is created
           //assemble path string with selected firmware version from dialog
            QString path_to_selected_firmware = QDir::homePath() +"/LordyLink/Firmware/" + update_dialog->get_firmware_version();
            Worker* update_worker = new Worker(path_to_selected_firmware, true); //the boolean is a dummy to have a dedicated ctor for this
            USBThread* update_thread = new USBThread;
            //setup gui feedback
            ui.QInstallLabel->show();
            ui.QInstallProgressBar->show();
            ui.QInstallProgressBar->valueChanged(0); //progress bar to zero
            //worker class contains most of lordylink business logic 
            update_worker->moveToThread(update_thread);
            //setup signals and slots
            //QT does cleanup when thread is finished
            connect(update_thread, &QThread::started, update_worker, &Worker::update);
            connect(update_worker, &Worker::finished, update_thread, &QThread::quit);
            connect(update_worker, &Worker::finished, update_worker, &Worker::deleteLater);
            connect(update_thread, &QThread::finished, update_thread, &QThread::deleteLater);
            //no GUI operations in thread, worker signals send GUI values to local slots
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
    }//end: if(usb_port->lordyphon_update_call()) 
   
    else { //lordyphon is not in update mode
        QMessageBox info;
        info.setText("please activate update mode (power on with rec button pressed)");
        info.exec();
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
    }
}




//this worker method requests a full memory dump from lordyphon
//and compares checksum from lordyphon with local checksum (summing up all bytes with an uint16_t
//-overflow is the same on both systems)
//if checksum is correct, file is saved in folder "Sets" and displayed in the QTableView Window
//where it can be renamed. default name is "saved set" and a time stamp.

void LordyLink::OnGetSetButton()
{   //always check if lordyphon is connected, no hot plugging detection implemented( QT doesn't offer one)
    if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && !usb_port->lordyphon_update_call()) {
        if (usb_port->clear_buffer())
           usb_port->close_usb_port();

        ui.QInstallProgressBar->reset();

        Worker* getSetWorker = new Worker;;
        USBThread* getSetThread = new USBThread;
        //update GUI
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
    else { //lordyphon is in update mode
        QMessageBox info;
        info.setText("not possible in update mode");
        info.exec();
    }
}

//this worker sends a selected set to lordyphon and overwrites the internal memory.
//user is made aware that this operation is destructive.

void LordyLink::OnSendSetButton()
{
   if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && !usb_port->lordyphon_update_call()) {
        if(usb_port->clear_buffer())
            usb_port->close_usb_port();

        //file selection must be valid
        if (selected_set != "") {
            QtSendDialog* send_dialog = new QtSendDialog;
            send_dialog->show();
            int send_dialog_code = send_dialog->exec();
            //user is sure
            if (send_dialog_code == QDialog::Accepted) {
                ui.QInstallProgressBar->reset();
                
                Worker* sendSetWorker = new Worker(selected_set);  //hand path to selected item to constructor

                USBThread* sendSetThread = new USBThread;
                //update GUI
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

        } //end: if (selected_set != "") 
        else {//selection is not valid
            QMessageBox info;
            info.setText("select a set");
            info.exec();
        }
    }
    else {
        //lordyphon in update mode
        QMessageBox info;
        info.setText("not possible in update mode");
        info.exec();
    }

}


//slot method implementation


//this message box is controlled from worker methods
void LordyLink::OnRemoteMessageBox(QString message)
{
    QMessageBox fromRemote;
    fromRemote.setText(message);
    fromRemote.exec();
}


//enable all buttons on main window
void LordyLink::OnActivateButtons()
{
    if(!ui.Q_UpdateLordyphonButton->isEnabled())
        ui.Q_UpdateLordyphonButton->setEnabled(true); //no idea why this needs a bool, it just does
    if (!ui.saveSetButton->isEnabled())                 
        ui.saveSetButton->setEnabled(true);
    if (!ui.sendSetButton->isEnabled())
        ui.sendSetButton->setEnabled(true);

    ui.QInstallProgressBar->hide();
    ui.QInstallLabel->hide();

}


//deactivate main window buttons
void LordyLink::OnDeactivateButtons()
{
    if (ui.QInstallProgressBar->isHidden())
        ui.QInstallProgressBar->show();
    if (ui.QInstallLabel->isHidden())
        ui.QInstallLabel->show();
    if (ui.Q_UpdateLordyphonButton->isEnabled())
        ui.Q_UpdateLordyphonButton->setDisabled(true);
    if (ui.saveSetButton->isEnabled())
        ui.saveSetButton->setDisabled(true);
    if (ui.sendSetButton->isEnabled())
        ui.sendSetButton->setDisabled(true);
}


//add new set to QTableView
void LordyLink::addNewSet(QString filename)
{
    QStandardItem* itemname = new QStandardItem(filename);
    itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);

    model->appendRow(QList<QStandardItem*>() << itemname);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("saved sets: "));
    //setup column size for better looks
    for (int col = 0; col < model->rowCount(); col++)
        ui.dirView->setColumnWidth(col, 320);
 }


//rename set function
void LordyLink::renameStart(const QModelIndex mindex)
{
    oldName = ui.dirView->model()->index(mindex.row(), 0).data().toString();
    qDebug() << "DoubleClicked: " << oldName;
}


//rename set function
void LordyLink::renameEnd(QStandardItem* item) 
{
   QString newname = QDir::homePath() + "/LordyLink/Sets/" + item->text();
    
    if (!newname.contains(".txt"))
        newname += ".txt";
     QFile::rename(QDir::homePath() + "/LordyLink/Sets/" + oldName , newname);  //set new name
}


//store selected setname in member variable
void LordyLink::selectItemToSend(const QModelIndex mindex)
{
    selected_set = ui.dirView->model()->index(mindex.row(), 0).data().toString();
}




