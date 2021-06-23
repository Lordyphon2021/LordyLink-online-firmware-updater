#pragma once
#include<string>
#include <array>
#include <vector>





class CParser
{

private:
	std::vector<std::string> string_collection;
	std::vector <char> bytes_to_send;
	 
public:
	bool get_strings_from_container(const std::vector<std::string>& collection); // read only
	bool parse_collection_to_bytes();
	bool send_bytes_to_uC();

};
