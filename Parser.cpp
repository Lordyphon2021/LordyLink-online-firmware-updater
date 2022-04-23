#include "Parser.h"
#include "ChecksumValidator.h"
#include <exception>

using namespace std;

//this method iterates through the hexfile-string-vector, (if record checksum is valid) 
// extracts all records to QVector<QByteArray> hexfile_data_vec, getter method is used
//in worker class to send records to the microcontroller
//----all values hexadecimal----

bool Parser::parse_hex(){															
	QByteArray temp_data_vec;				//holds the data-section of a single hexfile record
	size_t data_section_size = 0x00;	//how many nibbles (2 nibbles == 1 byte) are in record
	char record_type = 0x00;				//type of data in data-section
	uint16_t address = 0x0000;				//address-offset for flash-memory of microcontroller
	char checksum_from_file = 0x00;		    //8-bit checksum at end of record
	uint32_t start_bytes_sum = 0x00;		//bytewise sum of all elements except start byte and checksum, used for checksum verification
	ChecksumValidator checksum_calculated;

	try {

		if(hexfile_data_vec.size() != 0 && hex_file_vec.back() != ":00000001FF"){
			QMessageBox error;
			error.setText("Hexfile not valid, download again!");
			error.exec();

			return false;
		}
		else if (hexfile_data_vec.size() != 0) {
			QMessageBox error;
			error.setText("Hexfile empty, download again!");
			error.exec();

			return false;
		}
		
		//TODO: change std::string to QString
		for (auto it : hex_file_vec) {
			string std_it = it.toStdString(); 
			//parse string to individual elements
			size_t len = 2;
			//parse string to individual elements
			string substr = std_it.substr(1, 2);
			string pre = "0x";
			string full_nib = pre + substr;
			
			data_section_size = static_cast<uint8_t>(stoi(full_nib, nullptr, 16));
			substr = std_it.substr(7, 2);
			string full_rec_type = pre + substr;
			
			record_type = stoi(full_rec_type, nullptr, 16);
			
			if (record_type == 0x01) {  //exit condition (EOF), final record will not be parsed
				return true;
			}
			else if (record_type == 0x03) { //this record type indicates that hexfile is bigger than 64kB
				QMessageBox error;
				error.setText("file too big!");
				error.exec();

				return false;
			}

			substr = std_it.substr(3, 4);
			string full_address = pre + substr;
			
			address = static_cast<uint16_t>(stoi(full_address, nullptr, 16));
			checksum_from_file = stoi(std_it.substr(9 + (data_section_size * 2), 2), nullptr, 16);

			//fill data_bytes_vec with single bytes of record
			temp_data_vec.push_back(':'); // header for data recognition on uC
			temp_data_vec.push_back(static_cast<char>(data_section_size));
			temp_data_vec.push_back(static_cast<char>((address & 0xff00) >> 8));
			temp_data_vec.push_back(static_cast<char>(address & 0xff));
			temp_data_vec.push_back(record_type);

			for (size_t rec_pos = 0; rec_pos < data_section_size * 2; rec_pos += 2)
				temp_data_vec.push_back(static_cast<char>(stoi(std_it.substr((9 + rec_pos), 2), nullptr, 16)));
			
			//hand data to checksum validator class
			checksum_calculated.set_Data(temp_data_vec, checksum_from_file ); // data_vec without checksum
			
			if (checksum_calculated.is_valid()) {
				//if checksum is valid:
				//add checksum to temp record vector
				temp_data_vec.push_back(checksum_from_file); 
				//add temp record to hexfile vec
				hexfile_data_vec.push_back(temp_data_vec); //THIS IS THE FINAL DATA CONTAINER
			}
			else {
				QMessageBox error;
				error.setText("checksum error!");
				error.exec();
				return false;
			}
		//clear temp record for next iteration
			temp_data_vec.clear();
		
		}//end: for (auto it : hex_file_vec) {
	}//end: try
	catch (exception& e) { 
		qDebug() << e.what();
		QMessageBox error;
		error.setText("parser error");
		error.exec();

		return false;
	}
	return true;
}

//this method extracts data from saved set files

bool Parser::parse_eeprom(){
	
	try {

		if (eeprom_file_vec.size() == 0) {
			qDebug() << "file empty";
			return false;
		}
		for (auto it : eeprom_file_vec) {
			string std_it = it.toStdString();  
	
			for (size_t rec_pos = 0; rec_pos < 32; rec_pos += 2) 
				set_serial_data_array.push_back(static_cast<char>(stoi(std_it.substr((0 + rec_pos), 2), nullptr, 16)));
			
			eeprom_data_vec.push_back(set_serial_data_array); //FINAL EEPROM DATA CONTAINER
			set_serial_data_array.clear();
		}
	}
	catch (exception& e) {
		qDebug() << e.what();
		return false;
	}
	return true;
}









