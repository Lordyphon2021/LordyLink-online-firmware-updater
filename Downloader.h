#pragma once


#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QListWidget>
#include <QApplication.h>
#include <QMessagebox>

#include "Parser.h"

class Downloader : public QObject
{
    Q_OBJECT
public:
    Downloader(QObject* parent = nullptr);
    
void download(QString location, QString path); //main method
    
signals:
    void no_data();
    void download_finished();


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
    
};

