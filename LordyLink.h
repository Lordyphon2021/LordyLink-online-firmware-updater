#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LordyLink.h"
#include <QPushButton>
#include <qlineedit.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QListWidget>
#include <QString>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <String>
#include <QDebug>
#include <QSerialPort.h>
#include <QIODevice.h>
#include <QProgressBar>
#include "Parser.h"
#include "Serial.h"
#include "QUpdateDialog.h"
#include "Downloader.h"
#include "QtSendDialog.h"
#include "UsbThread.h"
#include "QNoHardwareDialog.h"
#include <QStandardItemModel>
#include <QDir>
#include <QModelIndex>




class LordyLink : public QMainWindow
{
    
private:
    Q_OBJECT
    
public:
    LordyLink(QWidget *parent = Q_NULLPTR);
    
    ~LordyLink() {
        QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
        firmware.setNameFilters(QStringList() << "*.*");
        firmware.setFilter(QDir::Files);

        foreach(QString dirFile, firmware.entryList())
            firmware.remove(dirFile);
    }
    
    Ui::LordyLinkClass ui;
    
signals:
    void rightMouseButtonClicked();
    
public slots:
    //MAIN FEATURES
    void OnUpdateButton();
    void OnGetSetButton();
    void OnSendSetButton();
    //Set QTableView
    void addNewSet(QString filename);
    void renameStart(const QModelIndex);
    void renameEnd(QStandardItem*);
    void selectItemToSend(const QModelIndex mindex);
    //GUI Feedback
    void ProgressBar_OnValueChanged(int val) { ui.QInstallProgressBar->setValue(val); }
    void ProgressBar_OnsetMax(int val) { ui.QInstallProgressBar->setMaximum(val); }
    void OnsetLabel(QString message){ ui.QInstallLabel->setText(message); }
    void OnActivateButtons();
    void OnDeactivateButtons();
    void activate_install_button(){ ui.Q_UpdateLordyphonButton->setEnabled(true); }
    //HELPER
    void set_firmware_path_from_dialog(QString path){ firmware_path = path; }
    void OnRemoteMessageBox(QString message);
    void try_download();
    void check_for_lordyphon();
    void hotplugtimer_on();
    void hotplugtimer_off();

private:
    
    SerialHandler* usb_port = nullptr;
    QUpdateDialog* update_dialog = nullptr;
    QNoHardwareDialog* dialog_no_hardware_found = nullptr;
    Downloader* filehandler = nullptr;
    QStandardItemModel* model = nullptr;
    QString home;
    QString oldName;
    QString selected_set;
    QTimer* download_timer = nullptr;
    QTimer* hot_plug_timer = nullptr;
    QString firmware_path;
   

    inline void delay(int millisecondsWait){
        QEventLoop loop;
        QTimer t;
        t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start(millisecondsWait);
        loop.exec();
    }
};

