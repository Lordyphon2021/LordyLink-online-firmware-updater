
#include "Serial.h"





bool SerialHandler::find_lordyphon_port()
{
	bool found_flag = false;
	

	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
	{
		if (info.manufacturer() == "FTDI") {
			found_flag = true;
			lordyphon_portname = info.portName();
			break;
		}
		else {
			found_flag = false;
		}
	}
	if (found_flag == false) {
		QMessageBox no_hardware;
		no_hardware.setText("hardware disconnected!");
		no_hardware.exec();
		return false;
	}

	lordyphon.setPortName(lordyphon_portname);
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

QByteArray SerialHandler::read_serial_data()
{
	QByteArray input_buffer;
	
	if (lordyphon.isOpen()) {
		input_buffer = lordyphon.readAll();

		if (input_buffer.isEmpty()) {

			return "error: read error";
		}
		return input_buffer;
	}

	return "error: connection closed";
	

}
		
bool SerialHandler::identify_lordyphon()
{
		
		write_serial_data(hand_shake_tx_phrase);
	
		if(read_serial_data()== hand_shake_rx_phrase) {
			
			return true;
		}
		
		return false;

		

}