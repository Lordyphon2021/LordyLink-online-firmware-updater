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

class HexToSerialParser
{
private:
	
	
	std::vector<QString>hex_file_vec;  // filled with hexfile records ( text line == record ) upon construction
	QByteArray serial_data_vec; // this is the data section to be sent to the microcontroller
	QVector<QByteArray> hexfile_data_vec; // contains parsed elements of complete hexfile
	QByteArray* record = nullptr;
	QListWidget* debugger;
	

public:
	
	
	HexToSerialParser( QString _path ) 
		 //constructor takes path to file location
	{
		QFile hex_file(_path);
		QString temp_record;
		hex_file.open(QIODevice::ReadOnly);

		if (hex_file.isOpen()) {				//copy hexfile to vector of strings
			while (! hex_file.atEnd()) {
				QString temp_record = hex_file.readLine();
			
			hex_file_vec.push_back(temp_record);
				temp_record.clear();
			}
			hex_file.close();
		}
		

		
	}
	HexToSerialParser(std::vector<std::string>& set_data)
	{



	}
	
	
	bool parse_hex();       
	QByteArray get_record(size_t index);
	size_t get_hexfile_size();
	QString& get_hexfile_vec(size_t index)
	{

		return hex_file_vec.at(index);

	}
	
	
	
};


