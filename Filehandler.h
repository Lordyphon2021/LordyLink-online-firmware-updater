#pragma once


#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QListWidget>
#include <QApplication.h>
#include <QMessagebox>

#include "Parser.h"

class Filehandler : public QObject
{
    Q_OBJECT
public:
    Filehandler(QListWidget* debug_list, QObject* parent = nullptr);
    
    
    void download(QString location, QString path);
    void get_FTP_list();





   

signals:



private slots:
    void readyRead();
    void finished(QNetworkReply* reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
   

private:
    QNetworkAccessManager manager;
    QFile file;
    QListWidget* debug_list;
    QMessageBox error_message;
    void wire(QNetworkReply* reply);
};

