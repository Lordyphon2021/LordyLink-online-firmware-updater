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
#include "filehandler.h"
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
   
  
     
    Ui::LordyLinkClass ui;
    
   

signals:
    
   

public slots:
   
    void ProgressBar_OnValueChanged(int val) {

        ui.QInstallProgressBar->setValue(val);
        
    }
    void ProgressBar_OnsetMax(int val)
    {

        ui.QInstallProgressBar->setMaximum(val);

    }
    void OnsetLabel(QString message)
    {

        ui.QInstallLabel->setText(message);

    }
    
    
    void OnUpdateButton();
    void OnGetSetButton();
    void OnSendSetButton();
    void OnRemoteMessageBox(QString message);
    void OnActivateButtons();
    void OnDeactivateButtons();
    void addNewSet(QString filename);
    void renameStart(const QModelIndex);
    void renameEnd(QStandardItem*);
    void selectItemToSend(const QModelIndex mindex);
    void get_path(QString path)
    {

        firmware_path = path;
    }
    

private:
    
   
    
    
    SerialHandler* usb_port;
    
   
    
   
    
    QUpdateDialog* update_dialog;
    QNoHardwareDialog* dialog_no_hardware_found = nullptr;
    Filehandler* filehandler = nullptr;
    QStandardItemModel* model = nullptr;
    QString home;
    QString oldName;
    QString selected_set;
    QString firmware_path;
   
};

