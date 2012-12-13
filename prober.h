#ifndef _PROBER_H_
#define _PROBER_H_

#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"

class IceProber : public sigslot::has_slots<>, public talk_base::MessageHandler {  
public:
    IceProber();
    ~IceProber(); 
    
    void onOnLine();
    void onOffline();
    void onRemoteOnline(std::string &);
    void onRemoteOffline(std::string &);
    void onRemoteMessage(std::string &, std::string &);

private:
    talk_base::Thread *signal_thread_;
    talk_base::Thread *worker_thread_;    
};

#endif
