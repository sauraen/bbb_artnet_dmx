#include "RoomLights.h"

const float RoomLights::frameperiodms = 10;

RoomLights::RoomLights(uArtThread *uth) : Thread("RoomLights"), uthread(uth) {
    running = true;
    statefrom = stateto = nullptr;
    
    fadevalue = 1.0f;
    fadetime = 1.0f;
    
    //Load states
    File statesdir = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory().getChildFile("states/");
    if(!statesdir.isDirectory()){
        statesdir = statesdir.getParentDirectory().getParentDirectory().getChildFile("states/");
    }
    if(!statesdir.isDirectory()){
        std::cout << "Could not find states directory, using no states (black)\n";
    }else{
        Array<File> statefiles = statesdir.findChildFiles(File::findFiles, false, "*.st");
        for(int i=0; i<statefiles.size(); ++i){
            File f = statefiles[i];
            State *st = new State();
            states.add(st);
            if(f.getSize() < 513){
                std::cout << "State file " << f.getFileName() << " is too small\n";
            }else{
                FileInputStream fis(f);
                if(fis.failedToOpen()){
                    std::cout << "State file " << f.getFileName() << " could not be opened\n";
                }else{
                    fis.read(&st->dmx, 512);
                    st->name = fis.readString();
                    std::cout << "Successfully loaded state file " << f.getFileName() 
                            << ": \"" << st->name << "\"\n";
                }
            }
        }
        if(statefiles.size() > 0){
            statefrom = stateto = states[0];
        }else{
            std::cout << "No state files in directory\n";
        }
    }
    
    startThread();
}
RoomLights::~RoomLights(){
    stopThread(3 * frameperiodms);
    
    
}

String RoomLights::gotMessage(String key, String value){
    const ScopedWriteLock swl(lock);
    if(key.equalsIgnoreCase("state")){
        int s = value.getIntValue();
        if(s >= 0 && s < states.size()){
            if(statefrom == nullptr || stateto == nullptr){
                statefrom = stateto = states[s];
                fadevalue = 1.0f;
            }else{
                statefrom = stateto;
                stateto = states[s];
                fadevalue = 0.0f;
            }
        }
        return "";
    }else if(key.equalsIgnoreCase("querystates"){
        String ret = "roomlights.states.size=" + String(states.size());
        for(int i=0; i<states.size(); ++i){
            ret += "&roomlights.states[" << String(i) << "].name=" + states[i]->name;
        }
        return ret;
    }else if(key.equalsIgnoreCase("fadetime")){
        fadetime = value.getFloatValue();
        return "";
    }
}

void RoomLights::releaseUniverse(){
    const ScopedWriteLock swl(lock);
    running = false;
}

void RoomLights::run() {
    uint8 *tempdmx = new uint8[512];
    while(!threadShouldExit()){
        {
            const ScopedWriteLock swl(lock);
            if(running){
                if(statefrom != nullptr && stateto != nullptr){
                    //Fade between states
                    fadevalue += ((float)frameperiodms * 0.001f) / fadetime;
                    if(fadevalue > 1.0f) fadevalue = 1.0f;
                    
                    //Show faded state
                    for(int i=0; i<512; ++i){
                        float a = (float)statefrom->dmx[i];
                        float b = (float)stateto->dmx[i];
                        tempdmx[i] = (uint8)(a * (1.0f - fadevalue)
                                + b * fadevalue);
                    }
                }else{
                    memset(tempdmx, 0, 512);
                }
                uthread->writeBuffer(tempdmx, 512);
            }
        }
        sleep(frameperiodms);
    }
    delete[] tempstate;
}

