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
#include <QScrollBar>
#include <QCloseEvent>



using namespace std;


//GUI constructor, sets up file system, downloads all available firmware files
// and checks for lordyphon on usb ports

LordyLink::LordyLink(QWidget* parent) : QMainWindow(parent)
{
    //setup gui

    ui.setupUi(this);
    
    // Stylesheet anwenden

    ui.delete_set_pushButton->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"    
        "    color: lightblue;"                 // Weißer Text
        "    border: 1px solid steelblue;"    // Dunklerer Rand für Buttons
        "    border-radius: 9px;"           // Abgerundete Ecken für Buttons
        "    padding: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: lightcoral;" 
        "    color: white; " 
        "}"
        "QPushButton:pressed {"
        "    background-color: #5a4e35;"    // Drück-Effekt für Buttons
        "}"
    );
    
    ui.sendSetButton->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"   
        "    color: lightblue;"               
        "    border: 1px solid steelblue;"   
        "    border-radius: 9px;"           
        "    padding: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: steelblue;" 
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #5a4e35;"    
        "}"
    );

    ui.saveSetButton->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"   
        "    color: lightblue;"               
        "    border: 1px solid steelblue;"   
        "    border-radius: 9px;"           
        "    padding: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: steelblue;"
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #5a4e35;"  
        "}"
    );

    ui.Q_UpdateLordyphonButton->setStyleSheet(
        "QPushButton {"
        "    background-color: lightblue;"   
        "    color: steelblue;"                
        "    border: 1px solid steelblue;"   
        "    border-radius: 9px;"         
        "    padding: 8px;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #b0bec5;"  
        "    color: #9e9e9e;"             
        "}"
        "QPushButton:hover {"
        "    background-color: steelblue;"  
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #5a4e35;"  
        "}"
    );
   
    
    // setup menuBar
    QAction* about_triggered = new QAction("about", this);
    QMenu* menu = menuBar()->addMenu("&info");
    menu->addAction(about_triggered);
    connect(about_triggered, SIGNAL(triggered()), this, SLOT(onAboutTriggered()));
    // hide update button on startup
    ui.Q_UpdateLordyphonButton->setDisabled(true);
    // create model for set files
    model = new QStandardItemModel();

    // create directories for sets, logs and firmware if they don't exist already
    QDir sets(QDir::homePath() + "/LordyLink/Sets");
    if (!sets.exists())
    {
        sets.mkpath(".");
    } 

    QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
    if (!firmware.exists())
    {
        firmware.mkpath(".");
    }

    QDir log(QDir::homePath() + "/LordyLink/log");
    if (!log.exists())
    {
        log.mkpath(".");
    }
       

    QDir download_dir(QDir::homePath() + "/LordyLink/downloads");
    if (!download_dir.exists())
    {
        download_dir.mkpath(".");
    } 

    // read sets from directory
    home = QDir::homePath() + "/LordyLink/Sets";
    QDir directory(home);
    QStringList txtfiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

    file_cleanup();  //delete downloads, firmware versions and empty set files on startup

    
    // setup scrollbar 
    QScrollBar* verticalScrollBar = ui.dirView->verticalScrollBar();
    
    verticalScrollBar->setStyleSheet("QScrollBar:vertical {"
        "  background: lightblue;"
        "  width: 7px;"
        "  border:3px lightblue;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: steelblue;"
        "  border-radius: 5px;"
        "}");

    ui.QInstallProgressBar->setStyleSheet("QProgressBar {"
        "border: lightblue;"
        "border-radius: 1px;"
        "background-color: transparent"
        "}"
        "QProgressBar {"
        "   text-align: right;"
        "   font-size: 11px;"
        "   color: white;"  // Textfarbe (kann je nach Qt-Version nicht immer wirken)
        "}"
        "QProgressBar::chunk {"
        "background-color: lightblue;" // Farbe des Fortschritts
        "border-radius: 5px;"
        "}");

  
    
    //show sets in QTableView
    foreach(QString filename, txtfiles) 
    {
        QStandardItem* itemname = new QStandardItem(filename);
        itemname->setBackground(QColor(Qt::transparent));
        itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
        
        model->appendRow(QList<QStandardItem*>() << itemname);
    }

    
    //connect qtableview with model
    ui.dirView->setModel(model);
     
    // set column size
    for (int col = 0; col < model->rowCount(); col++)
    {
        ui.dirView->setColumnWidth(col, 930);
    }
   
    //set background foto
   
    QPixmap bkgnd(QCoreApplication::applicationDirPath() + "/lordylink_background.jpg");
    qDebug() <<  QCoreApplication::applicationDirPath() + "/lordylink_background.jpg";
    bkgnd = bkgnd.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    bkgnd = bkgnd.scaled(this->size());
    QPalette palette;
    palette.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palette);

    ui.connection_label->setStyleSheet("QLabel { background-color : none; color : lightblue; }");
    ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightblue; }");
    //ui.dirView ->setStyleSheet("QLabel { background-color : transparent; color : white; }");
    
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

    //get firmware releases from server: all releases are in one .txt file,
    //TextfileExtracor class "unwraps" the file and creates individual firmware files for parser class

    try_download();
    download_timer = new QTimer(this);
    connect(download_timer, SIGNAL(timeout()), this, SLOT(try_download()));
    //try download every 5 seconds if file not complete
    //firmware now has fixed size so the combined file has to be a multiple of an individual firmware file
    download_timer->start(5000);
    
    //create serial port
    usb_port = new SerialHandler;
    
    checkConnection();
    
    //hotplug detection
    if (lordyphon_connected == true)
    {
        hot_plug_timer = new QTimer(this);
        connect(hot_plug_timer, SIGNAL(timeout()), this, SLOT(check_manufacturer_ID()));
        hot_plug_timer->start(2000);
    }
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
void LordyLink::OnUpdateButton()
{
    
    QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
        
    if (firmware.isEmpty())
    {
        show_messagebox("no internet connection!");
        return;
    }

    checkConnection();

    try
    {
        if (lordyphon_connected == true)
        {
            usb_port->open_lordyphon_port();
            delay(200);
            
            //lordyphon has to be in update mode for this
            if (usb_port->lordyphon_update_call())
            {  
                ui.hardware_connected_label->setText("Lordyphon updater on");

                if (usb_port->clear_buffer())
                {
                    usb_port->close_usb_port();
                }

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
                    connect(update_worker, SIGNAL(setAbortedThreadFlag()), this, SLOT(OnsetAbortedThreadFlag()));
                    connect(update_worker, SIGNAL(clearAbortedThreadFlag()), this, SLOT(OnclearAbortedThreadFlag()));
                    //during threads "update", "getSet" and "sendSet" all pushbuttons are deactivated
                    connect(update_worker, SIGNAL(activateButtons()), this, SLOT(OnActivateButtons()));
                    connect(update_worker, SIGNAL(activateButtons()), this, SLOT(hotplugtimer_on()));
                    //and reactivated when thread is finished
                    connect(update_worker, SIGNAL(deactivateButtons()), this, SLOT(OnDeactivateButtons()));
                    connect(update_worker, SIGNAL(deactivateButtons()), this, SLOT(hotplugtimer_off()));
                    connect(ui.abort_pushButton, &QPushButton::clicked, update_thread, &QThread::requestInterruption);
                    //connect(update_worker, SIGNAL(deactivateAbortButton()), this, SLOT(OnHideAbortButton()));
                    connect(ui.abort_pushButton, &QPushButton::clicked, update_thread, &QThread::requestInterruption);
                    update_mode = true;
                    update_thread->start();
                }
            }
            else 
            //lordyphon is not in update mode
            { 
                
                ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
                ui.hardware_connected_label->setText("Lordyphon updater off");
                delay(1000);
                show_messagebox("please activate update mode (hold stop button and press rec for 2 sec)");
                update_mode = false;
                
                if(usb_port->clear_buffer())
                {
                    usb_port->close_usb_port();
                }
            }
        }
    }
    catch (exception& e) 
    {
        ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
        ui.hardware_connected_label->setText("Lordyphon disconnected");
        show_messagebox(e.what());
    }
}
 
//this method starts a thread, which requests a full memory dump from lordyphon
//and compares checksum from lordyphon with local checksum (summing up all bytes with an uint16_t
//-overflow is the same on both systems)
//if checksum is correct, file is saved in folder "Sets" and displayed in the QTableView Window
//where it can be renamed and/ or deleted. default name is "saved set" and a time stamp.

void LordyLink::OnGetSetButton()
{  
    if (ui.Q_UpdateLordyphonButton->isEnabled())
    {
        ui.Q_UpdateLordyphonButton->setDisabled(true);
    }

    checkConnection();
        
    try
    {
        if (lordyphon_connected == true)
        {
            usb_port->open_lordyphon_port();

            if (!usb_port->lordyphon_update_call())
            {
                if (usb_port->clear_buffer())
                {
                    usb_port->close_usb_port();
                }

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
                connect(ui.abort_pushButton, &QPushButton::clicked, getSetThread, &QThread::requestInterruption);

                getSetThread->start();
            }
            else
            { //lordyphon is in update mode or midi mode
                usb_port->close_usb_port();
                show_messagebox("Exit Updater or activate USB on Lordyphon!");
            }
        }
    }
    catch (exception& e)
    {
        
        ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
        ui.hardware_connected_label->setText("Lordyphon disconnected    ");
        show_messagebox(e.what());
    }
}

//this method sends a selected set in a new thread to lordyphon and overwrites the internal memory.
//user is made aware that this operation is destructive.
void LordyLink::OnSendSetButton()
{
    checkConnection();

    if (ui.Q_UpdateLordyphonButton->isEnabled())
    {
        ui.Q_UpdateLordyphonButton->setDisabled(true);
    }

    if (lordyphon_connected == true)
    {
        try
        {
            usb_port->open_lordyphon_port();

            //this makes shure controller is NOT in update mode
            if (!usb_port->lordyphon_update_call())
            {
                if (usb_port->clear_buffer())
                {
                    usb_port->close_usb_port();
                } 
                ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightblue; }");
                ui.hardware_connected_label->setText("Lordyphon connected");

                //file selection must be valid
                if (selected_set != "") 
                {
                    //user dialog
                    QtSendDialog* send_dialog = new QtSendDialog;
                    send_dialog->show();
                    int send_dialog_code = send_dialog->exec();
                    
                    //user is sure
                    if (send_dialog_code == QDialog::Accepted) 
                    {
                        ui.QInstallProgressBar->reset();
                        //hand path to selected item to constructor
                        Worker* sendSetWorker = new Worker(selected_set);

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
                } 
                else 
                {
                    show_messagebox("please select a set!     ");
                }
            }
            else
            {
                //lordyphon in update mode
                usb_port->close_usb_port();
                show_messagebox("Exit Updater or activate USB on Lordyphon!");
                ui.hardware_connected_label->setText("Lordyphon updater on");
            }
        }
        catch (exception& e)
        {
            show_messagebox(e.what());
        }
    }
    else
    {
        show_messagebox("lordyphon not found");
    }
} 


//slot method implementations

//Menu "about"
void LordyLink::onAboutTriggered() 
{
    QStringList list;
    QString github_link = "https://github.com/Lordyphon2021/LordyLink-online-firmware-updater";
    QString gnu_link = "http://www.gnu.org/licenses/gpl-2.0.html";
    
    list << "LordyLink (c)2025 by Stefan Deisenberger"
         << ""
         << "check it out on github :\n"
         << github_link
         << ""
         << "This application uses Qt, a software framework developed by The Qt Company and others.\n"
         << "Qt is licensed under the GNU General Public License(GPL) version 2 or later."
         << ""
         << "You may obtain a copy of the GPL at\n "
         << gnu_link;
        
    QString info = list.join("\n");
       
    show_messagebox(info, "FCK PTN");
}

//this message box is controlled from worker methods
void LordyLink::OnRemoteMessageBox(QString message)
{
    QFont font("Lucida Typewriter", 8, QFont::Bold);
    QMessageBox fromRemote;
    fromRemote.setFont(font);
    fromRemote.setText(message);
    fromRemote.exec();
}


//enable all buttons in main window, hide abort button
void LordyLink::OnActivateButtons()
{
    ui.abort_pushButton->hide();
    delay(2000);
    
    if (!ui.Q_UpdateLordyphonButton->isEnabled())
    {
        ui.Q_UpdateLordyphonButton->setEnabled(true);
    }
      
    if (!ui.saveSetButton->isEnabled())
    {
        ui.saveSetButton->setEnabled(true);
    }
       
    if (!ui.sendSetButton->isEnabled())
    {
        ui.sendSetButton->setEnabled(true);
    }
       
    if (!ui.delete_set_pushButton->isEnabled())
    {
        ui.delete_set_pushButton->setEnabled(true);
    } 
    
    delay(200);
    ui.QInstallProgressBar->hide();
    ui.QInstallLabel->hide();
}

//deactivate main window buttons, show abort button
void LordyLink::OnDeactivateButtons()
{
    ui.abort_pushButton->show();
    
    if (ui.QInstallProgressBar->isHidden())
    {
        ui.QInstallProgressBar->show();
    }
    
    if (ui.QInstallLabel->isHidden())
    {
        ui.QInstallLabel->show();
    }
        
    if (ui.Q_UpdateLordyphonButton->isEnabled())
    {
        ui.Q_UpdateLordyphonButton->setDisabled(true);
    }
        
    if (ui.saveSetButton->isEnabled())
    {
        ui.saveSetButton->setDisabled(true);
    }
       
    if (ui.sendSetButton->isEnabled())
    {
        ui.sendSetButton->setDisabled(true);
    }
       
    if (ui.delete_set_pushButton->isEnabled())
    {
        ui.delete_set_pushButton->setDisabled(true);
    }
}


//add new set to QTableView
void LordyLink::addNewSet(QString filename)
{
    QStandardItem* itemname = new QStandardItem(filename);

    itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
    itemname->setBackground(QColor(Qt::transparent));
    
    model->appendRow(QList<QStandardItem*>() << itemname);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("saved sets: "));
    
    //setup column size for better looks
    for (int col = 0; col < model->rowCount(); col++)
    {
        ui.dirView->setColumnWidth(col, 930);
    }
 }


//rename set function
void LordyLink::renameStart(const QModelIndex mindex)
{
    oldName = ui.dirView->model()->index(mindex.row(), 0).data().toString();
}


//rename set function
void LordyLink::renameEnd(QStandardItem* item) 
{
    QString newname = QDir::homePath() + "/LordyLink/Sets/" + item->text();
    
    if (!newname.contains(".txt"))
    {
        newname += ".txt";
    }
        
    QFile::rename(QDir::homePath() + "/LordyLink/Sets/" + oldName , newname);  //set new name
}


//store selected setname in member variable
void LordyLink::selectItemToSend(const QModelIndex mindex)
{
    selected_set = ui.dirView->model()->index(mindex.row(), 0).data().toString();
}

void LordyLink::selectItemToDelete(const QModelIndex mindex) 
{
    to_delete = ui.dirView->model()->index(mindex.row(), 0).data().toString();
}

void LordyLink::deleteSet() 
{
    if(to_delete != "")
    {
        DeleteDialog* delete_dialog = new DeleteDialog;
        delete_dialog->show();
        int delete_dialog_code = delete_dialog->exec();

        //user is sure
        if (delete_dialog_code == QDialog::Accepted)
        {

            delete model;
            model = new QStandardItemModel();
            QFile::remove(QDir::homePath() + "/LordyLink/Sets/" + to_delete);

            home = QDir::homePath() + "/LordyLink/Sets";
            QDir directory(home);
            QStringList txtfiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

            foreach(QString filename, txtfiles) 
            {
                QStandardItem* itemname = new QStandardItem(filename);
                itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
                itemname->setBackground(QColor(Qt::transparent));
                model->appendRow(QList<QStandardItem*>() << itemname);
            }
            
            //connect qtableview with model
            ui.dirView->setModel(model);

            // set column size
            for (int col = 0; col < model->rowCount(); col++) 
            {
                ui.dirView->setColumnWidth(col, 930);
            }
        }
    }
    else
    {
        show_messagebox("No set selected");
    }
}


//download firmware_versions.txt from ftp_server, set off by QTimer and in CTOR
void LordyLink::try_download() 
{
    //set up dir
    QDir down= QDir::homePath() + "/LordyLink/downloads";
    QDir firm = QDir::homePath() + "/LordyLink/Firmware";
    QFile firmware_versions(QDir::homePath() + "/LordyLink/downloads/firmware_versions.txt");
    
    Downloader* download_from_ftp = new Downloader;
    //get status from downloader class
    connect(download_from_ftp, SIGNAL(download_status(bool)), this, SLOT(on_download_status(bool)));
    connect(download_from_ftp, SIGNAL(download_status_msg(QString)), this, SLOT(on_download_status_message(QString)));
    
    
    
    if (download_done == false && ( down.isEmpty() || firmware_versions.size() % firmware_size != 0 ) ) //this bool is set via signal
    { 
        ui.connection_label->setText("connecting to server");
        ui.Q_UpdateLordyphonButton->setDisabled(true); //button inactive until file is downloaded and extracted
        
        QString ftp_location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/firmware_versions/firmware_versions.txt";
        QString to_downloaded_file= QDir::homePath() + "/LordyLink/downloads/firmware_versions.txt";
       
        download_from_ftp->download(ftp_location, to_downloaded_file);
        if (downloader_message.isEmpty())
        {
            ui.connection_label->setText("connecting to server");
        }  
        else 
        {
            ui.connection_label->setText(downloader_message);
            downloader_message.clear();
        }
    }
    else if(download_done == true && firmware_versions.size() % firmware_size == 0)
    {
        //file is in folder "downloads" now, and has the correct size 
        ui.connection_label->setText("downloading...");
        delay(1000);
        
        TextfileExtractor extractor;
        
        ui.connection_label->setText("extracting...");
        delay(600);
        
        //start unzipper method
        if (extractor.unwrap(QDir::homePath() + "/LordyLink/downloads/firmware_versions.txt") == true)
        {
            ui.connection_label->setText("extracting done...");
            delay(600);
            ui.connection_label->setText("download success");
            delay(600);
            ui.connection_label->hide();
        }
        //activate upddate button only if unzipper returns true
        download_done = false;
        ui.Q_UpdateLordyphonButton->setEnabled(true);
    }
    else if (firmware_versions.size() % firmware_size != 0) //handle incomplete downloads
    {  
        ui.connection_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
        ui.connection_label->setText("download incomplete");
        download_done = false;
    }
}


void LordyLink::hotplugtimer_on()
{
    delay(500);
   
    if (!hot_plug_timer->isActive())
    {
        hot_plug_timer->start(2000);
    } 
}


void LordyLink::hotplugtimer_off() 
{
    
    if (hot_plug_timer->isActive()) 
    {
        hot_plug_timer->stop();
        delay(500);
    }
}

void LordyLink::check_for_lordyphon() 
{
    if (!usb_port->find_lordyphon_port()) 
    {
        ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
        ui.hardware_connected_label->setText("Lordyphon disconnected");
        show_messagebox("Lordyphon disconnected!");
    }
    else
    {
        ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightblue; }");
        ui.hardware_connected_label->setText("Lordyphon connected");
    }
}

//EXIT APP IF MAIN WINDOW WAS CLOSED 
void QWidget::closeEvent(QCloseEvent* event) 
{
    event->accept();  
    SerialHandler* usb_port = new SerialHandler;
    
    try
    {
        usb_port->find_lordyphon_port();
        if (!usb_port->lordyphon_port_is_open())
        {
            usb_port->open_lordyphon_port();
        }
        
        usb_port->quit_message();
        usb_port->close_usb_port();
    }
    catch (exception& e)
    {
        if (usb_port->lordyphon_port_is_open())
        {
            usb_port->close_usb_port();
        }
        
        exit(0);
    }

    exit(0);
}



void LordyLink::check_manufacturer_ID() 
{
    if (!usb_port->check_with_manufacturer_ID()) 
    { 
        show_messagebox("Lordyphon disconnected!", "Proceed", true);
        ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
        ui.hardware_connected_label->setText("Lordyphon disconnected");
    }
    else
    {
        ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightblue; }");
        ui.hardware_connected_label->setText("Lordyphon connected");
    }
}

void LordyLink::checkConnection()
{
    try
    {
        do
        {
            if (usb_port->find_lordyphon_port())
            {
                if (!usb_port->lordyphon_port_is_open())
                {
                    if (!usb_port->open_lordyphon_port())
                    {
                        show_messagebox("USB error!", "Proceed", true);
                    }
                }
                
                if (usb_port->lordyphon_handshake() == true || usb_port->lordyphon_update_call() == true)
                {
                    lordyphon_connected = true;
                    ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightblue; }");
                    ui.hardware_connected_label->setText("Lordyphon connected");

                    if (usb_port->clear_buffer())
                    {
                        usb_port->close_usb_port();
                    }
                }
                else
                {
                    // (mssge str, button text, quit button on/off)
                    ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
                    ui.hardware_connected_label->setText("Lordyhon USB mode off");
                    show_messagebox("Activate USB on lordyphon ( Press global and looper button)", "Proceed", true);
                  

                    if (usb_port->clear_buffer())
                    {
                        usb_port->close_usb_port();
                    }
                    
                    lordyphon_connected = false;
                    delay(3000);
                }
            }
            else
            {
                
                ui.hardware_connected_label->setStyleSheet("QLabel { background-color : none; color : lightcoral; }");
                ui.hardware_connected_label->setText("Lordyphon not connected");
                show_messagebox("Lordyphon not connected", "Proceed", true);
                lordyphon_connected = false;
            }

        } while (lordyphon_connected == false);
    }
    catch (exception& e)
    {
        lordyphon_connected = false;
        show_messagebox(e.what());
    }
}

