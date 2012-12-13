#include <iostream>
#include "prober.h"
#include "peer.h"

int main(int argc, char *argv[]) {

    Peer *pPeer = new Peer("127.0.0.1", 1979, argv[1]);
    pPeer->Start();    
    
    

    talk_base::Thread* main_thread = talk_base::Thread::Current();
    main_thread->Run();
    
    return 0;
}

