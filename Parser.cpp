#include "Parser.h"

bool CParser::get_strings_from_container(std::vector<std::string>& collection)
{
	string_collection = collection;
	
	if (string_collection.size() > 0)
		return true;
	
	return false;
}

bool CParser::parse_collection_to_bytes()
{
	for (auto i : string_collection) {
		i.substr(i.find(':'+ 1), )
	}
	
	
	
	
	return false;
}

bool CParser::send_bytes_to_uC()
{
	return false;
}
