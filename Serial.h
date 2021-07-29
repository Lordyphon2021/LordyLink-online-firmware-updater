#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>
#include<QIODevice.h>
#include<QListWidget>
#include <QMessageBox>


class SerialHandler
{

public:
	
	
	bool find_lordyphon_port();
	bool identify_lordyphon();
	bool write_serial_data(const QByteArray& tx_data);
	QByteArray read_serial_data();

private:
	
	QSerialPort lordyphon;
	QString lordyphon_portname;
	QByteArray serial_input_buffer;
	const QByteArray hand_shake_tx_phrase = "all cops are bastards!";
	const QByteArray hand_shake_rx_phrase = "ACAB!";


};
