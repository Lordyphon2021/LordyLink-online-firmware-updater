#pragma once
#include <QThread>
#include <qmutex.h>
#include <QWaitCondition>
#include "Parser.h"
#include "Serial.h"
#include <QProgressBar>
#include <qfile.h>
#include <QString>


class USBThread : public QThread
{
	Q_OBJECT

public:
	

signals:
	

private:

	void run() override;
	
	
	
	
};

class Worker : public QObject {
	Q_OBJECT

public:
	
	

public slots:
	void update();
	void get_sram_content();


signals:
	void ProgressBar_valueChanged(int val);
	void ProgressBar_setMax(int val);
	void setLabel(QString);
	void get_sram();
	
	
	
	void finished();
	void error(QString err);
private:
	
	HexToSerialParser* parser;
	SerialHandler* usb;
	QString lordyphon_portname;
	QString status;
	size_t progress = 0;
	QString Message;
	QString path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/";
	QString Filename = "sram_content.txt";

	QFile sram_content;
};