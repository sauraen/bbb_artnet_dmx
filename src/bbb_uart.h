#ifndef UART_BBB
#define UART_BBB

#define DMX_BAUD 250000
#define UART_PATH "/dev/ttyO"
#define SLOTS_PATH "/sys/devices/platform/bone_capemgr/slots"

class UART {
    
    private:
        
        int uartNum;
        int baudRate;
        int uartID;
        int initFlag;
        
    public:
        
        /*
         * Constructer 
         */
        UART(int uartNum);
        
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
        int dmx_write(void* data, size_t len);
        
        /*
         * Deconstructor
         * 
         * Closes uArt device. 
         */
        ~UART();
};

#endif
