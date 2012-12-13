#include <iostream>
#include "prober.h"
#include "peer.h"

int main(int argc, char *argv[]) {
    
    IceProber *pProber = new IceProber();
    Peer *pPeer = new Peer("127.0.0.1", 1979, argv[1]);
    
    pPeer->SignalOnline.connect(pProber, &IceProber::onOnLine);
    pPeer->SignalOffline.connect(pProber, &IceProber::onOffline);
    pPeer->SignalRemoteOnline.connect(pProber, &IceProber::onRemoteOnline);
    pPeer->SignalRemoteOffline.connect(pProber, &IceProber::onRemoteOffline);
    pPeer->SignalRemoteMessage.connect(pProber, &IceProber::onRemoteMessage);
    
    pPeer->Start();    
    
    talk_base::Thread* main_thread = talk_base::Thread::Current();
    main_thread->Run();
    
    return 0;
}

