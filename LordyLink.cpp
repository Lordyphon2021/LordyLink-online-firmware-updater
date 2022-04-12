#include "LordyLink.h"
#include <qstring.h>
#include<qlistwidget.h>
#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIODevice>
#include <QDialog>
#include "DeleteDialog.h"
#include <qpalette.h>
#include <qstandardpaths>
#include "TextfileExtractor.h"


using namespace std;


//GUI constructor, sets up file system, downloads all available firmware files
// and checks for lordyphon on usb ports

LordyLink::LordyLink(QWidget *parent) : QMainWindow(parent) {
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
    
    QDir log(QDir::homePath() + "/LordyLink/log");
    if (!log.exists())
        log.mkpath(".");
   
    QDir download_dir(QDir::homePath() + "/LordyLink/downloads");
    if (!download_dir.exists())
        download_dir.mkpath(".");

    // read sets from directory
    home = QDir::homePath() + "/LordyLink/Sets";
    QDir directory(home);
    QStringList txtfiles = directory.entryList(QStringList() << "*.txt", QDir::Files);
    
    foreach(QString filename, txtfiles) {
        qDebug() << filename;
        QStandardItem* itemname = new QStandardItem(filename);
        itemname->setBackground(QColor(Qt::lightGray));
        itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
        model->appendRow(QList<QStandardItem*>() << itemname);
    }
    //connect qtableview with model
    ui.dirView->setModel(model);
     
    // set column size
    for (int col = 0; col < model->rowCount(); col++){
        ui.dirView->setColumnWidth(col, 320);
        
    }
   // resize update button
    
    QSize q;
    q.setWidth(400);
    q.setHeight(50);
    ui.Q_UpdateLordyphonButton->setFixedSize(q);
    QFont font("Lucida Typewriter", 20, QFont::Bold);
   
    ui.Q_UpdateLordyphonButton->setFont(font);
    ui.Q_UpdateLordyphonButton->setPalette(QColor(Qt::lightGray));
    ui.Q_UpdateLordyphonButton->setText("update firmware        ");
    
    //set background foto
    
    QPixmap bkgnd(QCoreApplication::applicationDirPath() + "/lordylink_background.jpeg");
    bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palette);

    ui.connection_label->setStyleSheet("QLabel { background-color : none; color : white; }");
    ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : white; }");
    ui.dirView ->setStyleSheet("QLabel { background-color : none; color : white; }");
    //connect slots
    
    //double click to rename, item changed for rename end
    QObject::connect(ui.dirView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(renameStart(const QModelIndex)));
    QObject::connect(ui.dirView->model(), SIGNAL(itemChanged(QStandardItem*)), SLOT(renameEnd(QStandardItem*)));
    //single click to select item and copy path
    QObject::connect(ui.dirView, SIGNAL(clicked(const QModelIndex&)), SLOT(selectItemToSend(const QModelIndex)));
    QObject::connect(ui.dirView, SIGNAL(clicked(const QModelIndex&)), SLOT(selectItemToDelete(const QModelIndex)));
    //connect gui buttons
    QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), this, SLOT(OnUpdateButton()));
    QObject::connect(ui.saveSetButton, SIGNAL(clicked()), this, SLOT(OnGetSetButton()));
    QObject::connect(ui.sendSetButton, SIGNAL(clicked()), this, SLOT(OnSendSetButton()));
    QObject::connect(ui.delete_set_pushButton, SIGNAL(clicked()), this, SLOT(deleteSet()));
   
    ui.hardware_connected_label->setText("       ");
    ui.QInstallLabel->hide();
    ui.QInstallProgressBar->hide();
    ui.abort_pushButton->hide();
    
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
               
                QMessageBox error(QMessageBox::Information, "Information", "");
                QFont font("Lucida Typewriter", 8, QFont::Bold);
                error.setFont(font);
                error.setText("please connect Lordyphon...ain't no good without a Lordyphon! ");
                error.exec();
            }
            else {
                //open GUI on Lordyphon ID
                usb_port->open_lordyphon_port();
                if (usb_port->lordyphon_handshake()) {
                    ui.hardware_connected_label->setText("Lordyphon connected");
                    if (usb_port->clear_buffer())  
                        usb_port->close_usb_port(); 
                }
            }
         }//end: while (!usb_port->find_lordyphon_port())
        
   }//lordyphon found
    else if (usb_port->find_lordyphon_port() && usb_port->open_lordyphon_port() && usb_port->lordyphon_handshake()) {
        ui.hardware_connected_label->setText("Lordyphon connected");
        if (usb_port->clear_buffer())
            usb_port->close_usb_port();
    }

    
    //assume that there are 10 firmware files on the ftp server, try and load all, downloader will delete empty files
    //TODO: display available files on server
    
    try_download();
    download_timer = new QTimer(this);
    connect(download_timer, SIGNAL(timeout()), this, SLOT(try_download()));
    download_timer->start(5000);
    
    //hotplug detection
    hot_plug_timer = new QTimer(this);
    connect(hot_plug_timer, SIGNAL(timeout()), this, SLOT(check_manufacturer_ID()));
    hot_plug_timer->start(2000);
    
}


 //"update firmware" button opens dialog, where all available releases are displayed in a QTableView window.
 //path to selected version will be passed to the parser object in a new thread via signal (in update_thread)
 //where it will be checked for validity and checksum errors (a line of an intel hexfile is called record)
 //and then stored in a container of plain record-data QByteArrays (QVector<QByteArray>)
 //from which the data section will be transfered to lordyphon, one record at a time.
 //if lordyphon confirms correct checksum, next record will be sent until EOF.
 // -error handling explained in parser class / update_worker...
 //when transmission is ok, lordyphon burns the firmware data into the application section of its flash memory.
 //user is prompted to restart lordyphon
void LordyLink::OnUpdateButton(){

    try {
        QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
        
        if (firmware.isEmpty()) {
            QMessageBox info;
            QFont font("Lucida Typewriter", 8, QFont::Bold);
            info.setFont(font);
            info.setText("no internet connection!");
            info.exec();
            return;
        }
        usb_port->open_lordyphon_port();

        if (usb_port->lordyphon_update_call()) {  //lordyphon has to be in update mode for this
            ui.hardware_connected_label->setText("Lordyphon updater on");
            
            if (usb_port->clear_buffer())      
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
                QString path_to_selected_firmware = QDir::homePath() + "/LordyLink/Firmware/" + firmware_path;
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
                connect(update_worker, SIGNAL(activateButtons()), this, SLOT(hotplugtimer_on()));
                //and reactivated when thread is finished
                connect(update_worker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
                connect(update_worker, SIGNAL(deactivateButtons()), this, SLOT(hotplugtimer_off()));
                connect(ui.abort_pushButton, &QPushButton::clicked, update_thread, &QThread::requestInterruption);
                connect(update_worker, SIGNAL(deactivateAbortButton()), this, SLOT(OnHideAbortButton()));
                update_thread->start();
            }
            
        }//end: if(usb_port->lordyphon_update_call()) 
       
        else { //lordyphon is not in update mode
            QMessageBox info;
            QFont font("Lucida Typewriter", 8, QFont::Bold);
            info.setFont(font);
            info.setText("please activate update mode (power on with rec button pressed)");
            ui.hardware_connected_label->setText("Lordyphon updater off");
            info.exec();
           
            if (usb_port->clear_buffer())
                usb_port->close_usb_port();
        } 
    }
    catch (exception& e) {
        QFont Font("Lucida Typewriter", 8, QFont::Bold);
        QMessageBox error;
        error.setFont(Font);
        error.setText(e.what());
        error.exec();
        ui.hardware_connected_label->setText("Lordyphon disconnected");
    }
}
 
//this method starts a thread, which requests a full memory dump from lordyphon
//and compares checksum from lordyphon with local checksum (summing up all bytes with an uint16_t
//-overflow is the same on both systems)
//if checksum is correct, file is saved in folder "Sets" and displayed in the QTableView Window
//where it can be renamed and/ or deleted. default name is "saved set" and a time stamp.

void LordyLink::OnGetSetButton()
{  
    try {

        if (usb_port->find_lordyphon_port())
            usb_port->open_lordyphon_port();

        else 
            throw runtime_error("Lordyphon not connected");
        
        if (!usb_port->lordyphon_update_call()) {
            if (usb_port->clear_buffer())
                usb_port->close_usb_port();

            ui.QInstallProgressBar->reset();
            ui.hardware_connected_label->setText("Lordyphon connected");
            Worker* getSetWorker = new Worker;
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
            connect(getSetWorker, SIGNAL(activateButtons()), this, SLOT(hotplugtimer_on()));
            connect(getSetWorker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
            connect(getSetWorker, SIGNAL(deactivateButtons()), this, SLOT(hotplugtimer_off()));
            
            //connect(this, &LordyLink::abort , getSetWorker, &Worker::finished);
            
            connect(ui.abort_pushButton, &QPushButton::clicked, getSetThread, &QThread::requestInterruption);
            getSetThread->start();
        }
        else { //lordyphon is in update mode
            QMessageBox info;
            QFont font("Lucida Typewriter", 8, QFont::Bold);
            info.setFont(font);
            info.setText("not possible in update mode   ");
            info.exec();
        }
   
    }catch (exception& e) {

        QFont Font("Lucida Typewriter", 8, QFont::Bold);
        QMessageBox error;
        error.setFont(Font);
        error.setText(e.what());
        error.exec();
        ui.hardware_connected_label->setText("Lordyphon disconnected    ");
    }
}

//this method sends a selected set in a new thread to lordyphon and overwrites the internal memory.
//user is made aware that this operation is destructive.

void LordyLink::OnSendSetButton()
{
   try {

        if (usb_port->find_lordyphon_port())
            usb_port->open_lordyphon_port();

        else 
            throw runtime_error("Lordyphon not connected");

        if (!usb_port->lordyphon_update_call()) {
            if (usb_port->clear_buffer())
                usb_port->close_usb_port();

            ui.hardware_connected_label->setText("Lordyphon connected");

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
                    connect(sendSetWorker, SIGNAL(activateButtons()), this, SLOT(hotplugtimer_on()));
                    connect(sendSetWorker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
                    connect(sendSetWorker, SIGNAL(deactivateButtons()), this, SLOT(hotplugtimer_off()));
                    connect(ui.abort_pushButton, &QPushButton::clicked, sendSetThread, &QThread::requestInterruption);
                    sendSetThread->start();
                }

            } //end: if (selected_set != "") 
            else {//selection is not valid
                QMessageBox info;
                QFont font("Lucida Typewriter", 8, QFont::Bold);
                info.setFont(font);
                info.setText("please select a set!     ");
                info.exec();
            }
        }
        else{
            //lordyphon in update mode
            QMessageBox info;
            QFont font("Lucida Typewriter", 8, QFont::Bold);
            info.setFont(font);
            info.setText("not possible in update mode    ");
            info.exec();
        }
    
    }catch (exception& e) {

        
        QMessageBox error;
        QFont font("Lucida Typewriter", 8, QFont::Bold);
        error.setFont(font);
        error.setText(e.what());
        error.exec();
        ui.hardware_connected_label->setText("Lordyphon disconnected");
    }
}


//slot method implementations

//this message box is controlled from worker methods
void LordyLink::OnRemoteMessageBox(QString message){
    
    QFont font("Lucida Typewriter", 8, QFont::Bold);
    QMessageBox fromRemote;
    fromRemote.setFont(font);
    fromRemote.setText(message);
    fromRemote.exec();
}

//enable all buttons in main window, hide abort button
void LordyLink::OnActivateButtons(){
    
    qDebug() << "activate Buttons";
    ui.abort_pushButton->hide();

    delay(2000);
    
    if(!ui.Q_UpdateLordyphonButton->isEnabled())
        ui.Q_UpdateLordyphonButton->setEnabled(true); 
    if (!ui.saveSetButton->isEnabled())                 
        ui.saveSetButton->setEnabled(true);
    if (!ui.sendSetButton->isEnabled())
        ui.sendSetButton->setEnabled(true);
    if (!ui.delete_set_pushButton->isEnabled())
        ui.delete_set_pushButton->setEnabled(true);
    
    delay(200);
    ui.QInstallProgressBar->hide();
    ui.QInstallLabel->hide();

    
}

//deactivate main window buttons, show abort button
void LordyLink::OnDeactivateButtons(){
    
    ui.abort_pushButton->show();
    qDebug() << "deactivate Buttons";
    
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
    if (ui.delete_set_pushButton->isEnabled())
        ui.delete_set_pushButton->setDisabled(true);
   
}


//add new set to QTableView
void LordyLink::addNewSet(QString filename){
    
    QStandardItem* itemname = new QStandardItem(filename);
    itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
    itemname->setBackground(QColor(Qt::lightGray));
    model->appendRow(QList<QStandardItem*>() << itemname);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("saved sets: "));
    //setup column size for better looks
    for (int col = 0; col < model->rowCount(); col++)
        ui.dirView->setColumnWidth(col, 320);
 }


//rename set function
void LordyLink::renameStart(const QModelIndex mindex){
    
    oldName = ui.dirView->model()->index(mindex.row(), 0).data().toString();
    qDebug() << "DoubleClicked: " << oldName;
}


//rename set function
void LordyLink::renameEnd(QStandardItem* item) {
   
    QString newname = QDir::homePath() + "/LordyLink/Sets/" + item->text();
    
    if (!newname.contains(".txt"))
        newname += ".txt";
    QFile::rename(QDir::homePath() + "/LordyLink/Sets/" + oldName , newname);  //set new name
}


//store selected setname in member variable
void LordyLink::selectItemToSend(const QModelIndex mindex){
    
    selected_set = ui.dirView->model()->index(mindex.row(), 0).data().toString();
}

void LordyLink::selectItemToDelete(const QModelIndex mindex) {

    to_delete = ui.dirView->model()->index(mindex.row(), 0).data().toString();
}

void LordyLink::deleteSet() {

    DeleteDialog* delete_dialog = new DeleteDialog;
    delete_dialog->show();
    int delete_dialog_code = delete_dialog->exec();
    
    //user is sure
    if (delete_dialog_code == QDialog::Accepted) {

        delete model;
        model = new QStandardItemModel();
        QFile::remove(QDir::homePath() + "/LordyLink/Sets/" + to_delete);  //set new name

        home = QDir::homePath() + "/LordyLink/Sets";
        QDir directory(home);
        QStringList txtfiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

        foreach(QString filename, txtfiles) {
            qDebug() << filename;
            QStandardItem* itemname = new QStandardItem(filename);
            itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
            itemname->setBackground(QColor(Qt::lightGray));
            model->appendRow(QList<QStandardItem*>() << itemname);
        }
        //connect qtableview with model
        ui.dirView->setModel(model);

        // set column size
        for (int col = 0; col < model->rowCount(); col++) {
            ui.dirView->setColumnWidth(col, 320);
        }
    }
}



void LordyLink::try_download() {
    
    QDir down= QDir::homePath() + "/LordyLink/downloads";
    QDir firm = QDir::homePath() + "/LordyLink/Firmware";
    Downloader* download_from_ftp = new Downloader;
    
    connect(download_from_ftp, SIGNAL(download_status(bool)), this, SLOT(on_download_status(bool)));
   
    
    if (download_done == false) {
        
        ui.Q_UpdateLordyphonButton->setDisabled(true);
        QString ftp_location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/firmware_versions/firmware_versions.txt";
        QString to_downloaded_file= QDir::homePath() + "/LordyLink/downloads/firmware_versions.txt";
        download_from_ftp->download(ftp_location, to_downloaded_file);  //pass to method
        ui.connection_label->setText("attempting download...");
    }
    else if(download_done == true && firm.isEmpty()){
        ui.connection_label->setText("downloading...");
        delay(1000);
        TextfileExtractor textextract;
        ui.connection_label->setText("extracting...");
        delay(600);
        if (textextract.unzipper(QDir::homePath() + "/LordyLink/downloads/firmware_versions.txt") == true) {
            ui.connection_label->setText("extracting done...");
            delay(600);
            ui.connection_label->setText("download success");
            delay(600);
            ui.connection_label->setText("updater ready");
        }
        ui.Q_UpdateLordyphonButton->setEnabled(true);
    }
}


void LordyLink::hotplugtimer_on() {

    qDebug() << " hotplugtimer on";
    delay(500);
   
    if(!hot_plug_timer->isActive())
        hot_plug_timer->start(2000);

}


void LordyLink::hotplugtimer_off() {

    qDebug() << "hotplugtimer off";
    
    if (hot_plug_timer->isActive()) {
        hot_plug_timer->stop();
        delay(500);
    }
}

void LordyLink::check_for_lordyphon() {
    
    if (!usb_port->find_lordyphon_port()) {
        ui.hardware_connected_label->setText("Lordyphon disconnected");
        QMessageBox error;
        error.setText("Lordyphon disconnected!");
        error.exec();

    }
    else
        ui.hardware_connected_label->setText("Lordyphon connected");
}


void LordyLink::check_manufacturer_ID() {
   
    if (!usb_port->check_with_manufacturer_ID()) {
        ui.hardware_connected_label->setText("Lordyphon disconnected");
        QMessageBox error;
        error.setText("Lordyphon disconnected!");
        error.exec();

    }
    else
        ui.hardware_connected_label->setText("Lordyphon connected");
}



