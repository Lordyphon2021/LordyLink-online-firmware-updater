#pragma once
#include <QThread>
#include <qmutex.h>
#include <QWaitCondition>
#include "Parser.h"
#include "Serial.h"
#include "Filehandler.h"
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
#include <filesystem>

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
	Worker(){} // default
	
	Worker(QString path):selected_set(path){}  //ctor for send set
	
	Worker(QString path, bool status): selected_firmware(path){}    //ctor for firmware update

	
	

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
	void newItem(QString);
	void finished();
	void error(QString);

private:
	
	Parser* parser = nullptr;
	Downloader* filehandler = nullptr;
	SerialHandler* usb = nullptr;
	QString lordyphon_portname;
	QString status;
	size_t progress = 0;
	QString Message;
	QString selected_set;
	
	QFile eeprom_content;
	QString set_dir;
	QString selected_firmware;
	
	int version = 0;


	inline void delay(int millisecondsWait)
	{
		QEventLoop loop;
		QTimer t;
		t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		t.start(millisecondsWait);
		loop.exec();
	}
	
}; 