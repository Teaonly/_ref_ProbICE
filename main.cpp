#include <iostream>
#include "prober.h"
#include "peer.h"

int main(int argc, char *argv[]) {
    
    IceProber *pProber = new IceProber();
    pProber->Login("127.0.0.1", 1979, argv[1], argv[2]);
    pProber->Run();    
    
    return 0;
}

