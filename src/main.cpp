#include <iostream>
#include <string>
#include <cstring>

#include "artnetNode.h"

void printBuf(uint8* buf) {

    int currByte = 0;
    
    for (int i = 0; i < 32; i++) {
        std::cout << "  " << i;
    }
    
    std::cout << "\n";

    for (int i = 0; i < 80; i++) {
        std::count << "-";
    }
    
    /*
    for (int i = 0; i < 16; i++) {
        
        for(int j = 0; j < 32; j++) {
        
        }
        std::count << "\n"
    }
    */


}



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
        
        if (strcmp(line, "0") == 0) {
            artnetNode::copyUniBuf(dataBuf, 0);
            printBuf(dataBuf);
        }
        else if(strcmp(line, "1") == 0) {
            artnetNode::copyUniBuf(dataBuf, 1);
        }
        else if(strcmp(line, "2") == 0) {
            artnetNode::copyUniBuf(dataBuf, 2);
        }
        else if(strcmp(line, "3") == 0) {
            artnetNode::copyUniBuf(dataBuf, 3);
        }
        else if(strcmp(line, "exit") == 0) {
            
            cout << "\nExiting. . . \n"
            artnetNode::Finalize();
            cout << ". . .Finished exiting\b"
            break;
        }
        else {
            std::cout << "Unknown command, try again\n";
            continue;
        }
        
    }   
    
    
    
    return 0;
}




