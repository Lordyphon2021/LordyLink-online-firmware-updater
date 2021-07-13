#pragma once

#include <QObject>
#include < QtNetwork/QNetworkAccessManager >
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <QDebug>


class Downloader : public QObject
{
	Q_OBJECT

public:
	explicit Downloader(QObject* parent = nullptr);
	void download(); 

signals:


public slots:



private:
	





};