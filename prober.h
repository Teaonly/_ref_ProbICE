#ifndef _PROBER_H_
#define _PROBER_H_

#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"

class PPSession;

class IceProber : public sigslot::has_slots<>, public talk_base::MessageHandler {  
public:
    IceProber();
    ~IceProber(); 
    
    virtual void OnMessage(talk_base::Message *msg);
    void onOnLine(bool isOk);
    void onOffline();
    void onRemoteOnline(const std::string &);
    void onRemoteOffline(const std::string &);
    void onRemoteMessage(const std::string &, const std::string &);

private:
    PPSession *session_;
    talk_base::Thread *signal_thread_;
    talk_base::Thread *worker_thread_;    
};

#endif
