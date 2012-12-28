#include <iostream>
#include <fstream>

#include "talk/p2p/client/basicportallocator.h"
#include "talk/p2p/base/p2ptransportchannel.h"
#include "talk/p2p/client/socketmonitor.h"
#include "talk/base/network.h"
#include "prober.h"
#include "ppsession.h"
#include "peer.h"

enum {
    MSG_CREATE_SESSION,
    MSG_DO_INITIATE,
    MSG_DO_ACCEPT,
    MSG_DO_DATA,
};

IceProber::IceProber(const std::string& fname) {
    session_ = NULL;
    peer_ = NULL;
    targetTransport_ = NULL;
    targetChannel_ = NULL;

    output_ = new std::ofstream();
    output_->open(fname.c_str(), std::ios::out); 
    monitor_ = NULL;

    network_manager_ = NULL;
    port_allocator_ = NULL;
    remote_online_ = false;

    content_name_ = "data";
    channel_name_ = "prober";

    worker_thread_ = new talk_base::Thread();
    worker_thread_->Start();
    signal_thread_ = new talk_base::Thread();
    signal_thread_->Start();
}

IceProber::~IceProber() {
    if ( signal_thread_ ) {
        signal_thread_->Stop();
        delete signal_thread_;
    }        
    if ( worker_thread_ ) {
        worker_thread_->Stop();
        delete worker_thread_;
    } 

    if( output_ && output_->is_open() ) {
        output_->close();
        delete output_;
    }
}

void IceProber::Login(const std::string &server, 
               const unsigned short port,
               const std::string& myName,
               const std::string& remoteName) {
    my_name_ = myName;
    remote_name_ = remoteName;

    network_manager_ = new talk_base::BasicNetworkManager();
    talk_base::SocketAddress address_stun("stun.l.google.com", 19302);
    talk_base::SocketAddress address_nil;
    port_allocator_ = 
        new cricket::BasicPortAllocator(network_manager_,
                                        address_stun,            //stun
                                        address_nil,            //relay:udp
                                        address_nil,            //relay:tcp
                                        address_nil);           //relay:ssl
    port_allocator_->set_flags(cricket::PORTALLOCATOR_DISABLE_TCP 
                              + cricket::PORTALLOCATOR_DISABLE_RELAY);

    peer_ =  new Peer(server, 1979, my_name_, signal_thread_);
    peer_->SignalOnline.connect(this, &IceProber::onOnLine);
    peer_->SignalOffline.connect(this, &IceProber::onOffline);
    peer_->SignalRemoteLogin.connect(this, &IceProber::onRemoteLogin);
    peer_->SignalRemoteOnline.connect(this, &IceProber::onRemoteOnline);
    peer_->SignalRemoteOffline.connect(this, &IceProber::onRemoteOffline);
    peer_->SignalRemoteMessage.connect(this, &IceProber::onRemoteMessage);
    peer_->SignalPrintString.connect(this, &IceProber::onPrintString);
    peer_->Start();
    
    signal_thread_->Post(this, MSG_CREATE_SESSION);
}

void IceProber::Run() {
    talk_base::Thread* main_thread = talk_base::Thread::Current();
    main_thread->Run();
}

void IceProber::OnMessage(talk_base::Message *msg) { 
    switch (msg->message_id) {
        case MSG_CREATE_SESSION:
            createSession_s();
            break;
        case MSG_DO_INITIATE:
            doInitiate_s();
            break;
        case MSG_DO_ACCEPT:
            doAccept_s();
            break;
        case MSG_DO_DATA:
            doData_s();
            break;
    }
}

void IceProber::onSignalRequest(PPSession *session) {
    session_->OnSignalingReady();
}

void IceProber::onOutgoingMessage(PPSession *session, const PPMessage& msg) {
    std::string msgType;
    switch(msg.type) {
        case PPMSG_SESSION_INITIATE:
            msgType = PP_STR_INITIATE;
            break;
        case PPMSG_SESSION_ACCEPT:
            msgType = PP_STR_ACCEPT;
            break;
        case PPMSG_TRANSPORT_INFO:
            msgType = PP_STR_TRANSPORT;
            break;

        default:
            return;
    }

    std::vector<std::string> msgBody;
    msgBody.push_back(msgType);
    for(int i = 0; i < (int)msg.argvs.size(); i++) {
        msgBody.push_back(msg.argvs[i]);
    }
    peer_->SendMessage( remote_name_,  msgBody);
}

void IceProber::onStateChanged(PPSession *session) {
    switch(session_->state() ) {
        case cricket::BaseSession::STATE_RECEIVEDINITIATE:
            if (session_->content_name() == content_name_ ) {
                signal_thread_->Post(this, MSG_DO_ACCEPT);
            }
            break; 
        case cricket::BaseSession::STATE_INPROGRESS:
            //std::cout << "Session is established!" << std::endl;
            SignalPrintString( "Session is established!" );
            break; 
        
        default:
            return;;
    }    
}

void IceProber::onSessionTimeout(PPSession *session) {
    // If p2ptransportchannl can't be write always, timeout will happen
    // We should send terminate to remote and shut this prober.
    
    //std::cout << "* Exception * Building p2p timeout ..." << std::endl;
    SignalPrintString ( "* Exception * Building p2p timeout ..." );

    //exit(-1);
    SignalExit();
}

void IceProber::onOnLine(bool isOk) {
    if ( isOk ) {
        //std::cout << "Connected to server is OK, wait remote..." << std::endl;
        SignalPrintString ("Connected to server is OK, wait remote...");
    } else {
        //std::cout << "Can't connect to server, please start the session server." << std::endl;
        SignalPrintString("Can't connect to server, please start the session server.");
        
        //exit(-1);
        SignalExit();
    }
}

void IceProber::onOffline() {
    //std::cout << "* Exception * Disconnected from server!" << std::endl;
    SignalPrintString("* Exception * Disconnected from server!");

    //exit(-1);
    SignalExit();
}

void IceProber::onRemoteLogin(const std::string& remote) {
    if ( remote == remote_name_ && (remote_online_== false) ) {
        remote_online_ = true;
        signal_thread_->Post(this, MSG_DO_INITIATE);
    }
}

void IceProber::onRemoteOnline(const std::string &remote) {

}

void IceProber::onRemoteOffline(const std::string &remote) {
    //std::cout << "* Exception * Remote is disconnected from server!" << std::endl;
    SignalPrintString("* Exception * Remote is disconnected from server!");

    //exit(-1);
    SignalExit();
}

void IceProber::onRemoteMessage(const std::string &remote, const std::vector<std::string>& msgBody) {
    PPMessage msg;
    if ( msgBody[0] == PP_STR_INITIATE) {
        msg.type = PPMSG_SESSION_INITIATE;                
    } else if ( msgBody[0] == PP_STR_ACCEPT) {
        msg.type = PPMSG_SESSION_ACCEPT;
    } else if ( msgBody[0] == PP_STR_TRANSPORT) {
        msg.type = PPMSG_TRANSPORT_INFO;
    } else {
        // messages don't handle 
        return;
    } 
    
    for(int i = 1; i < (int)msgBody.size(); i++) {
        msg.argvs.push_back( msgBody[i] );
    }
    
    // this function is running in signal_thread_ 
    session_->OnIncomingMessage(msg);
}

void IceProber::onPrintString(const std::string& msg) {
    SignalPrintString(msg);
}

void IceProber::onMonitorCallback(cricket::SocketMonitor* mo, const std::string& evt, const std::vector<cricket::ConnectionInfo>& connections) {
	ASSERT(monitor_ == mo);
	unsigned int now = talk_base::Time();

    *output_ << "____________________________________________________" << std::endl;     
    *output_ << "Update:" << now << std::endl;
    *output_ << "Event:" << evt << std::endl;
    *output_ << targetChannel_->ToString()  << std::endl;
    *output_ << "--------Connections----------------" << std::endl;
    for(int i = 0; i < (int)connections.size(); i++) {
        *output_ << i << "." ;
        *output_ << "\tbest:" << connections[i].best_connection;
        *output_ << "\twritable:" << connections[i].writable;
        *output_ << "\treadabel:" << connections[i].readable;
        *output_ << "\ttimeout:" << connections[i].timeout;
        *output_ << "\tnew_connection:" << connections[i].new_connection;
        *output_ << "\trtt:" << connections[i].rtt;
        *output_ << "\tsent:" << connections[i].sent_total_bytes;
        *output_ << "\trecv:" << connections[i].recv_total_bytes;
        *output_ << std::endl;
        *output_ << "\tLocal:" << connections[i].local_candidate.ToString() << std::endl;
        *output_ << "\tRemote:" << connections[i].remote_candidate.ToString() << std::endl;
    }
    *output_ << std::endl;
    *output_ << std::endl;
}

void IceProber::onChannelWriteable(cricket::TransportChannel*) {
    targetChannel_->SendPacket("ABCD", 4);
    signal_thread_->PostDelayed(1000, this, MSG_DO_DATA); 
}

void IceProber::onChannelReadPacket(cricket::TransportChannel*,const char* data, size_t len) {
    char temp[128];
    memcpy(temp, data, len);
    temp[len] = 0;

    std::string tempStr = "Get packet:";
    tempStr = tempStr + std::string((const char*)temp);

    //std::cout << "Get packet:" << temp << std::endl;
    SignalPrintString(tempStr);
}

void IceProber::createSession_s() {
    session_ = new PPSession("iceprober", signal_thread_, worker_thread_, port_allocator_);
    session_->SignalRequestSignaling.connect(this, &IceProber::onSignalRequest);
    session_->SignalOutgoingMessage.connect(this, &IceProber::onOutgoingMessage);
    session_->SignalStateChanged.connect(this, &IceProber::onStateChanged);
    session_->SignalTimeout.connect(this, &IceProber::onSessionTimeout);

    session_->set_allow_local_ips(true);
    session_->CreateChannel(content_name_, channel_name_);
}

void IceProber::doInitiate_s() {
    session_->Initiate(content_name_);
    setupTarget();    
}

void IceProber::doAccept_s() {
    session_->Accept();
    setupTarget(); 
}

void IceProber::doData_s() {
    onChannelWriteable(NULL);    
}

void IceProber::setupTarget() {
    targetTransport_ = session_->GetTransport(content_name_);
    TransportChannel* channel = targetTransport_->GetChannel(channel_name_);
	if ( channel ) {
		targetChannel_ = channel->GetP2PChannel();
        
        targetChannel_->SignalWritableState.connect(this, &IceProber::onChannelWriteable);
        targetChannel_->SignalReadPacket.connect(this, &IceProber::onChannelReadPacket);
        
        if( output_ && output_->is_open() ) {
            monitor_ = new SocketMonitor(channel, worker_thread_, signal_thread_);
            monitor_->SignalUpdate.connect(this, &IceProber::onMonitorCallback);  
            monitor_->Start(500);
        }
    }
}

