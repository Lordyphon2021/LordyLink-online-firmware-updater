#include "Serial.h"
#include <qdebug.h>
#include <qexception.h>




//WRAPPER CLASS FOR QSERIALPORT
//Controller has no vendor ID to be identified with.
//Identification process works like this:
//SerialHandler goes through all USB ports and checks for manufacturer ID "FTDI"
//if present, controller is identified with call/response phrases.
//hotplug detection remembers last known port of the controller and only scans for the manufacturer ID
//as this will not interfere with controller performance.
//TODO: handle more than one controllers in a system with unique addresses

//Note: Asyncronous readAll() method is too slow for this task. 
//I chose only to read known message sizes ( setReadBufferSize(expected size) ) for reliable results


SerialHandler::SerialHandler(QObject* parent): QObject(parent)
{
	lordyphon_port = new QSerialPort(this);
	lordyphon_port->setReadBufferSize(10);
	QObject::connect(lordyphon_port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

//FINAL IDENTIFICATION INSTEAD OF CHECKING VENDOR ID		
bool SerialHandler::lordyphon_handshake() 
{
	bool retVal = false;
	try
	{
		if (lordyphon_port_is_open())
		{
			write_serial_data(lordyphon_call.hand_shake_tx_phrase); //call
			wait_for_ready_read(2000);
			
			if (input_buffer == lordyphon_response.hand_shake_rx_phrase) //check response
			{
				retVal = true;
			}
		}
	}
	catch (QException& e)
	{
		qDebug() << e.what();
	}

	return retVal;
}

void SerialHandler::quit_message() 
{
	try
	{
		if (lordyphon_port_is_open())
		{
			write_serial_data(lordyphon_call.lordylink_quit); //call
			lordyphon_port->close();
		}
	}
	catch (QException& e)
	{
		qDebug() << e.what();
	}
}

//CHECK WHETHER LORDYPHON IS IN UPDATE MODE
bool SerialHandler::lordyphon_update_call() 
{
	bool retVal = false;
	
	try
	{
		if (lordyphon_port_is_open())
		{
			write_serial_data(lordyphon_call.update_tx_phrase);
			wait_for_ready_read(1000);
			lordyphon_port->close();
		}
	}
	catch (QException& e)
	{
		qDebug() << e.what();
	}

	if (input_buffer == lordyphon_response.update_rx_phrase_y)
	{
		input_buffer = "";
		retVal = true;
	}
	else if (input_buffer == lordyphon_response.update_rx_phrase_n)
	{
		retVal = false;
	}
	else 
	{
		QMessageBox error;
		error.setText("Activate USB-Mode on Lordyphon (press global + looper button)");
		error.exec();

		retVal = false;
	}

	return retVal;
}

bool SerialHandler::find_lordyphon_port()
{
	bool retVal = false;
	
	try 
	{
		port_index = 0;
		//go through all available USB ports...
		foreach(const QSerialPortInfo& info, QSerialPortInfo::availablePorts())
		{
			//this in't enough for identification, I have no vendor ID yet, any FTDI based devices will be found here...
			if (info.manufacturer() == "FTDI") {		
				//handshake will confirm lordyphon ID
				lordyphon_portname = info.portName(); 
					//try handshakes on each FTDI port to identify lordyphon
					open_lordyphon_port();
					lordyphon_port->write("!");
					QString test = lordyphon_port->readAll();
					lordyphon_port->close();			
					
					retVal = true; 
					break;
			}
			++port_index;  // lordyphon usb port number stored here...
		}
	}
	catch (QException& e) 
	{
		qDebug() << e.what();
	}

	return retVal;
}

bool SerialHandler::check_with_manufacturer_ID() 
{
	size_t check_index = 0;
	bool retVal = false;
	
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
	{
		//check if lordyphon is still on the same usb port
		if (info.manufacturer() == "FTDI" && port_index == check_index) 
		{ 
			retVal = true;
			break;
		}
		++check_index;
	}

	return retVal;
}

bool SerialHandler::open_lordyphon_port()// open connection with correct port name
{
	bool retVal = false;

	try
	{
		//set parameters
		lordyphon_port->setPortName(lordyphon_portname);
		lordyphon_port->open(QIODevice::OpenMode(QIODevice::ReadWrite));
		lordyphon_port->setBaudRate(QSerialPort::Baud115200);
		lordyphon_port->setDataBits(QSerialPort::Data8);
		lordyphon_port->setParity(QSerialPort::NoParity);
		lordyphon_port->setStopBits(QSerialPort::StopBits::OneStop);
		lordyphon_port->setFlowControl(QSerialPort::FlowControl::NoFlowControl);

		while (!lordyphon_port->isOpen())
		{ 
			; 
		}

		retVal = true;
	}
	catch (QException& e)
	{
		qDebug() << e.what();
	}

	return retVal;
}
	
bool SerialHandler::clear_buffer() 
{ 
	return lordyphon_port->clear(); 
} 

//CLOSE PORT AFTER ACTION
void SerialHandler::close_usb_port()
{ 
	lordyphon_port->close(); 
}

//ADJUST BUFFERSIZE TO MATCH EXPECTED MESSAGE SIZE
void SerialHandler::set_buffer_size(qint64 size)const
{
	lordyphon_port->setReadBufferSize(size);
}

bool SerialHandler::write_serial_data(const QByteArray& tx_data) 
{
	bool retVal = false;
	
	try
	{
		if (lordyphon_port->isOpen())
		{
			lordyphon_port->write(tx_data);

			if(lordyphon_port->waitForBytesWritten())
			{
				retVal = true;
			}
		}
	}
	catch (QException& e)
	{
		qDebug() << e.what();
	}
	
	return retVal;
}

//BLOCKING FUNCTION, WAITS UNTIL BUFFER IS FULL OR UNTIL TIMEOUT (MS)
//TIMEOUT -1 => WAITS FOREVER
//EMITS SIGNAL TO onReadyRead() WHEN IT'S DONE

bool SerialHandler::wait_for_ready_read(int timeout)const 
{
	return lordyphon_port->waitForReadyRead(timeout);
}

//CATCHES SIGNAL OF wait_for_ready_read, READS BUFFER SIZE
void SerialHandler::onReadyRead()
{
	input_buffer = lordyphon_port->read(lordyphon_port->readBufferSize());
}


//CHECK IF OPEN
bool SerialHandler::lordyphon_port_is_open()
{
	bool retVal = false;

	try
	{
		if (lordyphon_port->isOpen())
		{
			retVal = true;
		}
	}
	catch (QException& e)
	{
		qDebug() << e.what();
	}
	
	return retVal;;
}


