#pragma once
#include <vector>
#include <numeric>
#include <QApplication>


//this helper class accepts extracted elements of intel hexfile 
//or any QByteArray and its corresponding checksum.

class ChecksumValidator 
{

private:

	
	QByteArray data_vec;
	uint8_t checksum_calculated;
	uint8_t checksum_from_file;
	
	
public:
	// data vec: complete record without checksum
	void set_Data(const QByteArray& _data_vec, char _checksum_from_file) 
	{
		data_vec = _data_vec;
		checksum_from_file = _checksum_from_file;
	}
	

	bool is_valid()  
	{
		
		if(data_vec.at(0) == ':')
			data_vec.remove(0, 1); // remove hexfile-header ':' for calculation
		
		
		uint32_t vec_sum = std::accumulate(data_vec.begin(), data_vec.end(), 0); 
		checksum_calculated =  ~(vec_sum & 0x00ff ) + 0x01 ;
		
		return(checksum_from_file == checksum_calculated);
	}
};