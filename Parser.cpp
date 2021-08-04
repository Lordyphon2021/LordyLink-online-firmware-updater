#include "Parser.h"
#include "ChecksumValidator.h"
#include <exception>



using namespace std;


//this method iterates through the hexfile-string-vector, (if record checksum is valid) 
// extracts all elements to user defined type "RecordElements",
//creates a std::vector<RecordElements>
//from which the serial data can be retrieved via transmit-method without losing its corresponding metadata.
//----all values hexadecimal----

bool HexToSerialParser::parse()     
{															
	QByteArray temp_data_vec;				//holds the data-section of a single hexfile record
	size_t nibbles_in_data_section = 0x00;	//how many nibbles (2 nibbles == 1 byte) are in record
	char record_type = 0x00;				//type of data in data-section
	uint16_t address = 0x0000;				//address-offset for flash-memory of microcontroller
	char checksum_from_file = 0x00;		//8-bit checksum at end of record
	uint32_t start_bytes_sum = 0x00;		//bytewise sum of all elements except start byte and checksum, used for checksum verification
	
	ChecksumValidator checksum_calculated;

	
	
	try {

		TempRecord temprecord;
		
		for (auto it : hex_file_vec) {
			
			//parse string to individual elements
			nibbles_in_data_section = static_cast<char>(stoi(it.substr(1, 2)), nullptr, 16);
			record_type = stoi(it.substr(7, 2), nullptr, 16);
			
			if (record_type == 0x01) {  //exit condition (EOF), final record will not be parsed
				return true;
			}
			
			address = static_cast<uint16_t>(stoi(it.substr(3, 4), nullptr, 16));
			checksum_from_file = stoi(it.substr(9 + (nibbles_in_data_section * 2), 2), nullptr, 16);

			//fill data_bytes_vec with single bytes of record
			temp_data_vec.push_back(':');
			temp_data_vec.push_back(static_cast<char>(nibbles_in_data_section));
			temp_data_vec.push_back(static_cast<char>((address & 0xff00) >> 8));
			temp_data_vec.push_back(static_cast<char>(address & 0xff));
			temp_data_vec.push_back(record_type);

			for (size_t rec_pos = 0; rec_pos < nibbles_in_data_section * 2; rec_pos += 2)
				temp_data_vec.push_back(static_cast<char>(stoi(it.substr((9 + rec_pos), 2), nullptr, 16)));
			
			temp_data_vec.push_back(checksum_from_file);
			
			//sum all bytes except start-byte ':' and checksum_in_file for checksum verification
			start_bytes_sum = static_cast<uint8_t>(nibbles_in_data_section) + ((address & 0xff00) >> 8) + (address & 0xff) + record_type; //address is 16 bit value, split into two bytes

			//hand data to checksum validator
			checksum_calculated.set_Data(temp_data_vec, checksum_from_file, start_bytes_sum);

			if (checksum_calculated.is_valid()) {
				//if checksum is valid, fill container with temporary object
				
				temprecord.set_data(temp_data_vec, nibbles_in_data_section, checksum_from_file);
				temprec_vec.push_back(temprecord);
			}
			else {
				cout << "checksum error!" << endl;
				return false;
			}

			//clear temp data vector for next iteration
			temp_data_vec.clear();
			
			

		}

	}
	catch (exception& e) { 

		cout << e.what();
	}
}

TempRecord HexToSerialParser::get_temprec(size_t index)
{

	return temprec_vec.at(index);


}

size_t HexToSerialParser::get_hexfile_size()
{

	return temprec_vec.size();

}



