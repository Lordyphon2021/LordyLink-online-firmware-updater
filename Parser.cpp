#include "Parser.h"
#include "ChecksumValidator.h"


using namespace std;

void HexToDataBytesParser::parse_hex_to_serial_data_bytes()                         // diese methode extrahiert zeile für zeile die verschiedenen typen aus dem hex-file und befüllt den uC_data_vec
{													  // mit den temporär erzeugten uC_dat objekten
	vector<uint8_t> temp_bytes;
	size_t max_nibble_count;
	uint16_t checksum_in_file = 0;

	for (auto i : hex_file_vec) {

		max_nibble_count = stoi(i.substr(1, 2), nullptr, 16);  //wieviele halbe datenbytes sind in record(zeile)?
		checksum_in_file = stoi(i.substr(9 + max_nibble_count * 2, 2), nullptr, 16); // checksumme auslesen

		for (size_t rec_pos = 0; rec_pos < max_nibble_count * 2; rec_pos += 2)		//datenbytes der zeile in temp-vector füllen
			temp_bytes.push_back(static_cast<uint8_t>(stoi(i.substr(9 + rec_pos, 2), nullptr, 16)));

		uint8_t temp_rec_type = stoi(i.substr(7, 2), nullptr, 16);		//rec type auslesen
		size_t temp_address_count = stoi(i.substr(3, 4), nullptr, 16);	//zugehörige adresse auslesen

		uC_dat* temp_uC_data = new uC_dat(temp_address_count, checksum_in_file, temp_rec_type, temp_bytes);	//temp. uC_dat objekt erzeugen und 

		ChecksumValidator* checksum_checker = new ChecksumValidator(temp_bytes, static_cast<uint8_t>(checksum_in_file));

		if (checksum_checker->check_if_valid()) {
			uC_data_vec.push_back(*temp_uC_data);
			checksum_checker->display_calc_checksum();
		}
		else
			cout << "checksum error!" << endl;

		uint16_t checksum_in_file = 0;
		temp_bytes.clear();																	// aufräumen

		delete temp_uC_data;
		delete checksum_checker;
	}
}

void HexToDataBytesParser::transmit_bytes_to_uC()
{

	for (auto i : uC_data_vec)
		i.debug_print_bytes();
	//TODO serial port send

}

