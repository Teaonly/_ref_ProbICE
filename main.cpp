#include <iostream>
#include "talk/base/basicdefs.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "prober.h"
#include "peer.h"

int main(int argc, char *argv[]) {
    talk_base::LogMessage::LogThreads();
    talk_base::LogMessage::LogTimestamps();

   
    IceProber *pProber = new IceProber();
    pProber->Login("127.0.0.1", 1979, argv[1], argv[2]);
    pProber->Run();    
    
    return 0;
}

