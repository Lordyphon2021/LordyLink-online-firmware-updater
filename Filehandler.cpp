#include "Filehandler.h"




Filehandler::Filehandler(QListWidget* _line_edit, QObject* parent) : debug_list(_line_edit)
            , QObject(parent)

{
    connect(&manager, &QNetworkAccessManager::finished, this, &Filehandler::finished);
}

void Filehandler::get_FTP_list() {





}

void Filehandler::download(QString location, QString path)
{
   

    file.setFileName(path);
    
    if (!file.open(QIODevice::WriteOnly))
    {
        
        debug_list->addItem ( file.errorString());
        qApp->processEvents();
        return;
    }

    QUrl url(location);
    
    url.setPort(21);
    url.setUserName("stefandeisenberger86881");
    url.setPassword("3333Sync!!!");
   
    

    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply* reply = manager.get(request);
    
    
    
    wire(reply);

    
    debug_list->addItem("connecting...");
    qApp->processEvents();

}

void Filehandler::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply)
    {
        QByteArray data = reply->readAll();
        file.write(data);
        
        
       
        debug_list->addItem(data);
        qApp->processEvents();
    }
    else {
        
        
        debug_list->addItem("readyread error");
        qApp->processEvents();
    }
}

void Filehandler::finished(QNetworkReply* reply)
{
    
    debug_list->addItem ("done");
    qApp->processEvents();
    file.close();
    reply->close();
}


void Filehandler::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal <= 0) {
        
        
        debug_list->addItem("no data received... ");
        qApp->processEvents();
        return;
    }
   
   
}

void Filehandler::error(QNetworkReply::NetworkError code)
{
   
    
    debug_list->addItem(code + "                      ");
    qApp->processEvents();

    
    
   
    
    
   
}

void Filehandler::wire(QNetworkReply* reply)
{
   
    connect(reply, &QNetworkReply::readyRead, this, &Filehandler::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &Filehandler::downloadProgress);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &Filehandler::error);
    
    debug_list->addItem("wiring...");
    qApp->processEvents();


}

