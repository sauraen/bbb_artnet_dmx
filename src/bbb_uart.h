#ifndef UART_BBB
#define UART_BBB

#include "juce.h"

#define DMX_BAUD 250000
#define UART_PATH "/dev/ttyO"
#define SLOTS_PATH "/sys/devices/platform/bone_capemgr/slots"

class uArt
{
    
    private:
        
        int uartNum;
        int baudRate;
        int uartID;
        int initFlag;
        
    public:
        
        /*
         * Constructer 
         */
        uArt(int uartNum);
        
        /* 
         * Initializes Uart port
         *
         * Loads uart to cape manager, this requires that the proper uEnv.txt config.
         * Because of this, must run program with elavated privaledges (sudo). 
         * Refer to README for further detailed. 
         * Then opens the port and sets the appropriate options with the baud rate we want
         * for DMX, as well as two stop bits.
         */
        int init();
        
        /*
         * Write DMX packet of size len
         *
         * UART must first be initialized by calling init().
         * First sends a break for a min of 100us, then sends a high signal for min 15us
         * to satisfy DMX packet framing specifications.
         * After sends the first byte of 0s, then sends main packet of up to 512 bytes.  
         */
        int dmx_write(uint8* data, size_t len);
        
        /*
         * Deconstructor
         * 
         * Closes uArt device. 
         */
        ~uArt();
};

class uArtThread : public Thread
{

    public:
        uArtThread(String name, uint8* msgBuffer);
        
        virtual ~uArtThread();

        virtual void run() override;

        ReadWriteLock getLock();
    
    private:
        uint8* buffer;
        ReadWriteLock myLock;
        uArt uart;
};

#endif
