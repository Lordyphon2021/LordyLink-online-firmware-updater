#include "Downloader.h"




Downloader::Downloader(QObject* parent):QObject(parent){
    
    connect(&manager, &QNetworkAccessManager::finished, this, &Downloader::finished);
}


void Downloader::download(QString location, QString path){ //location and path via CTOR

    file.setFileName(path); //download destination
    
    if (!file.open(QIODevice::WriteOnly)){
       qDebug() << file.errorString();
       return;
    }
    
    QUrl url(location);
    url.setPort(21);
    url.setUserName("**************");
    url.setPassword("**************");
   
    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply* reply = manager.get(request);
    wire(reply);
}

void Downloader::readyRead(){
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
   
    if (reply){
        QByteArray data = reply->readAll();
        file.write(data); // download data to file
    }
    else {
        qDebug() << "readyread error";
    }
}

void Downloader::finished(QNetworkReply* reply){
    
    file.close();
    reply->close();
    emit download_finished();
    qDebug() << "download finished"; 
}


void Downloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    
    if (bytesTotal <= 0) {
        
        qDebug() << "no data received... ";
        emit no_data();
        file.close();
        file.remove(); //if file empty, delete right away
        return;
    }
}

void Downloader::error(QNetworkReply::NetworkError code){
   
    qDebug() << "error" << code;
}

void Downloader::wire(QNetworkReply* reply){
    
    connect(reply, &QNetworkReply::readyRead, this, &Downloader::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::downloadProgress);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &Downloader::error);
}

