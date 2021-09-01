#pragma once

#include<iostream>
#include <fstream>
#include <vector>
#include <string>
#include <QApplication>
#include <QListWidget>
#include <QFile>
#include <QMessageBox>
#include <qdebug>




//this class parses an intel hex-file (.txt) to serial data bytes for microcontroller bootloader section

class Parser
{
private:
	
	std::vector<QString>eeprom_file_vec;  // filled with lordyphon set data
	std::vector<QString>hex_file_vec;  // filled with hexfile records ( text line == record ) upon construction
	QByteArray hex_serial_data_array; // this is the data section to be sent to the microcontroller
	QByteArray set_serial_data_array;
	QVector<QByteArray> hexfile_data_vec; // contains parsed elements of complete hexfile
	QVector<QByteArray>  eeprom_data_vec;
	//QByteArray* record = nullptr;
	QListWidget* debugger;
	

public:
	
	
	Parser( QString _path ) 
		 //constructor takes path to file location
	{
		if(! _path.contains(".txt"))
			_path += ".txt";
		qDebug() << "in parser :" <<_path;
		
		
		QFile data_file(_path);
		QString temp_record;
		data_file.open(QIODevice::ReadOnly);

		if (data_file.isOpen()) {				//copy hexfile to vector of strings
			while (! data_file.atEnd()) {
				QString temp_record = data_file.readLine();
				
				
				
				
				if (temp_record.at(0) == ':') {     //if hexfile, load into hexfile-vec
					hex_file_vec.push_back(temp_record);
					temp_record.clear();

				}
				else {							//else load into eeprom data vec

					eeprom_file_vec.push_back(temp_record);
					temp_record.clear();

				}

				
				
			}
			data_file.close();
		}
		

		
	}
	
	
	//parse methods implemented in cpp file
	bool parse_hex();  
	bool parse_eeprom();
	
	
	//getter methods implemented here:
	
	QByteArray get_hex_record(size_t index)
	{
		return hexfile_data_vec.at(index);

	}
	
	
	QByteArray get_eeprom_record(size_t index)
	{
		return eeprom_data_vec.at(index);

	}
	
	
	size_t get_hexfile_size()
	{

		return hexfile_data_vec.size();

	}
	
	size_t get_eeprom_size() 
	{

		return eeprom_data_vec.size();


	}
	
	QString& get_eeprom_vec(size_t index)
	{

		return hex_file_vec.at(index);

	}
	uint16_t get_eeprom_checksum()
	{
		uint16_t checksum = 0;
		
		
		
		for (auto qbytearray : eeprom_data_vec) {

			for (int j = 0; j < 16; ++j)
				checksum += qbytearray.at(j);



		}
			
		
		return checksum;

	}
	
	
	QString& get_hexfile_vec(size_t index)
	{

		return eeprom_file_vec.at(index);

	}
	
	
	
};


