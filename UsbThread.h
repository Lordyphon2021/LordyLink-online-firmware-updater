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
	void valueChanged(int val);
	void setMax(int val);
	void setLabel(QString);

private:

	void run() override;
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