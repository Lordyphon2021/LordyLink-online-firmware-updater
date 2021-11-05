#pragma once
#include <vector>
#include <numeric>
#include <QApplication>


//this helper class accepts extracted elements of intel hexfile 
//or any QByteArray and its corresponding checksum.

class ChecksumValidator {

private:

	QByteArray data_vec;
	uint8_t checksum8_calculated = 0;
	uint16_t checksum16_calculated = 0;
	uint8_t checksum_from_file = 0;
	uint16_t checksum_from_lordyphon = 0;
	
	
	
public:
	// data vec: complete record without checksum
	void set_Data(const QByteArray& _data_vec, char _checksum_from_file){
		data_vec = _data_vec;
		checksum_from_file = _checksum_from_file;
	}
	void set_Data(const QByteArray& _data_vec, uint16_t _checksum_from_lordyphon){
		data_vec = _data_vec;
		checksum_from_lordyphon = _checksum_from_lordyphon;
	}

	bool is_valid(){
		
		if (data_vec.at(0) == ':') {  // is hexfile
			
			data_vec.remove(0, 1); // remove hexfile-header ':' for calculation
			uint32_t vec_sum = std::accumulate(data_vec.begin(), data_vec.end(), 0);
			checksum8_calculated = ~(vec_sum & 0x00ff) + 0x01;
			qDebug() << "checksum from file: " << checksum_from_file;
			qDebug() << "local checksum: " << checksum8_calculated;
			return (checksum_from_file == checksum8_calculated);
		}
		else {						// is eeprom data

			for (auto i : data_vec)
				checksum16_calculated += static_cast<unsigned char>(i);  //qbytearray returns signed char, need unsigned for correct value
		
			qDebug() << "checksum from lordyphon: " << checksum_from_lordyphon;
			qDebug() << "local checksum: " << checksum16_calculated;
			
			return(checksum_from_lordyphon == checksum16_calculated);
		}
	}
};