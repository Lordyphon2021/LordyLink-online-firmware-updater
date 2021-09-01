#include "Filehandler.h"




Filehandler::Filehandler(QObject* parent):QObject(parent)

{
    connect(&manager, &QNetworkAccessManager::finished, this, &Filehandler::finished);
   
}



void Filehandler::download(QString location, QString path)
{
   
    
    
    file.setFileName(path);
    
    if (!file.open(QIODevice::WriteOnly))
    {
        
        qDebug() << file.errorString();
       
        return;
    }
    
    
    QUrl url(location);
    
    url.setPort(21);
    url.setUserName("stefandeisenberger86881");
    url.setPassword("3333Sync!!!");
   
    

    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply* reply = manager.get(request);
    
   
    wire(reply);

    
   
    

}

void Filehandler::readyRead()
{
    
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply)
    {
        QByteArray data = reply->readAll();
        file.write(data);
        
       
        
    }
    else {
        
        
        qDebug() << "readyread error";
        
    }
}

void Filehandler::finished(QNetworkReply* reply)
{
    
  
    
    file.close();
    reply->close();
    emit download_finished();
    qDebug() << "done"; 
    
}


void Filehandler::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal <= 0) {
        
        
        qDebug() << "no data received... ";
        emit no_data();
        file.close();
        file.remove();
        return;
    }
   
   
}

void Filehandler::error(QNetworkReply::NetworkError code)
{
   
    
    qDebug() << "error" << code;
    

    
    
   
    
    
   
}

void Filehandler::wire(QNetworkReply* reply)
{
   
    connect(reply, &QNetworkReply::readyRead, this, &Filehandler::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &Filehandler::downloadProgress);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &Filehandler::error);
    
   
   


}

