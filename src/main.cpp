#include <iostream>
#include <string>
#include <cstring>

#include "artnetNode.h"

int main()
{   
	
    
      
    if(artnetNode::Init() != 0) return -1;
    
    std::cout << "Initialized, Running\n";
    std::count << "Type Universe number (1-4) and press enter to get state of universe\n" <<
                  "Type exit to finish running program\n";
    
	
    std::string line; 
    uint8 dataBuf[512]; //buffer to hold data for current state of a given universe

    std::getline(std::cin, line);
    while (std::getline(std::cin, line) != nullptr) {
	    if (line == "") {
            continue;
        }
        if (strncmp(line, "1") == 0) {
    } 
    
    
    artnetNode::Finalize();

    
    return 0;
}




