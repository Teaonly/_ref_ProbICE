#ifndef _PROBER_H_
#define _PROBER_H_

#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"


// predefined classes 
namespace cricket {
class HttpPortAllocator;
}
namespace talk_base { 
class BasicNetworkManager;
}
class PPSession;
class Peer;

class IceProber : public sigslot::has_slots<>, public talk_base::MessageHandler {  
public:
    IceProber();
    ~IceProber();
    
    virtual void OnMessage(talk_base::Message *msg);
    void Login(const std::string &server, 
               const unsigned short port,
               const std::string &myName,
               const std::string &remoteName);
    
    void Run();

protected:
    void onOnLine(bool isOk);
    void onOffline();
    void onRemoteOnline(const std::string &);
    void onRemoteOffline(const std::string &);
    void onRemoteMessage(const std::string &, const std::string &);

private:
    PPSession *session_;
    Peer *peer_;

    std::string my_name_;
    std::string remote_name_;
    talk_base::Thread *signal_thread_;
    talk_base::Thread *worker_thread_;    
    
    talk_base::BasicNetworkManager *network_manager_;
    cricket::HttpPortAllocator *port_allocator_;
};

#endif
