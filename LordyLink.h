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
#include<QIODevice.h>
#include <QProgressBar>
#include "Filehandler.h"
#include "Parser.h"
#include "Serial.h"
#include "QUpdateDialog.h"
#include "UsbThread.h"
#include <qscopedpointer.h>



class LordyLink : public QMainWindow
{
    
private:
    Q_OBJECT
    

public:
    LordyLink(QWidget *parent = Q_NULLPTR);
   
  
     
    Ui::LordyLinkClass ui;
   
   

signals:

   

public slots:
   
    void OnValueChanged(int val) {

        ui.QInstallProgressBar->setValue(val);
        
    }
    void OnsetMax(int val)
    {

        ui.QInstallProgressBar->setMaximum(val);

    }
    void OnsetLabel(QString message)
    {

        ui.QInstallLabel->setText(message);

    }
    void usb_action_wrapper();
   
   
    

private slots:
    

private:
    
   
    
    
    SerialHandler* usb_port;
    USBThread* thread;
   
    void error_message_box(const char* message);
    std::string path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt";
    
    QUpdateDialog update;
   
};

