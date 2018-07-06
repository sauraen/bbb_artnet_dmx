#ifndef UART_BBB
#define UART_BBB

#include <termios.h>

class UART {
    private:
        int uartNum;
        int baudRate;
        int uartID;
        struct termios* uartTerm;


    public:
        UART(int uartNum, int baudRate);
        
        int write(char data);
        
        int status();

        ~UART();

        
};

#endif
