#include <iostream>
#include <time.h>
#include "talk/base/basicdefs.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"
#include "prober.h"
#include "peer.h"


class SimpleConsole : public sigslot::has_slots<> {
public:
    SimpleConsole() {
    }
    ~SimpleConsole() {
    }
    
    void OnPrintString(const std::string& msg) {
        time_t tim = time(NULL);
        struct tm * now = localtime(&tim);
        char *time_string = asctime(now);
        if (time_string) {
            size_t time_len = strlen(time_string);
            if (time_len > 0) {
            time_string[time_len-1] = 0;    // trim off terminating \n
            }
        }

        std::cout << time_string << " " << msg << std::endl;
    }

    void OnExit(const int ret) {
        exit(-1);
    }
}; 


int main(int argc, char *argv[]) {
    //talk_base::LogMessage::LogToDebug(talk_base::LS_VERBOSE);
    talk_base::LogMessage::LogToDebug(talk_base::LS_ERROR);
    talk_base::LogMessage::LogTimestamps();
    talk_base::LogMessage::LogThreads();

    if ( argc < 4) {
        std::cout << "usage: IceProbe server local_name  remote_name  [logfile, default is pplog.txt]" << std::endl;
    }
    
    IceProber *pProber ;
    if ( argc == 5)  {
        pProber = new IceProber(argv[4]);
    } else {
        pProber = new IceProber();
    }

    SimpleConsole myConsole;
    pProber->SignalPrintString.connect( &myConsole, &SimpleConsole::OnPrintString);
    pProber->SignalExit.connect( &myConsole, &SimpleConsole::OnExit);

    pProber->Login(argv[1], 1979, argv[2], argv[3]);
    pProber->Run();    
    
    return 0;
}

