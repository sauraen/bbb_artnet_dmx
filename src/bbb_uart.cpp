#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<termios.h>
#include<string.h>

#include "bbb_uart.h"

using namespace std;

UART::UART(int num, int baud) {
    
    uartNum = num;    
    baudRate = baud;
       
    string name = "/dev/tty0";
    string namepath = name + to_string(num);

    if ((uartID = open(namepath.c_str(), O_WRONLY | O_NOCTTY | O_NDELAY)) < 0) {
        cerr << "UART" << num << " could not be opened" << endl;
    }
    
    uartTerm = (struct termios*) calloc(sizeof(struct termios), 1);
    

}

int UART::write(char data) {


    return 0;
}

int UART::status() {

    return 0;
}

UART::~UART() {

}

