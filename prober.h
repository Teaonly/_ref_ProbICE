#ifndef _PROBER_H_
#define _PROBER_H_

#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"

// predefined classes 
namespace cricket {
class BasicPortAllocator;
class TransportChannel;
class P2PTransport;
}
namespace talk_base { 
class BasicNetworkManager;
}
class PPMessage;
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
    // call back from peer
    void onOnLine(bool isOk);
    void onOffline();
    void onRemoteLongin(const std::string&);
    void onRemoteOnline(const std::string&);
    void onRemoteOffline(const std::string&);
    void onRemoteMessage(const std::string&, const std::vector<std::string>& );

    // call back from PPSession
    void onSignalRequest(PPSession *);
    void onOutgoingMessage(PPSession *, const PPMessage&);
    void onStateChanged(PPSession *);
    
    //internal helper functions
    void createSession_s();

private:
    PPSession *session_;
    Peer *peer_;
   
    cricket::Transport*             targetTransport_;
    cricket::P2PTransportChannel*   targetChannel_;
    
    bool remote_online_;
    std::string content_name_;
    std::string channel_name_;
    std::string my_name_;
    std::string remote_name_;

    // prebuild resource for ppsession 
    talk_base::BasicNetworkManager *network_manager_;
    cricket::BasicPortAllocator *port_allocator_;
    talk_base::Thread *signal_thread_;
    talk_base::Thread *worker_thread_;    
};

#endif
