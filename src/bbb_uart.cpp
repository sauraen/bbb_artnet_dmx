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
    FILE* capeFile;
    string namepath = UART_PATH + to_string(uartNum);
    
    /* Open slots path to load the appropriate caped */
    if((capeFile = fopen(SLOTS_PATH, "w")) < 0) {
        cerr << "INIT: UART" << uartNum << " capemgr could not be opened" << endl;
        cerr << "Is path in bbb_uart.h correct for your device?" << endl;
        return -1;
    }
    
    /* Load uart depending on the number, also checks if correct uartnum input */
    switch(uartNum) {
        case 1: 
            if(fwrite("BB-UART1", 1, 9, capeFile) < 0) {
                cerr << "INIT: UART" << uartNum << " capemgr could not be opened" << endl;
                return -1;
            }
            fclose(capeFile);
            break;

        case 2: 
            if(fwrite("BB-UART2", 1, 9, capeFile) < 0) {
                cerr << "INIT: UART" << uartNum << " capemgr could not be opened" << endl;
                return -1;
            }
            fclose(capeFile);
            break;
        
        case 3: 
            if(fwrite("BB-UART3", 1, 9, capeFile) < 0) {
                cerr << "INIT: UART" << uartNum << " capemgr could not be opened" << endl;
                return -1;
            }
            fclose(capeFile);
            break;
        
        case 4: 
            if(fwrite("BB-UART4", 1, 9, capeFile) < 0) {
                cerr << "INIT: UART" << uartNum << " capemgr could not be opened" << endl;
                return -1;
            }
            fclose(capeFile);
            break;

        case 5: 
            if(fwrite("BB-UART5", 1, 9, capeFile) < 0) {
                cerr << "INIT: UART" << uartNum << " capemgr could not be opened" << endl;
                return -1;
            }
            fclose(capeFile);
            break;

        default:
            cerr << "INIT: Invalid uartnum entered" << endl;
            return -1;
    }
    

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
    
    uartTerm.c_cflag &= ~CBAUD; //Ignoring baudrate
    uartTerm.c_cflag |= BOTHER; //termios2, other baud rate
    uartTerm.c_cflag |= CLOCAL; //Ignore control lines

    uartTerm.c_ospeed = baudRate; //Setting output rate
    uartTerm.c_cflag |= CSTOPB; //two stop bits set
    
    /* Writing termios options to uart */
    if (ioctl(uartID, TCSETS2, &uartTerm) < 0) {
        cerr << "INIT: UART" << uartNum << " Termios2 setting failure" << endl;
        return -1;
    }
    
    initFlag = 1; //init complete, recoginizing

    return 0;
}

int uArt::dmx_write(uint8* data, size_t len) {
    char zeroByte = 0x00;

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
    
    /* Writes beginning byte of 0 */
    if (write(uartID, &zeroByte, 1) < 0) {
        cerr << "dmx_write: Uart" << uartNum << " failed to write" << endl;
        return -1;
    }
    
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
}

uArtThread::uArtThread(String name, uint8* msgBuffer, int uartNumber) 
: Thread(name, 0), buffer(msgBuffer)
{   
    uart = uArt(uartNumber);
    uart.init();
    startThread();
}

uArtThread::~uArtThread()
{
    stopThread(50); //forcibly killed after 50 milliseconds
}

uArtThread::run() {
    
    while (!threadShouldExit())
    {
        wait(100);
        
        do 
        {
            const ScopedReadLock myScopedLock(myLock);

            uart.dmxWrite()
        }

        std::cout << "Hello!" << std::endl;
    }
}

ReadWriteLock uArtThread::getLock()
{
    return myLock;
}
