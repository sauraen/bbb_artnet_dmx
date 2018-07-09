#include "bbb_uart.h"

int main() {

    UART uart4 = UART(4, 250000, TX, true);
    uart4.init();
    
    return 0;
}
