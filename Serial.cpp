#include "Serial.h"



//WRAPPER CLASS FOR QSERIALPORT


SerialHandler::SerialHandler(QObject* parent): QObject(parent){
	lordyphon_port = new QSerialPort(this);
	lordyphon_port->setReadBufferSize(10);
	QObject::connect(lordyphon_port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

bool SerialHandler::find_lordyphon_port(){
	
	bool found = false;
	
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
	{
		if (info.manufacturer() == "FTDI") {  //this in't enough for identification, I have no vendor ID
			lordyphon_portname = info.portName();  //handshake will confirm lordyphon ID
			found = true;
			break;
		}
		else {
			found = false;
		}
	}
	return found;
}

bool SerialHandler::open_lordyphon_port()// open connection with correct port name
{
	lordyphon_port->setPortName(lordyphon_portname);	
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
	
bool SerialHandler::clear_buffer() { return lordyphon_port->clear(); } 

//CLOSE PORT AFTER ACTION, THERE CAN ONLY BE ONE LORDYPHON PORT
void SerialHandler::close_usb_port(){ lordyphon_port->close(); }

//ADJUST BUFFERSIZE TO MATCH EXPECTED MESSAGE SIZE
void SerialHandler::set_buffer_size(qint64 size)const{
	lordyphon_port->setReadBufferSize(size);
}

bool SerialHandler::write_serial_data(const QByteArray& tx_data) {
	if (lordyphon_port->isOpen()) {
		lordyphon_port->write(tx_data);
		
		if(lordyphon_port->waitForBytesWritten())
			return true;
	}
	return false;
}

//BLOCKING FUNCTION, WAITS UNTIL BUFFER IS FULL OR UNTIL TIMEOUT (MS)
//TIMEOUT -1 => WAITS FOREVER
//EMITS SIGNAL WHEN IT'S DONE

void SerialHandler::wait_for_ready_read(int timeout)const {
	lordyphon_port->waitForReadyRead(timeout);
}

//CATCHES SIGNAL OF wait_for_ready_read, READS BUFFER SIZE
void SerialHandler::onReadyRead(){
	input_buffer = lordyphon_port->read(lordyphon_port->readBufferSize());
}

//FINAL IDENTIFICATION INSTEAD OF CHECKING VENDOR ID		
bool SerialHandler::lordyphon_handshake(){		
	
	while(!lordyphon_port->isOpen())
		;  //wait till port is open (might take a couple of ms)
		
	write_serial_data(lordyphon_call.hand_shake_tx_phrase); //call
	wait_for_ready_read(1000);
		
	if (input_buffer == lordyphon_response.hand_shake_rx_phrase) //check response
		return true;

	return false;
}

//CHECK WHETHER LORDYPHON IS IN UPDATE MODE
bool SerialHandler::lordyphon_update_call(){
	
	while (!lordyphon_port->isOpen())
		;

	write_serial_data(lordyphon_call.update_tx_phrase);
	wait_for_ready_read(1000);

	if (input_buffer == lordyphon_response.update_rx_phrase_y)
		return true;
	
	else if (input_buffer == lordyphon_response.update_rx_phrase_n)
		return false;
	
	else {
		QMessageBox error;
		error.setText("rx error, please try again");
		error.exec();
		
		return false;
	}
}

//CHECK IF OPEN
bool SerialHandler::lordyphon_port_is_open(){
	
	if(lordyphon_port->isOpen())
		return true;
	
	return false;
}

