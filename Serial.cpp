#include "Serial.h"

SerialHandler::SerialHandler(QObject* parent)
	: QObject(parent)
{
	lordyphon_port = new QSerialPort(this);
	lordyphon_port->setReadBufferSize(10);
	QObject::connect(lordyphon_port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	QObject::connect(lordyphon_port, SIGNAL(QSerialPort::errorOccurred()), this, SLOT(QSerialPort::SerialPortError error()const));
	QObject::connect(lordyphon_port, SIGNAL(device_not_found()), this, SLOT(onUsbError()));
}


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
		else {
			found = false;
			emit device_not_found();
		}
	}
	return found;
}

bool SerialHandler::open_lordyphon_port()
{
	lordyphon_port->setPortName(lordyphon_portname);	// open connection with correect port name
	lordyphon_port->open(QIODevice::OpenMode(QIODevice::ReadWrite));
	lordyphon_port->setBaudRate(QSerialPort::Baud57600);
	lordyphon_port->setDataBits(QSerialPort::Data8);
	lordyphon_port->setParity(QSerialPort::NoParity);
	lordyphon_port->setStopBits(QSerialPort::StopBits::OneStop);
	lordyphon_port->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
	
	if (lordyphon_port->isOpen())
		return true;
	
	return false;
	
}
	
void SerialHandler::dump_baud_rate()
{
	if(lordyphon_port->isOpen())
		lordyphon_port->setBaudRate(QSerialPort::Baud9600);

}
bool SerialHandler::clear_buffer()
{

	return lordyphon_port->clear();


}



void SerialHandler::close_usb_port()
{
	lordyphon_port->close();
}

void SerialHandler::onReadyRead()
{
	input_buffer = lordyphon_port->read(lordyphon_port->readBufferSize());
}

void SerialHandler::set_buffer_size(qint64 size)
{
	lordyphon_port->setReadBufferSize(size);
}

bool SerialHandler::write_serial_data(const QByteArray& tx_data) 
{

	if (lordyphon_port->isOpen()) {
		
		lordyphon_port->write(tx_data);
		
		if(lordyphon_port->waitForBytesWritten())
			return true;
	}
	//emit device_not_found();
	return false;

}

void SerialHandler::wait_for_ready_read(int timeout)
{
	lordyphon_port->waitForReadyRead(timeout);
}


		
bool SerialHandler::lordyphon_handshake()
{		
	
	while(!lordyphon_port->isOpen())
		;
		
	write_serial_data(hand_shake_tx_phrase);
	wait_for_ready_read(1000);
		
		
	if (input_buffer == hand_shake_rx_phrase) 
		return true;
	else
		
	
	return false;
}
bool SerialHandler::lordyphon_update_call()
{

	while (!lordyphon_port->isOpen())
		;

	write_serial_data(update_tx_phrase);
	wait_for_ready_read(1000);


	if (input_buffer == update_rx_phrase_y)
		return true;
	
	else if (input_buffer == update_rx_phrase_n)
		return false;
	
	else {
		emit device_not_found();
		QMessageBox error;
		error.setText("update response not valid");
		error.exec();
		
		return false;

	}
}


bool SerialHandler::lordyphon_port_is_open()
{
	if(lordyphon_port->isOpen())
		return true;
	
	else {
		emit device_not_found();
		return false;
	}
}

void SerialHandler::onUsbError()
{

	QMessageBox error;
	error.setText("lordyphon disconnected");
	error.exec();


}
