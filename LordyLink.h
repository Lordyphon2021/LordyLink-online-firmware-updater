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
#include <QDebug>
#include <QSerialPort.h>
#include<QIODevice.h>
#include "Filehandler.h"
#include "Parser.h"
#include "Serial.h"


class LordyLink : public QMainWindow
{
    
private:
    Q_OBJECT
    

public:
    LordyLink(QWidget *parent = Q_NULLPTR);
   
  
     
    Ui::LordyLinkClass ui;
   
   

signals:

public slots:
   
    void download_wrapper();
    void usb_action_wrapper();
   
    

private slots:
    

private:
    
   
    Filehandler* filehandler = nullptr;
    QNetworkAccessManager* manager;
    SerialHandler usb;
    void download(Filehandler* filehandler);
    void error_message_box(const char* message);
    
    
   
};

