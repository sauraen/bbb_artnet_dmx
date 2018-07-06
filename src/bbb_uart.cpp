#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<termios.h>
#include<string.h>

#include "bbb_uart.h"

using namespace std;

UART::UART(int num, int baud, UART_TYPE type) {
    uartNum = num;    
    baudRate = baud;
    uartType = type;

    uartID = -1;
    uartTerm = NULL;
    uartType = TX; //setting defaults
}

/*
 * Function to initialize and error check UART setup
 */
int UART::init() {
    int baud;
    
    string name = "/dev/tty0";
    string namepath = name + to_string(uartNum);
    
    /* Checking if correct baudRate was input. Some values that are acceptable in the termios structure
     * have been ommitted. */
    switch (baudRate) {
        case 1200:
            baud = B1200;
            break;
        
        case 1800:
            baud = B1800;
            break;
        
        case 2400:
            baud = B2400;
            break;

        case 4800:
            baud = B4800;
            break;

        case 9600:
            baud = B9600;
            break;

        case 19200:
            baud = B19200;
            break;

        case 38400:
            baud = B38400;
            break;

        case 57600:
            baud = B57600;
            break;

        case 115200:
            baud = B115200;
            break;

        case 230400:
            baud = B230400;
            break;

        default:
            cerr << "UART" << uartNum << " Was given incorrect, or not accounted for, baudRate" << endl;
            return -1;
    }
    
    /* Open devide in appropriate mode */
    if (uartType == TX) {
        uartID = open(namepath.c_str(), O_WRONLY | O_NOCTTY | O_NDELAY);
    }
    else if (uartType == RX) {
        uartID = open(namepath.c_str(), O_WRONLY | O_NOCTTY | O_NDELAY); //TODO
    }
    else {
        uartID = open(namepath.c_str(), O_WRONLY | O_NOCTTY | O_NDELAY); //TODO
    }
    /* Checking if opened correctly */
    if (uartID < 0) {
        cerr << "UART" << uartNum << " could not be opened" << endl;
        return -1;
    }
    
    /* Initializing termios and err checking mem space */
    uartTerm = (struct termios*) calloc(sizeof(struct termios), 1);
    
    if (uartTerm == NULL) {
        cerr << "UART" << uartNum << " mem alloc error for termios struct" << endl;
        return -1;
    }
    
    /* Initalize params of uartTerm associated with file */
    tcgetattr(uartID, uartTerm);
    
    /* Setting c_cflag (control flag) for term */
    if (uartType == TX) {
        uartTerm->c_cflag = baud | CS8 | CLOCAL | CSTOPB;
    }
    else {
        uartTerm->c_cflag = baud | CS8 | CREAD | CLOCAL | CSTOPB;
    }
    
    /* Setting c_iflag (input) */
    uartTerm->c_iflag = IGNPAR;
    
    /* Flush any outlying data */
    tcflush(uartID, TCIOFLUSH);
    
    /* Update file with new settings */
    tcsetattr(uartID, TCSANOW, uartTerm);

    /* NOTE canoncal mode set by default */

    return 0;
}

int UART::uart_write(unsigned char* data, int len) {
    
    if (write(uartID, &data, len) < 0) {
        return -1;
    }

    return 0;
}

int UART::uart_read(void* buffer, int len) {
    int count;

    if ((count = read(uartID, buffer, len) < 0)) {
        cerr << "UART" << uartNum << " Couldnt read data" << endl;
        return -1;
    }
    
    return count;
}

int UART::status() {

    return 0;
}

int UART::getID() {
    return uartID;
}

UART::~UART() {
    
    if (uartTerm != NULL) {
        free((void*) uartTerm);
    }
    
    if (uartID != -1) {     
        close(uartID);
    }
}

