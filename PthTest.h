#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"

class PthTest : public sigslot::has_slots<>, public talk_base::MessageHandler {  
public:
    PthTest();
    ~PthTest(); 
    
    virtual void OnMessage(talk_base::Message *msg);

private:
    talk_base::Thread *signal_thread_;
    talk_base::Thread *worker_thread_;    
};

#endif
