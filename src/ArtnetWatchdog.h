#include <juce.h>

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
