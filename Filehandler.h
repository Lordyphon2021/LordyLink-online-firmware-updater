#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

//#include "LordyLink.h"



class FileHandler
{
private:

    std::ifstream file;
    std::vector<std::string> record_collection;
    
   

  
    

public:
    
    const size_t return_record_coll_size() const { return record_collection.size(); }
    std::string& record_collection_at(size_t pos) { return record_collection.at(pos); }
    
    bool GetPageContent();
    bool writeFileToContainer();

};