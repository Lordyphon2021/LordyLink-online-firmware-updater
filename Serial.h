#pragma once
#include <QApplication.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include<QIODevice.h>
#include<QListWidget>
#include <QMessageBox>
#include <QBuffer>


class SerialHandler : public QObject
{
	Q_OBJECT
		

public:


	
	SerialHandler( QObject* parent = nullptr );
	bool find_lordyphon_port();
	void dump_baud_rate();
	
	bool open_lordyphon_port();
	bool lordyphon_handshake();
	bool write_serial_data(const QByteArray& tx_data);
	bool port_at_end();
	void wait_for_ready_read(int timeout);
	
	bool lordyphon_port_is_open();
	void close_usb_port();
	void wire();
	void set_buffer_size(qint64 size);
	
	QByteArray& getInputBuffer()
	{
		
		return input_buffer;
	}
	
signals:

	

public slots:

	void onReadyRead();
	

private:
	bool message_received = false;
	QSerialPort* lordyphon_port;
	
	QString lordyphon_portname;
	QByteArray input_buffer = 0;
	const QByteArray hand_shake_tx_phrase = "!c++ is great!   ";
	const QByteArray hand_shake_rx_phrase = "YES!";
	
	

};
