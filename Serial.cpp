#include "Serial.h"

SerialHandler::SerialHandler(QObject* parent)
	: QObject(parent)
{
	lordyphon_port = new QSerialPort(this);
	lordyphon_port->setReadBufferSize(10);
	QObject::connect(lordyphon_port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
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
		else
			found = false;
	}
	return found;
}

bool SerialHandler::open_lordyphon_port()
{
	lordyphon_port->setPortName(lordyphon_portname);	// open connection with correect port name
	lordyphon_port->open(QIODevice::OpenMode(QIODevice::ReadWrite));
	lordyphon_port->setBaudRate(QSerialPort::Baud9600);
	lordyphon_port->setDataBits(QSerialPort::Data8);
	lordyphon_port->setParity(QSerialPort::NoParity);
	lordyphon_port->setStopBits(QSerialPort::StopBits::OneStop);
	lordyphon_port->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
		
	if (lordyphon_port->isOpen())
		return true;
		
	return false;
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
	else{
		QMessageBox error;
		error.setText(input_buffer);
		error.exec();
	}
	return false;
}


bool SerialHandler::lordyphon_port_is_open()
{
	return lordyphon_port->isOpen();
}
