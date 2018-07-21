#include <iostream>
#include <string>
#include <cstring>

#include "bbb_uart.h"

class SwitcherThread : public Thread {

	public:
		SwitcherThread(uArtThread &u) : Thread("Switcher"), uart(u) {
			startThread();
		}
		
		virtual ~SwitcherThread() {
			stopThread(10);
		}

		virtual void run() override {
			uint8 buff[512];
			
			memset(buff, 0, 512);

			while(!threadShouldExit()) {	
				
				buff[0] = 255;
				buff[1] = 0;
				buff[2] = 0;
				buff[3] = 0;
				buff[4] = 255;
				buff[5] = 0;
				buff[6] = 255;
				buff[7] = 255;
				
				uart.writeBuffer(buff);
				

				Thread::sleep(1000);
				
				if (threadShouldExit()) return;
				
				buff[0] = 0;
				buff[1] = 255;
				buff[2] = 255;
				buff[3] = 255;
				buff[4] = 0;
				buff[5] = 0;
				buff[6] = 255;
				buff[7] = 255;

				uart.writeBuffer(buff);

				Thread::sleep(1000);
			}
		}
	
	private:
		uArtThread &uart;


};


int main()
{   
	
    
    uArtThread th1("uart4", 4);

    uArtThread th2("uart2", 2);
    
    uArtThread th3("uart1", 1);
    
    uArtThread th4("uart5", 5);
	
    th1.init();
    th2.init();
    th3.init();
    th4.init();
    
    SwitcherThread switcher(th3);
    
    Thread::sleep(1000);
		

    for (std::string line; std::getline(std::cin, line);)
    {
	if(line == "") break;
    } 

    
    
    
    return 0;
}




