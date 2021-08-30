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
#include <QStandardItemModel>
#include <QDir>



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
    
    
    void on_update_button();
   
    void OnGetSetButton();
    
    void OnSendSetButton();
    
    void OnRemoteMessageBox(QString message);
    
    void OnActivateButtons();
    
    void OnDeactivateButtons();
    
    void refresh(QString filename)
    {
        
        
      QStandardItem* itemname = new QStandardItem(filename);
      itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
      
      
      
      model->appendRow(QList<QStandardItem*>() << itemname);
      model->setHeaderData(0, Qt::Horizontal, QObject::tr("saved sets: "));
      
      for (int col = 0; col < model->rowCount(); col++)
      {
          ui.dirView->setColumnWidth(col, 300);
      }
   
    }
   
    //void renameStart(const QModelIndex);
    //void renameEnd(QStandardItem*);

    

private:
    
   
    
    
    SerialHandler* usb_port;
    
   
    //void error_message_box(const char* message);
    std::string path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt";
    
    QUpdateDialog update;

    QStandardItemModel* model;
    QString home;
    QString oldName;
   
};

