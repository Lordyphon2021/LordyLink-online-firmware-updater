#pragma once
#include <QThread>
#include <qmutex.h>
#include <QWaitCondition>
#include "Parser.h"
#include "Serial.h"
#include "Downloader.h"
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
#include "LordyphonMessageStrings.h"


class USBThread : public QThread
{
	Q_OBJECT
	
	void run() override;

};

class Worker : public QObject 
{
	Q_OBJECT

public:
	Worker(QString com_port) : usb_com_port(com_port) {} // default for get set 
	Worker(QString com_port, QString path) :usb_com_port(com_port), selected_set(path) {}  //ctor for send set
	
	Worker(uint8_t thread_no, const QSerialPortInfo& info)
		 :thread_number(thread_no), port_info(info)
	{} 
	
	//ctor for firmware update, bool is a dummy
	Worker(QString com_port, QString path, bool) : usb_com_port(com_port), selected_firmware(path) {}
	
	

public slots:

	//MAIN FEATURES OF LORDYLINK
	void update();
	void get_eeprom_content();
	void send_eeprom_content();
	void get_usb_port();

	
signals:
	//GUI update
	void ProgressBar_valueChanged(int val);
	void ProgressBar_setMax(int val);
	void setLabel(QString);
	void remoteMessageBox(QString);
	void activateButtons();
	void deactivateButtons();
	void deactivateAbortButton();
	void LordyphonConnected(bool);
	void SetPortname(QString);
	
	
	// pass to QTableview
	void newItem(QString);
	//thread
	void finished();
	
private:

	QString selected_set; //initialized in ctor
	QString selected_firmware;  //initialized in ctor
	uint8_t ready_read_timeout_ctr = 0;
	
	//Lordyphon message strings
	LordyphonCall call_lordyphon;
	LordyphonResponse lordyphon_response;
	uint8_t thread_number = 0;
	QSerialPortInfo port_info;
	QString usb_com_port;
          
    //timing
	inline void delay(int millisecondsWait)
	{
		QEventLoop loop;
		QTimer t;
		t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		t.start(millisecondsWait);
		loop.exec();
	}
}; 