#pragma once
#include <juce.h>
#include "bbb_uart.h"

class RoomLights : private Thread {
public:
    RoomLights(uArtThread *uth);
    virtual ~RoomLights();
    
    String gotMessage(String key, String value);
    
    void releaseUniverse();
private:
    virtual void run() override;
    
    uArtThread *uthread;
    ReadWriteLock lock;
    bool running;
    const static float frameperiodms;
    
    struct State {
        uint8 dmx[512];
        String name;
        
        State(){
            memset(&dmx, 0, 512);
        }
    }
    
    OwnedArray<State> states;
    State *statefrom, *stateto;
    float fadevalue, fadetime;
};
