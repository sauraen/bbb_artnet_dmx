#pragma once
namespace artnetNode {
    
    int Init();

    void Finalize();
    
    void copyUniBuf(uint8* destBuf, uint16 len=512, uint16 universeNum);

}
