#include <iostream>
#include "talk/base/basicdefs.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "prober.h"
#include "peer.h"

int main(int argc, char *argv[]) {
    //talk_base::LogMessage::LogToDebug(talk_base::LS_VERBOSE);
    talk_base::LogMessage::LogToDebug(talk_base::LS_ERROR);
    talk_base::LogMessage::LogTimestamps();
    talk_base::LogMessage::LogThreads();

    if ( argc < 4) {
        std::cout << "usage: IceProbe server local_name  remote_name" << std::endl;
    }

    IceProber *pProber = new IceProber();
    pProber->Login(argv[1], 1979, argv[2], argv[3]);
    pProber->Run();    
    
    return 0;
}

