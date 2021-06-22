#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>



class CParser
{
private:

    std::ifstream hex_file;
    std::string _path;
    std::vector<char> hex_container;
    char record_dataBytes = 0;
    std::vector<char> tx_data_bytes;
    size_t length = 0;
    std::string single_record_string;
    std::vector<std::string> record_collection;

    bool parseFile();
    bool sendParsedToUsb();
    

public:

    bool writeFileToContainer();
    
    const size_t return_record_coll_size()const
    {
        return record_collection.size();
    }
    
    std::string& record_collection_at(size_t pos)
    {

        return record_collection.at(pos);

    }
    
    bool GetPageContent();





};