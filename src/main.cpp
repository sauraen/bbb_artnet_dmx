#include <iostream>
#include <string>
#include <stdio.h>
#include <cstring>
#include <juce.h>

#include "artnetNode.h"

void printBuf(uint8* buf) {

    int currByte = 0;
    
    printf("    ");
    for (int i = 0; i < 16; i++) {
        printf("%04d ", i);
    }
    
    std::cout << "\n";

    for (int i = 0; i < 116; i++) {
        std::cout << "-";
    }
    
    std::cout << "\n";

    
    for (int i = 0; i < 32; i++) {
        printf("%03d|", (i * 16));

        for(int j = 0; j < 16; j++) {
            printf("0x%02X ", (unsigned char) buf[currByte]);
            currByte++;
        }
        std::cout << "\n";
    }
}



int main()
{   
	
    
      
    if(artnetNode::Init() != 0) return -1;
    
    std::cout << "Initialized, Running\n";
    std::cout << "Type Universe number (1-4) and press enter to get state of universe\n" <<
                  "Type exit to finish running program\n";
    
	
    std::string line; 
    uint8 dataBuf[512]; //buffer to hold data for current state of a given universe

    while (true) {   
        std::getline(std::cin, line);
        
        if (line == "") {
            continue;
        }
        
        if (strcmp(line.c_str(), "0") == 0) {
            artnetNode::copyUniBuf(dataBuf, 0);
            printBuf(dataBuf);
        }
        else if(strcmp(line.c_str(), "1") == 0) {
            artnetNode::copyUniBuf(dataBuf, 1);
            printBuf(dataBuf);
        }
        else if(strcmp(line.c_str(), "2") == 0) {
            artnetNode::copyUniBuf(dataBuf, 2);
            printBuf(dataBuf);
        }
        else if(strcmp(line.c_str(), "3") == 0) {
            artnetNode::copyUniBuf(dataBuf, 3);
            printBuf(dataBuf);
        }
        else if(strcmp(line.c_str(), "exit") == 0) {
            
            std::cout << "\nExiting. . . \n";
            artnetNode::Finalize();
            std::cout << ". . .Finished exiting\b";
            break;
        }
        else {
            std::cout << "Unknown command, try again\n";
            continue;
        }
        
    }   
    
    
    
    return 0;
}




