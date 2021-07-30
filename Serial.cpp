
#include "Serial.h"





bool SerialHandler::find_lordyphon_port()
{
	bool found_flag = false;
	
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
	{
		if (info.manufacturer() == "FTDI") {
			
			lordyphon_portname = info.portName();
			found_flag = true;
			break;				// exit foreach with correct portname
		}
		else {
			found_flag = false;
		}
	}
	if (found_flag == false) {
	
		return false;
	}

	lordyphon.setPortName(lordyphon_portname);	// open connection with correect port name
	lordyphon.open(QIODevice::OpenMode(QIODevice::ReadWrite));
	lordyphon.setBaudRate(QSerialPort::Baud9600);
	lordyphon.setDataBits(QSerialPort::Data8);
	lordyphon.setParity(QSerialPort::NoParity);
	lordyphon.setStopBits(QSerialPort::OneStop);
	lordyphon.setFlowControl(QSerialPort::NoFlowControl);

	return true;
}
	
	
bool SerialHandler::write_serial_data(const QByteArray& tx_data) 
{

	if (lordyphon.isOpen()) {
		lordyphon.write(tx_data);
		return true;
	}

	return false;

}

QByteArray& SerialHandler::read_serial_data()
{
	input_buffer.clear();
	
	if (lordyphon.isOpen()) {
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
	
	if(read_serial_data()== hand_shake_rx_phrase) {
		return true;
	}

	
		
	return false;

}