#pragma once

#include<iostream>
#include <vector>
#include <string>
#include <QMainWindow>



class HexToSerialDataParser : public QMainWindow
{
private:
	Q_OBJECT
	std::vector<std::string>hex_file_vec;  //input-format, wird im konstruktor übergeben

	class uC_dat {    //nested class, eigener datentyp, um alle informationen eines einzelnen records (zeile des hex-files) abzulegen

	private:
		size_t address_count = 0; // direct member init
		uint8_t rec_type = 0;
		uint8_t checksum_in_file = 0;
		std::vector<uint8_t> transmit_bytes;  //endformat datenbytes für den 8-bit microcontroller

	public:
		uC_dat() {} //standard konstruktor
		uC_dat(size_t _address_count, uint8_t _rec_type, uint8_t _checksum_in_file, const std::vector<uint8_t>& _transmit_bytes) //konstruktor, um temporäre objekte zu erzeugen
			:address_count(_address_count),
			rec_type(_rec_type),
			checksum_in_file(_checksum_in_file),
			transmit_bytes(_transmit_bytes)
		{}

		void debug_print_bytes()  // debuggen: gibt nur die datenbytes einer zeile auf der console aus
		{
			for (auto i : transmit_bytes)
				std::cout << std::hex << static_cast<int> (i) << " ";  // cast zu int, um in der console die bytes als hex-zahlen zu sehen, sonst kommen nur asci zeichen
		}

	}uC_data;  // objekt direkt erzeugen

	std::vector <uC_dat> uC_data_vec;  // output-format, hier landet das extrahierte endprodukt nach dem parsen.
	uint8_t checksum_calculated = 0;

public:

	HexToSerialDataParser(const std::vector<std::string>& input_vec)   //konstruktor bekommt das gesamte hex-file als vector übergeben
		: hex_file_vec(input_vec)
		
		, uC_data()	
		
		// standard-konstruktor der nested class, direct member-init
	{
		QObject(parent);
	}


	void parse_hex_to_serial_data_bytes();                         // diese methode extrahiert zeile für zeile die verschiedenen typen aus dem hex-file und befüllt den uC_data_vec
	

	void transmit_bytes_to_uC();
	
};