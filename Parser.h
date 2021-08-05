#pragma once

#include<iostream>
#include <fstream>
#include <vector>
#include <string>
#include <QApplication>
#include <QListWidget>
#include <QFile>

class TempRecord
{

private:
	QByteArray _temp_data_vec;
	char _nibbles;
	char _checksum_from_file;

public:
	void set_data(QByteArray temp_data_vec, char nibbles_in_record, char checksum_from_file)
	{
		_temp_data_vec = temp_data_vec;
		_nibbles = nibbles_in_record;
		_checksum_from_file = checksum_from_file;

	}
	QByteArray get_hex_record()
	{
		return _temp_data_vec;
	}
	char get_checksum_in_file()
	{
		return _checksum_from_file;
	}
	char get_nibbles_in_file()
	{
		return _nibbles;
	}


};

//this class parses an intel hex-file (.txt) to serial data bytes for microcontroller bootloader section

class HexToSerialParser
{
private:
	
	
	std::vector<QString>hex_file_vec;  // filled with hexfile records ( text line == record ) upon construction
	QByteArray serial_data_vec; // this is the data section to be sent to the microcontroller
	QVector<TempRecord> temprec_vec; // contains parsed elements of complete hexfile
	QByteArray* record = nullptr;
	QListWidget* debugger;
	

public:
	
	/*
	QFile inputFile(fileName);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			...
		}
		inputFile.close();
	}

	*/
	HexToSerialParser( QString _path, QListWidget* _debugger ) 
		: debugger(_debugger) //constructor takes path to file location
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
		else
			debugger->addItem("file not found");

		for (auto i : hex_file_vec)				// debugging console output
			debugger->addItem(i);
	}
	
	
	
	bool parse();       
	TempRecord get_temprec(size_t index);
	size_t get_hexfile_size();
	QString& get_hexfile_vec(size_t index)
	{

		return hex_file_vec.at(index);

	}
	
	
	
};


