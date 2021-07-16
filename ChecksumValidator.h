#pragma once
#include <vector>
#include <numeric>


class ChecksumValidator
{

private:

	
	std::vector <uint8_t> data_vec;
	uint8_t checksum8_calc = 0;
	uint8_t checksum8_input = 0;
	uint16_t checksum16_calc = 0;
	uint16_t checksum16_input = 0;
	bool bit_mode = 0;

public:

	ChecksumValidator(const std::vector<uint8_t>& input_vec, uint8_t checksum8_from_file)
	{
		data_vec = input_vec;
		checksum8_input = checksum8_from_file;
		bit_mode = 0;
	}
	ChecksumValidator(const std::vector<uint8_t>& input_vec, uint16_t checksum32_from_file)
	{
		data_vec = input_vec;
		checksum16_input = checksum32_from_file;
		bit_mode = 1;
	}

	bool check_if_valid()
	{
		
		uint32_t temp_sum = std::accumulate(data_vec.begin(), data_vec.end(), 0);

		if (bit_mode == 0) {
			checksum8_calc = static_cast <uint8_t> ( ~( temp_sum & 0xff ) + 1 );
			return (checksum8_input == checksum8_calc);
		}
		else{
			checksum16_calc = static_cast <uint16_t> ( ~( temp_sum & 0xffff ) + 1 );
			return (checksum16_input == checksum16_calc);
		}
	}
	void display_calc_checksum()
	{
		
		bit_mode == 0 ? (std::cout << "calculated 8 bit checksum: " << std::hex << static_cast <int>(checksum8_calc) << std::endl)
			: (std::cout << "calculated 16 bit checksum: " << std::hex << static_cast <int>(checksum16_calc) << std::endl);
	}

};