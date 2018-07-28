#pragma once

#include <juce.h>
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



