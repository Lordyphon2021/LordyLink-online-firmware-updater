#pragma once
#include <QApplication.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include<QIODevice.h>
#include<QListWidget>
#include <QMessageBox>
#include <QBuffer>
#include "LordyphonMessageStrings.h"
#include <exception>

//WRAPPER FOR QSERIALPORT

class SerialHandler : public QObject
{
	Q_OBJECT
		
public:
	
	SerialHandler( QObject* parent = nullptr );
	bool find_lordyphon_port();
	bool open_lordyphon_port();
	bool lordyphon_handshake();
	bool write_serial_data(const QByteArray& tx_data);
	bool wait_for_ready_read(int timeout)const;
	bool lordyphon_update_call();
	bool lordyphon_port_is_open();
	void close_usb_port();
	void set_buffer_size(qint64 size)const;
	bool clear_buffer();
	bool check_with_manufacturer_ID();
	QByteArray& getInputBuffer(){ return input_buffer; }
	
signals:
	
public slots:

	void onReadyRead();
	
	

private:
	
	QSerialPort* lordyphon_port;
	QString lordyphon_portname;
	QByteArray input_buffer = 0;
	QSerialPortInfo save_info;
	size_t port_index = 0;
	LordyphonCall lordyphon_call;
	LordyphonResponse lordyphon_response;

};
