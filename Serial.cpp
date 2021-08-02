
#include "Serial.h"

SerialHandler::SerialHandler(QObject* parent)
	: QObject(parent)
{}


bool SerialHandler::find_lordyphon_port()
{
	
	bool found = false;
	
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
	{
		if (info.manufacturer() == "FTDI") {
			lordyphon_portname = info.portName();
			found = true;
			break;

		}
		else
			found = false;
	}
	return found;
		
}


bool SerialHandler::open_lordyphon_port()
{
	
	
		lordyphon.setPortName(lordyphon_portname);	// open connection with correect port name
		lordyphon.open(QIODevice::OpenMode(QIODevice::ReadWrite));
		lordyphon.setBaudRate(QSerialPort::Baud9600);
		lordyphon.setDataBits(QSerialPort::Data8);
		lordyphon.setParity(QSerialPort::NoParity);
		lordyphon.setStopBits(QSerialPort::OneStop);
		lordyphon.setFlowControl(QSerialPort::NoFlowControl);
		
		if (lordyphon.isOpen())
			return true;
		
	
		return false;
}
	







bool SerialHandler::write_serial_data(const QByteArray& tx_data) 
{

	if (lordyphon.isOpen()) {
		
		lordyphon.write(tx_data);
		//lordyphon.waitForBytesWritten();
		return true;
	}

	return false;

}

QByteArray& SerialHandler::read_serial_data()
{
	input_buffer.clear();
	
	if (lordyphon.isOpen()) {
		
		while (lordyphon.waitForReadyRead(1000))  //timeout 1000ms
			;  //do nothing
		
		input_buffer = lordyphon.readAll();
		
		

			if (input_buffer.isEmpty()) {

				input_buffer = "error: buffer empty";

				return input_buffer;

			}

			return input_buffer;
		
	}

	input_buffer =  "error: connection closed";
	return input_buffer;
	

}
		
bool SerialHandler::lordyphon_handshake()
{		
	write_serial_data(hand_shake_tx_phrase);
	
	QString response = read_serial_data();
	
	if (response == hand_shake_rx_phrase) {
		
		
		return true;
	}
	else {
		QMessageBox error;
		error.setText(response);
		error.exec();

	}
	return false;

}