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
	
	void finished();
	
	//void error(QString); //debugging

private:

	//Hier bin ich mir unsicher, wie man da mit Membervariablen versus lokalen Variablen
	//umgehen soll. Die Worker-Klasse ist eher ein Spezialfall, 
	//der private-Bereich wird von drei Threads benutzt, 
	//das macht die Sache recht unübersichtlich, weil wegen USB immer nur ein Thread aktiv sein darf.
	//Doch alles eher lokal in den Methoden erzeugen? Besser lesbar war's nämlich schon vorher...
	

	
	Parser* hex_parser = nullptr;
	Parser* eeprom_parser = nullptr;
	
	SerialHandler* usb_port_update_thread = nullptr;
	SerialHandler* usb_port_get_thread = nullptr;
	SerialHandler* usb_port_send_thread = nullptr;
	
	QString lordyphon_portname;
	QString rx_status;
	QString Message;
	QString selected_set; //initialized in ctor
	QFile eeprom_content_logfile;
	QString set_dir;
	QString selected_firmware;  //initialized in ctor
	int version = 0;
	QString checksum_status_message;
    QByteArray header = "#";
    QByteArray burn_flash = "w";
    QByteArray tx_data;
    size_t index = 0;
    int bad_checksum_ctr = 0;       
    int rx_error_ctr = 0;
    bool carry_on_flag = true;  
	
	
	QByteArray eeprom;  //data container 

	uint16_t checksum_lordyphon = 0;
	uint16_t eeprom_local_checksum = 0;
	size_t progress_bar_ctr = 0;
	size_t rec_ctr = 0;

	

	//essential for timing
	inline void delay(int millisecondsWait)
	{
		QEventLoop loop;
		QTimer t;
		t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		t.start(millisecondsWait);
		loop.exec();
	}
	
}; 