#include "artnetNode.h"

#include "bbb_uart.h"

#include <cstring>
#include <cstdio>
#include <atomic>

namespace artnetNode {
    
    namespace {
        
        CriticalSection mutex;
        
        // Program parameters
        
        uint8 net, subnet;
        bool reldmx;
        
        // uArt threads
        
        uArtThread *uthreads[4];
        
        void FinalizeUarts() {
            const ScopedLock lock(mutex);
            for(int i = 0; i < 4; i++){
                if(uthreads[i] == nullptr) continue;
                delete uthreads[i]; //uArtThread destructor calls stopThread
                uthreads[i] = nullptr;
            }
        }
        
        int InitUarts() {
            const ScopedLock lock(mutex);
            for(int i=0; i<4; ++i){
                if(uthreads[i] != nullptr){
                    std::cout << "Internal error, double-initialized uarts!\n";
                    FinalizeUarts();
                    return -1;
                }
            }
            
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
                FinalizeUarts();
                return -1;
            }
            return 0;
        }
        
        // Watchdog
        
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
                const ScopedLock lock(mutex);
                if(counter >= 20){
                    std::cout << "Began receiving Art-Net data\n";
                    if(reldmx){
                        InitUarts();
                    }
                }
                counter = 0;
            }
            
        private:
            virtual void run() override {
                while(!threadShouldExit()){
                    {
                        const ScopedLock lock(mutex);
                        if(counter < 1000){
                            ++counter;
                        }
                        if(counter == 20){
                            std::cout << "Stopped receiving Art-Net data\n";
                            if(reldmx){
                                FinalizeUarts();
                            }
                        }
                    }
                    sleep(50);
                }
            }
            
            int counter;
        };

        ArtnetWatchdog dog;
        
        
        // Art-Net
        
        void packetReceived(uint8 *buffer, int buflen, const String &senderIP, int senderport) {
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
            const ScopedLock lock(mutex);
            dog.notifyWatchdog();
            if (buffer[11] < 14) {
                std::cerr << "Artnet Version less than 14\n";
                return;
            }
            
            uint16 opcode = ((uint16*)buffer)[4];
            if(opcode == 0x2000) { //ArtPoll
                std::cout << "Received ArtPoll, reply not implemented yet\n";
            }/*else if(opcode == 0x2400){ //ArtCommand
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
                        String res = roomLights->gotMessage(key, value);
                        if(res.isNotEmpty()){
                            sendArtCommand(res, senderIP, senderport);
                        }
                    }
                    
                }
            }*/ else if(opcode == 0x5000) { //ArtDmx
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
                /*if(u == roomLightsUniverse){
                    roomLights->releaseUniverse();
                }*/
                if(uthreads[u] == nullptr){
                    std::cout << "Internal error, uart " << u << " was not initialized!\n";
                    return;
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
        
        // Networking
    	
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
                    String senderIP;
                    int senderport;
					int bytesRead = dsocket.read(buffer, 4096, false, senderIP, senderport);
                    if (bytesRead <= 0) {
						std::cerr << "UDP read error\n";
						stopThread(5);
						return;
					}
					packetReceived(buffer, bytesRead, senderIP, senderport);
				}
			}
		};
        
        DatagramSocket sock(true);
        UDPListener *listener = nullptr;
        
        //RoomLights *roomLights;
        //const static int roomLightsUniverse = 3;
        
        /*
        void sendArtCommand(const String &cmd, const String &senderIP, int senderport){
            std::cout << "Sending ArtCommand \"" << cmd << "\"\n";
            char* buffer = new char[16+512];
            strncpy(buffer, "Art-Net", 8);
            buffer[8] = 0x00;
            buffer[9] = 0x24; //Command
            buffer[10] = 0x00;
            buffer[11] = 14; //Art-Net version
            buffer[12] = 0x7F;
            buffer[13] = 0xFF; //Experimental ESTA code
            int slen = cmd.copyToUTF8(buffer, 512);
            buffer[14] = slen >> 8;
            buffer[15] = slen & 0xFF;
            sock.write(senderIP, senderport, cmd, 16 + slen);
            delete[] buffer;
        }
        */
        

    } //end anonymous namespace

    int Init(bool release_dmx) {
        net = 0;
        subnet = 0;
        reldmx = release_dmx;
        
        if(!sock.bindToPort(0x1936)) {
            std::cerr << "DatagramSocket failed to bind to port 0x1936\n";
            Finalize();
            return -1;
        }
        listener = new UDPListener(sock);
        
        for(int i=0; i<4; ++i) uthreads[i] = nullptr;
        if(!reldmx){
            InitUarts();
        }
        
        return 0;
    }

    void Finalize() {
        if (listener != nullptr) delete listener;
        sock.shutdown();
        FinalizeUarts();
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
        const ScopedLock lock(mutex);
        if(uthreads[universeNum] == nullptr){
            std::cout << "readUniverse: UART is currently shut down\n";
            return;
        }
        uthreads[universeNum]->readBuffer(destBuf, len);
    }

}
