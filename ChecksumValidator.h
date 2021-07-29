#pragma once
#include <vector>
#include <numeric>
#include <QApplication>


//this class accepts extracted elements of intel hexfile or any uint8_t data vector and its corresponding checksum.

class ChecksumValidator 
{

private:

	uint32_t start_bytes_sum;
	QByteArray data_vec;
	uint8_t checksum_calculated;
	uint8_t checksum_from_file;
	
	
	

public:
	//start_bytes_sum is set to default 0 to accept other data than intel hex-files
	void set_Data(const QByteArray& _data_vec, const uint8_t _checksum_from_file, const uint32_t _start_bytes_sum = 0 ) 
	{
		data_vec = _data_vec;
		checksum_from_file = _checksum_from_file;
		start_bytes_sum = _start_bytes_sum;
	}
	

	bool is_valid()  // bytewise addition of data vector (and start_bytes from hexfile, if passed to constructor), 
					 //8 bit checksum is calculated from the two's complement of the least significant byte of the sum,
					 //is then compared with the passed checksum. 
	{
		uint32_t vec_sum = std::accumulate(data_vec.begin(), data_vec.end(), 0);
		checksum_calculated =  ~((start_bytes_sum + vec_sum) & 0x00ff ) + 0x01 ;
		
		return(checksum_from_file == checksum_calculated);
	}
};