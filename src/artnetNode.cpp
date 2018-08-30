#include "artnetNode.h"
#include "bbb_uart.h"
#include <juce.h>
#include <cstring>
#include <cstdio>

namespace artnetNode {
    
    namespace {
        uArtThread *uthreads[4];
        
        DatagramSocket sock(true);
        
        uint8 net, subnet;
        
        class ArtnetWatchdog : private Thread {
        public:
            ArtnetWatchdog() : Thread("ArtnetWatchdog") {
                counter = 1000;
                startThread();
            }
            
            virtual ~ArtnetWatchdog(){
                stopThread(100);
            }
            
            void notifyWatchdog() {
                if(counter >= 20){
                    std::cout << "Began receiving Art-Net data\n";
                }
                counter = 0;
            }
            
        private:
            virtual void run() override {
                while(!threadShouldExit()){
                    if(counter == 20){
                        std::cout << "Stopped receiving Art-Net data\n";
                    }
                    if(counter < 1000) ++counter;
                    sleep(50);
                }
            }
            
            unsigned int counter;
        };
        
        ArtnetWatchdog dog;

        class RoomLights : private Thread {
        public:
            RoomLights(uArtThread *uth) : Thread("RoomLights"), uthread(uth) {
                running = true;
                
                nstates = 4;
                states = new uint8*[nstates];
                for(int i=0; i<nstates; ++i){
                    states[i] = new uint8[512];
                    uint8* s = states[i];
                    s[5] = 0x00; //UV
                    s[6] = 0xFF; //Dimmer
                    s[7] = 0xFF; //Strobe
                    switch(i){
                    case 1:
                        s[0] = 0xFF; //Red
                        s[1] = 0xD0; //Green
                        s[2] = 0x00; //Blue
                        s[3] = 0xFF; //White
                        s[4] = 0xFF; //Amber
                        break;
                    case 2:
                        s[0] = 0x80; //Red
                        s[1] = 0x80; //Green
                        s[2] = 0x00; //Blue
                        s[3] = 0xFF; //White
                        s[4] = 0xFF; //Amber
                        break;
                    case 3:
                        s[0] = 0x00; //Red
                        s[1] = 0x00; //Green
                        s[2] = 0x00; //Blue
                        s[3] = 0xFF; //White
                        s[4] = 0x00; //Amber
                        break;
                    default:
                        s[0] = 0xFF; //Red
                        s[1] = 0xFF; //Green
                        s[2] = 0xFF; //Blue
                        s[3] = 0xFF; //White
                        s[4] = 0xFF; //Amber
                        break;
                    }
                    //Copy to all fixtures
                    for(int j=8; j<512; ++j){
                        s[j] = s[j&7];
                    }
                }
                
                statefrom = stateto = 0;
                fadevalue = 1.0f;
                fadetime = 1.0f;
                
                startThread();
            }
            virtual ~RoomLights(){
                stopThread(3 * frameperiodms);
                
                for(int i=0; i<nstates; ++i){
                    delete[] states[i];
                }
                delete[] states;
            }
            
            void gotMessage(String key, String value){
                const ScopedWriteLock swl(lock);
                if(key.equalsIgnoreCase("state")){
                    statefrom = stateto;
                    stateto = value.getIntValue();
                    fadevalue = 0.0f;
                }else if(key.equalsIgnoreCase("fadetime")){
                    fadetime = value.getFloatValue();
                }
            }
            
            void releaseUniverse(){
                const ScopedWriteLock swl(lock);
                running = false;
            }
        private:
            virtual void run() override {
                uint8 *tempstate = new uint8[512];
                while(!threadShouldExit()){
                    {
                        const ScopedWriteLock swl(lock);
                        if(running){
                            //TODO: other things like time
                            
                            //Fade between states
                            fadevalue += ((float)frameperiodms * 0.001f) / fadetime;
                            if(fadevalue > 1.0f) fadevalue = 1.0f;
                            
                            //Show faded state
                            for(int i=0; i<512; ++i){
                                float a = (float)states[statefrom][i];
                                float b = (float)states[stateto][i];
                                tempstate[i] = (uint8)(a * (1.0f - fadevalue)
                                        + b * fadevalue);
                            }
                            uthread->writeBuffer(tempstate, 512);
                        }
                    }
                    sleep(frameperiodms);
                }
                delete[] tempstate;
            }
            
            uArtThread *uthread;
            ReadWriteLock lock;
            bool running;
            const float frameperiodms = 10;
            
            uint8 **states;
            int nstates;
            int statefrom, stateto;
            float fadevalue;
            
            float fadetime;
        };
        
        RoomLights *roomLights;
        const static int roomLightsUniverse = 3;
        
        void packetReceived(uint8 *buffer, int buflen) {
				
			if (buflen < 12) {
                std::cerr << "Packet less than 12 bytes\n";
                return;
            }
            if (strncmp((char*)buffer, "Art-Net", 8) != 0) {
                std::cerr << "Packet did not start with \"Art-Net\\0\"\n";
                return;
            }
            
            //Everything after this point counts as having received Art-Net data,
            //even if it is internally invalid
            dog.notifyWatchdog();
            
            if (buffer[11] < 14) {
                std::cerr << "Artnet Version less than 14\n";
                return;
            }

            uint16 opcode = ((uint16*)buffer)[4];
            
            if(opcode == 0x2000) { //ArtPoll
                std::cout << "Received ArtPoll, reply not implemented yet\n";
            }else if(opcode == 0x2400){ //ArtCommand
                if(buflen < 16){
                    std::cerr << "ArtCommand packet less than 16 bytes\n";
                    return;
                }
                uint16 slen = ((uint16)buffer[14] << 8) | buffer[15];
                if(buflen < 16 + slen){
                    std::cerr << "ArtCommand packet too small for message inside\n";
                    return;
                }
                if(slen > 512){
                    std::cerr << "ArtCommand packet with message > 512 bytes\n";
                    return;
                }
                String m = String::createStringFromData(&buffer[16], slen);
                StringArray cmds = StringArray::fromTokens(m, "&", "");
                for(int i=0; i<cmds.size(); ++i){
                    String cmd = cmds[i];
                    if(!cmd.contains("=")){
                        std::cerr << "ArtCommand command \"" << cmd << "\" invalid\n";
                        continue;
                    }
                    String key = cmd.upToFirstOccurrenceOf("=", false, true);
                    String value = cmd.fromFirstOccurrenceOf("=", false, true);
                    if(key.startsWithIgnoreCase("roomlights.")){
                        key = key.fromFirstOccurrenceOf(".", false, true); //Cut off prefix & dot
                        std::cout << "Valid roomlights command \"" << key << "=" << value << "\"\n";
                        roomLights->gotMessage(key, value);
                    }
                    
                }
            } else if(opcode == 0x5000) { //ArtDmx
                //std::cout << "ArtDmx\n";
                
                if(buflen < 20) {
                    std::cout << "ArtDmx packet too short\n";
                    return;
                }
                if(buffer[15] != net || ((buffer[14] & 0xF0) >> 4) != subnet || (buffer[14] & 0xF0) > 3) {
                    std::cout << "Packet for net " << (int) buffer[15] << " subuni " << (int) buffer[14] << "\n";
                    return;
                }
                
                uint16 dmxlen = ((uint16)buffer[16] << 8) | buffer[17];
                if (dmxlen < 2 || dmxlen > 512 || (dmxlen + 18) > buflen) {
                    std::cout << "Invalid DMX length\n";
                    return;
                }
                
                int u = buffer[14] & 0x03;
                
                if(u == roomLightsUniverse){
                    roomLights->releaseUniverse();
                }
                
                uthreads[u]->writeBuffer(&buffer[18], dmxlen);
		
		        /*
                printf("Valid DMX data: ");

                for(int i = 0; i < 16; i++) {
                    printf("%02X ", buffer[18 + i]);
                }
                printf("\n");
	 	        */
            } else if(opcode == 0x5100) { //ArtNzs
                std::cout << "Received ArtNzs, handling not implemented yet\n";
            } else if(opcode == 0x6000) { //ArtAddress
                std::cout << "Received ArtAddress, not implemented yet\n";
            } else {
                printf("Received Art-Net with opcode = 0x%04X\n", opcode);
            }
		}
    	
        class UDPListener : public Thread {
		
		public:
			UDPListener(DatagramSocket &socket) : Thread("UDPListener"),  dsocket(socket) {
				
				startThread();
			}	
		    virtual ~UDPListener() {
                stopThread(100);
            }
            
		private:
			DatagramSocket &dsocket;
			uint8 buffer[4096];

			void run() override {
		        
                std::cout << "UDPListener started listening\n";

				while(!threadShouldExit()) {
					
                    int sockCheck = 0;

                    while (sockCheck == 0) {

					    sockCheck = dsocket.waitUntilReady(true, 10);

                        
					    if(threadShouldExit()) return;
                    }
					
                    if (sockCheck < 0) {
                        std::cout << "UDPListener waitUntilReady failed\n";
                        return;
                    }

					int bytesRead = dsocket.read(buffer, 4096, false);
		            /*
                    printf("UDP received: %d bytes read. 0x%08X %08X %08X %08X\n", bytesRead,
                           ((uint32*)buffer)[0], ((uint32*)buffer)[1],((uint32*)buffer)[2],
					       ((uint32*)buffer)[3]);
	                */

                    if (bytesRead <= 0) {
						std::cerr << "UDP read error\n";
						stopThread(5);
						return;
					}
					
					packetReceived(buffer, bytesRead);
					
				}
			}
		};
        
        UDPListener *listener = nullptr;

    }

    int Init() {
        net = 0;
        subnet = 0;

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
        
        roomLights = new RoomLights(uthreads[roomLightsUniverse]);
        
        if(!sock.bindToPort(0x1936)) {
            std::cerr << "DatagramSocket failed to bind to port 0x1936\n";
            Finalize();
            return -1;
        }

        listener = new UDPListener(sock);
        
        return 0;
    }

    void Finalize() {
        
        if (listener != nullptr) delete listener;
        
        sock.shutdown();
        
        if(roomLights != nullptr) delete roomLights;
        
        for(int i = 0; i < 4; i++) delete uthreads[i];
    }

    void readUniverse(uint8* destBuf, uint16 universeNum, uint16 len) {
        
        if (destBuf == nullptr) {
            std::cout << "readUniverse: buff is null\n";
            return;
        }
        
        if (universeNum < 0 || universeNum > 3) {
            std::cout << "readUniverse: invalid universe num\n";
            return;
        }
        
        uthreads[universeNum]->readBuffer(destBuf, len);
    }


}


