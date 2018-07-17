#include <iostream>
#include <string>


#include "bbb_uart.h"

int main()
{   
    String name = "BBB";
    uint8 msg[512];
    
    
    uArtThread th(name, msg, 4);
    
    Thread::sleep(1000);

    
    
    return 0;
}




