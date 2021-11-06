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
	Worker(){} // default for get set
	Worker(QString path):selected_set(path){}  //ctor for send set
	Worker(QString path, bool ): selected_firmware(path){}  //ctor for firmware update, bool is a dummy

public slots:

	//MAIN FEATURES OF LORDYLINK
	void update();
	void get_eeprom_content();
	void send_eeprom_content();
	

signals:
	//GUI update
	void ProgressBar_valueChanged(int val);
	void ProgressBar_setMax(int val);
	void setLabel(QString);
	void remoteMessageBox(QString);
	void activateButtons();
	void deactivateButtons();
	// pass to QTableview
	void newItem(QString);
	//thread
	void finished();
	
private:

	QString selected_set; //initialized in ctor
	QString selected_firmware;  //initialized in ctor
	
	//Lordyphon message strings
	struct LordyphonCall {
		QByteArray burn_flash = "w";
		QByteArray say_it_again = "?";
		QByteArray dump_request = "r";
		QByteArray request_checksum = "s";
		QByteArray transfer_request = "R";
		QByteArray begin_tansfer = "//";
		QByteArray burn_eeprom = "ß";
	}call_lordyphon;
          
    
    
	
	
	
	
	

	

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