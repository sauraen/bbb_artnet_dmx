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
        printf("%02d ", i);
    }
    
    std::cout << "\n";

    for (int i = 0; i < ((16 * 3) + 3); i++) {
        std::cout << "-";
    }
    
    std::cout << "\n";

    
    for (int i = 0; i < 32; i++) {
        printf("%03d|", (i * 16) + 1);

        for(int j = 0; j < 16; j++) {
            printf("%02X ", (unsigned char) buf[currByte]);
            currByte++;
        }
        std::cout << "\n";
    }
}



int main()
{
    if(artnetNode::Init() != 0) return -1;
    
    std::cout << "Initialized, Running\n";
    std::cout << "Type universe number (0-3) and press enter to print the state of the universe\n" <<
                  "Type \"exit\" to shut down the program\n";
    
    std::string line; 
    uint8 dataBuf[512]; //buffer to hold data for current state of a given universe

    while (true) {   
        std::getline(std::cin, line);
        
        if (line == "") {
            continue;
        }
        
        if(line[1] == 0 && line[0] >= '0' && line[0] <= '3') {
            artnetNode::readUniverse(dataBuf, line[0] - '0');  
            printBuf(dataBuf);
        } else if(strcmp(line.c_str(), "exit") == 0) {
            std::cout << "\nExiting. . . \n";
            artnetNode::Finalize();
            std::cout << ". . .Finished exiting\n";
            break;
        } else {
            std::cout << "Unknown command, try again\n";
            continue;
        }
        
    }   
    
    return 0;
}




