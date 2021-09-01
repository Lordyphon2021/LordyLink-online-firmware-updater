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
    Filehandler(QObject* parent = nullptr);
    
    
    void download(QString location, QString path);
    
    bool download_finished()
    {

        return status;

    }




   

signals:
    void no_data();


private slots:
    void readyRead();
    void finished(QNetworkReply* reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
   

private:
    QNetworkAccessManager manager;
    QFile file;
    QListWidget* debug_list = nullptr;
    QMessageBox error_message;
    void wire(QNetworkReply* reply);
    bool status = false;
};

