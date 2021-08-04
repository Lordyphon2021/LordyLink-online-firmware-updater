#pragma once

#include<iostream>
#include <fstream>
#include <vector>
#include <string>
#include <QApplication>

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
	
	
	std::vector<std::string>hex_file_vec;  // filled with hexfile records ( text line == record ) upon construction
	QByteArray serial_data_vec; // this is the data section to be sent to the microcontroller
	QVector<TempRecord> temprec_vec; // contains parsed elements of complete hexfile
	QByteArray* record = nullptr;

	

public:

	HexToSerialParser(const std::string& _path)  //constructor takes path to file location
	{
		std::ifstream hex_file;
		std::string temp_record;
		hex_file.open(_path);	

		if (hex_file.is_open()) {				//copy hexfile to vector of strings
			while( std::getline(hex_file, temp_record) ) {
				hex_file_vec.push_back(temp_record);
				temp_record.clear();
			}
			hex_file.close();
		}
		else
			std::cout << "file not found!" << std::endl;

		for (auto i : hex_file_vec)				// debugging console output
			std::cout << i << std::endl;
	}
	
	HexToSerialParser(const std::vector<std::string>& _hex_file_vec)  //overloaded constructor takes vector of strings containing hex-file
		: hex_file_vec(_hex_file_vec)
											
	{}
	
	
	bool parse();       
	TempRecord get_temprec(size_t index);
	size_t get_hexfile_size();
	
	
	
};


