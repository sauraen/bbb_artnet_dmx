#include "artnetNode.h"
#include "bbb_uart.h"
#include "SwitcherThread.h"
#include <juce.h>
#include <cstring>

namespace artnetNode {
    
    namespace {
        uArtThread *uthreads[4];
       
        SwitcherThread *sthread;

    	class UDPListener : public Thread {
		
		public:
			UDPListener(DatagramSocket &socket) : Thread("UDPListener"),  dsocket(socket) {
				
				startThread();
			}	
				
		private:
			DatagramSocket &dsocket;
			uint8 buffer[4096];

			void run() override {
				
				while(!threadShouldExit()) {
					
					dsocket.waitUntilReady(true, -1);
					
					if(threadShouldExit()) return;

					int bytesRead = dsocket.read(buffer, 4096, false);
					
					if (bytesRead <= 0) {
						std::cerr << "UDP read error\n";
						stopThread(0);
						return;
					}
					
					packetReceived(buffer, bytesRead);
					
				}
			}


		
		}

		void packetReceived(uint8 *buffer, int buflen) {
				
			if (buflen < 12) return;
            
            if (strncmp(buffer, "Art-Net", 8) != 0) return;

            if (buffer[12] < 14) return;

            uint16 opcode = ((uint16*)buffer)[4];
            
            


			

		}

    }

			




    int Init() {
        uthreads[0] = new uArtThread("UART1 Thread", 1);
        uthreads[1] = new uArtThread("UART2 Thread", 2);
        uthreads[2] = new uArtThread("UART4 Thread", 4);
        uthreads[3] = new uArtThread("UART5 Thread", 5);
        
        bool errFlag = false;

        for (int i = 0; i < 4; i++){
            if (uthreads[i]->init() != 0) {
                std::cerr << "uthread" << i << " failed to initialize\n";
                errFlag = true;
            }   
        }
        
        if (errFlag) {
            Finalize();
            return -1;
        }
        
        
        sthread = new SwitcherThread(*uthreads[0]);
        

        return 0;
    }

    void Finalize() {
        delete sthread;

        for(int i = 0; i < 4; i++) delete uthreads[i];
    }




}


