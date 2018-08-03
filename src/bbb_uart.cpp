#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <string.h>
#include <stdio.h>

#include "bbb_uart.h"

using namespace std;

uArt::uArt(int num) {
    uartNum = num;    
    baudRate = DMX_BAUD;
    initFlag = 0; //init is 0 meaning not yet initialized, shouldnt be able to write/read
    uartID = -1; //setting to invalid ID as default if file not opened
}

int uArt::init() {   
    string namepath = UART_PATH + to_string(uartNum);
    
    /* Opening devide as a transmitting */
    uartID = open(namepath.c_str(), O_WRONLY | O_NOCTTY | O_NDELAY);
    
    /* Checking if opened correctly */
    if (uartID < 0) {
        cerr << "INIT: UART" << uartNum << " could not be opened" << endl;
        return -1;
    }
    
    /* Struc to be populated with uart data */
    struct termios2 uartTerm;
    
    /* getting info on uart */
    if (ioctl(uartID, TCGETS2, &uartTerm) < 0) {
        cerr << "INIT: UART" << uartNum << " Termios2 getting failure" << endl;
        return -1;
    }
    

    uartTerm.c_cflag &= ~(CSIZE | PARENB);
    uartTerm.c_cflag |= CS8;
    
    uartTerm.c_cflag &= ~CBAUD; //Ignoring baudrate
    uartTerm.c_cflag |= BOTHER; //termios2, other baud rate
    uartTerm.c_cflag |= CLOCAL; //Ignore control lines
    uartTerm.c_cflag |= CSTOPB; //two stop bits set
    
    
    uartTerm.c_oflag &= ~OPOST;
    uartTerm.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
   

    uartTerm.c_ospeed = baudRate; //Setting output rate
    
    /* Writing termios options to uart */
    if (ioctl(uartID, TCSETS2, &uartTerm) < 0) {
        cerr << "INIT: UART" << uartNum << " Termios2 setting failure" << endl;
        return -1;
    }
    
    initFlag = 1; //init complete, recoginizing

    return 0;
}

int uArt::dmx_write(uint8* data, size_t len) {
    uint8 zeroByte = 0x00;
    data[0] = zeroByte;

    if (initFlag != 1) {
        cerr << "dmx_write: Uart" << uartNum << " has not been initiated." << endl;
        return -1;
    }
    
    /* Send 0s to indicate break */
    if( ioctl(uartID, TIOCSBRK) < 0) {
        cerr << "dmx_write: Uart" << uartNum << " start of break failed" << endl;
        return -1;
    }
    
    usleep(100); //wait 100 micro seconds to ensure proper break times before msg
    
    /* Stop sending, now sending 1s as default */
    if( ioctl(uartID, TIOCCBRK) < 0) {
        cerr << "uart_dmxWrite: Uart" << uartNum << " end of break failed" << endl;
        return -1;
    }
    
    usleep(15); //wait 15 micro seconds to ensure propber MAB hold times
    
    /* Writes beginning byte of 0 
    if (write(uartID, &zeroByte, 1) < 0) {
        cerr << "dmx_write: Uart" << uartNum << " failed to write" << endl;
        return -1;
    }*/
    
    /* Writes up to 512 bytes of the actual packet, 512 max not checked */
    if (write(uartID, data, len) < 0) {
        cerr << "dmx_write: Uart" << uartNum << " failed to write" << endl;
        return -1;
    }

    return 0;
}

uArt::~uArt() {     
    if (uartID != -1) {     
        close(uartID);
    }
    std::cout << "Closing uart " << uartNum << "\n";
}

uArtThread::uArtThread(String threadName, int uartNumber) 
: Thread(threadName), uart(uartNumber)
{
    buffer = new uint8[513];
    name = threadName;
    uartNum = uartNumber;
}

int uArtThread::init()
{
    if(uart.init() == -1)
    {
        std::cerr << "UART: " << name << " Failed to initialize" << std::endl;
        return -1;
    }
    
    startThread();
    return 0;
}

void uArtThread::writeBuffer(uint8* data, uint16 len) {
    
	const ScopedReadLock myScopedLock(myLock);
	buffer[0] = 0;

	memcpy(&buffer[1], data, len);
    
}

void uArtThread::copyBuffer(uint8* destBuf, uint16 len) {
    
    const ScopedReadLock myScopedLock(myLock);
    
    memcpy(destBuf, &buffer[1], len);   

}


uArtThread::~uArtThread()
{
    delete[] buffer;
    stopThread(100); //forcibly killed after 100 milliseconds
}

void uArtThread::run() {
    
    while (!threadShouldExit())
    {
        wait(10);
        
        {
            const ScopedReadLock myScopedLock(myLock);
	
            if (uart.dmx_write(buffer, 513) == -1) {
                std::cerr << "uartThread:run():" << name << ": failed to write" << std::endl;
            }

        }
    }
}



