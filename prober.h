#ifndef _PROBER_H_
#define _PROBER_H_

#include <iostream>
#include <fstream>
#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"

// predefined classes 
namespace cricket {
class BasicPortAllocator;
class P2PTransportChannel;
class TransportChannel;
class SocketMonitor;
struct ConnectionInfo;
class Transport;
}
namespace talk_base { 
class BasicNetworkManager;
}
class PPMessage;
class PPSession;
class Peer;

class IceProber : public sigslot::has_slots<>, public talk_base::MessageHandler {  
public:
    IceProber(const std::string& fname="pplog.txt");
    ~IceProber();
    
    virtual void OnMessage(talk_base::Message *msg);
    void Login(const std::string &server, 
               const unsigned short port,
               const std::string &myName,
               const std::string &remoteName);
    
    void Run();

    sigslot::signal1<const std::string&> SignalPrintString;
    sigslot::signal1<const int> SignalExit;

protected:
    // call back from peer
    void onOnLine(bool isOk);
    void onOffline();
    void onRemoteLogin(const std::string&);
    void onRemoteOnline(const std::string&);
    void onRemoteOffline(const std::string&);
    void onRemoteMessage(const std::string&, const std::vector<std::string>& );
    void onPrintString(const std::string& );
        
    // call back from PPSession
    void onSignalRequest(PPSession *);
    void onOutgoingMessage(PPSession *, const PPMessage&);
    void onStateChanged(PPSession *);
    void onSessionTimeout(PPSession *);

    // monitor call back
    void onChannelWriteable(cricket::TransportChannel*);
    void onChannelReadPacket(cricket::TransportChannel*,const char*, size_t);
    void onMonitorCallback(cricket::SocketMonitor* mo, const std::string& evt, const std::vector<cricket::ConnectionInfo>& connections);
    
    //internal helper functions
    void createSession_s();
    void doInitiate_s();
    void doAccept_s();
    void doData_s();
    void setupTarget();

private:
    PPSession *session_;
    Peer *peer_;
   
    cricket::Transport*             targetTransport_;
    cricket::P2PTransportChannel*   targetChannel_;

    std::ofstream*              output_;
    cricket::SocketMonitor*     monitor_;

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
