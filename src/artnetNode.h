#pragma once
#include <juce.h>

namespace artnetNode {
    
    int Init();

    void Finalize();
    
    void copyUniBuf(uint8* destBuf, uint16 universeNum, uint16 len=512);

}
