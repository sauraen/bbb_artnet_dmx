#include <unistd.h>
#include <string.h>

#include "bbb_uart.h"

int main() {
        
    UART test = UART(4);
    test.init();

    char msg[512];
    memset(msg, 0, 512);

    while(1) {
        if (test.dmx_write((void*) msg, 512) < 0) {
            break;
        }
        usleep(1000);
    }

    return 0;

}
