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
	bool lordyphon_update_call();
	bool lordyphon_port_is_open();
	void close_usb_port();
	void wire();
	void set_buffer_size(qint64 size);
	bool clear_buffer();
	
		QByteArray& getInputBuffer()
	{
		
		return input_buffer;
	}
	
signals:
	
	void device_not_found();
	

public slots:

	void onReadyRead();
	void onUsbError();
	

private:
	bool message_received = false;
	QSerialPort* lordyphon_port;
	
	QString lordyphon_portname;
	QByteArray input_buffer = 0;
	const QByteArray hand_shake_tx_phrase = "!c++ is great!   ";
	const QByteArray hand_shake_rx_phrase = "YES!";
	const QByteArray update_tx_phrase = "update";
	const QByteArray update_rx_phrase_y = "sure";
	const QByteArray update_rx_phrase_n = "nope";

};
