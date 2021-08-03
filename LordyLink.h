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
#include "Filehandler.h"
#include "Parser.h"
#include "Serial.h"
#include "QUpdateDialog.h"



class LordyLink : public QMainWindow
{
    
private:
    Q_OBJECT
    

public:
    LordyLink(QWidget *parent = Q_NULLPTR);
   
  
     
    Ui::LordyLinkClass ui;
   
   

signals:

   

public slots:
   
 
    void usb_action_wrapper();
    
   
    

private slots:
    

private:
    
   
    
    QNetworkAccessManager* manager;
    SerialHandler* usb;
    
   
    void error_message_box(const char* message);
    std::string path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt";
    HexToSerialParser* parser = nullptr;
    QUpdateDialog update;
   
};

