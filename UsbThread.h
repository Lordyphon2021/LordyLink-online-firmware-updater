#pragma once
#include <QThread>
#include <qmutex.h>
#include <QWaitCondition>
#include "Parser.h"
#include "Serial.h"
#include <QProgressBar>
#include <qfile.h>
#include <QString>
#include <fstream>
#include <vector>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <qstring.h>
#include <QDateTime>

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
	void get_eeprom_content();
	void send_eeprom_content();

signals:
	void ProgressBar_valueChanged(int val);
	void ProgressBar_setMax(int val);
	void setLabel(QString);
	void remoteMessageBox(QString);
	void activateButtons();
	void deactivateButtons();
	
	void finished();
	void error(QString err);
private:
	
	Parser* parser;
	SerialHandler* usb;
	QString lordyphon_portname;
	QString status;
	size_t progress = 0;
	QString Message;
	//std::string path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/";
	//std::string Filename = "sram_content.txt";
	
	QFile sram_content;
	QString set_dir;


	inline void delay(int millisecondsWait)
	{
		QEventLoop loop;
		QTimer t;
		t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		t.start(millisecondsWait);
		loop.exec();
	}
	
};