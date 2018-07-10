#include <unistd.h>
#include <string.h>

#include "bbb_uart.h"

int main() {
        
    UART test = UART(4, 250000, TX, true);
    test.init();

    const char* msg = "Hello this is a test";

    while(1) {
        if (test.dmx_write((void*) msg, strlen(msg)) < 0) {
            break;
        }
        
        usleep(1000);
    }

    return 0;

}
