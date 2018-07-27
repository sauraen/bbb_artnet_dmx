#include "bbb_uart.h"
#include <juce.h>

namespace artnetNode {
    
    namespace {
        uArtThread *uthreads[4];
       
        SwitcherThread *sthread;

    

    }






    int Init() {
        uthread[0] = new uArtThread("UART1 Thread", 1);
        uthread[1] = new uArtThread("UART2 Thread", 2);
        uthread[2] = new uArtThread("UART4 Thread", 4);
        uthread[3] = new uArtThread("UART5 Thread", 5);
        
        bool errFlag = false;

        for (int i = 0; i < 4; i++){
            if (uthread[i]->init() != 0) {
                std::cerr << "uthread" << i << " failed to initialize\n";
                errFlag = true;
            }   
        }
        
        if (errFlag) {
            Finalize();
            return -1;
        }
        
        
        sthread = new SwitcherThread(*uthread[0]);
        

        return 0;
    }

    void Finalize() {
        delete sthread;

        for(int i = 0; i < 4; i++) delete uthread[i];
    }

}
