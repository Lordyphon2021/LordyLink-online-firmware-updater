#pragma once


#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <qlineedit.h>
#include <QApplication.h>

class Filehandler : public QObject
{
    Q_OBJECT
public:
    Filehandler(QLineEdit* _line_edit, QObject* parent = nullptr);
    
    
    void download(QString location, QString path);

signals:



private slots:
    void readyRead();
    void finished(QNetworkReply* reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    
private:
    QNetworkAccessManager manager;
    QFile file;
    QLineEdit* line_edit;

    void wire(QNetworkReply* reply);
};

