#include <iostream>
#include <string>
#include <cstring>

#include "artnetNode.h"

int main()
{   
	
    
    
    if(artnetNode::Init() != 0) return -1;

    
		

    for (std::string line; std::getline(std::cin, line);) {
	    if(line == "") break;
    } 
    
    
    artnetNode::Finalize();

    
    return 0;
}




