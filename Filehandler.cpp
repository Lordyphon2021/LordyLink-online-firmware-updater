#include "Filehandler.h"




Filehandler::Filehandler(QLineEdit* _line_edit, QObject* parent) : line_edit(_line_edit)
            , QObject(parent)

{
    
    
    connect(&manager, &QNetworkAccessManager::finished, this, &Filehandler::finished);
}



void Filehandler::download(QString location, QString path)
{
   

    file.setFileName(path);
    
    if (!file.open(QIODevice::WriteOnly))
    {
        line_edit->clear();
        line_edit->setText ( file.errorString());
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

    line_edit->clear();
    line_edit->setText("connecting...");
    qApp->processEvents();

}

void Filehandler::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply)
    {
        QByteArray data = reply->readAll();
        file.write(data);
        
        qApp->processEvents();
        line_edit->clear();
        line_edit->setText(data);
        qApp->processEvents();
    }
    else {

        line_edit->clear();
        line_edit->setText("readyread error");
        qApp->processEvents();
    }
}

void Filehandler::finished(QNetworkReply* reply)
{
    line_edit->clear();
    line_edit->setText ("done");
    qApp->processEvents();
    file.close();
    reply->close();
}


void Filehandler::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal <= 0) {
        
        line_edit->clear();
        line_edit->setText("no data " + bytesReceived);
        qApp->processEvents();
        return;
    }
   
   
}

void Filehandler::error(QNetworkReply::NetworkError code)
{
    line_edit->clear();
    line_edit->setText( "Error: " + code );
    qApp->processEvents();
}

void Filehandler::wire(QNetworkReply* reply)
{
   
    connect(reply, &QNetworkReply::readyRead, this, &Filehandler::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &Filehandler::downloadProgress);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &Filehandler::error);
    line_edit->clear();
    line_edit->setText("wiring...");
    qApp->processEvents();


}

