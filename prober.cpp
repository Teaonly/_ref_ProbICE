#include <iostream>
#include "talk/p2p/client/basicportallocator.h"
#include "talk/p2p/base/p2ptransportchannel.h"
#include "talk/base/network.h"
#include "prober.h"
#include "ppsession.h"
#include "peer.h"

enum {
    MSG_CREATE_SESSION,
    MSG_DO_INITIATE,
    MSG_DO_ACCEPT,
};

IceProber::IceProber() {
    session_ = NULL;
    peer_ = NULL;
    targetTransport_ = NULL;
    targetChannel_ = NULL;

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
}

void IceProber::Login(const std::string &server, 
               const unsigned short port,
               const std::string& myName,
               const std::string& remoteName) {
    my_name_ = myName;
    remote_name_ = remoteName;

    network_manager_ = new talk_base::BasicNetworkManager();
    talk_base::SocketAddress address_stun;  // addresss_stun("baidu.com", 1979);
    talk_base::SocketAddress address_nil;
    port_allocator_ = 
        new cricket::BasicPortAllocator(network_manager_,
                                        address_stun,            //stun
                                        address_nil,            //relay:udp
                                        address_nil,            //relay:tcp
                                        address_nil);           //relay:ssl
    port_allocator_->set_flags(cricket::PORTALLOCATOR_DISABLE_TCP 
                              + cricket::PORTALLOCATOR_DISABLE_RELAY);

    peer_ =  new Peer(server, 1979, my_name_, worker_thread_);
    peer_->SignalOnline.connect(this, &IceProber::onOnLine);
    peer_->SignalOffline.connect(this, &IceProber::onOffline);
    peer_->SignalRemoteLogin.connect(this, &IceProber::onRemoteLogin);
    peer_->SignalRemoteOnline.connect(this, &IceProber::onRemoteOnline);
    peer_->SignalRemoteOffline.connect(this, &IceProber::onRemoteOffline);
    peer_->SignalRemoteMessage.connect(this, &IceProber::onRemoteMessage);
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
    }
}

void IceProber::onSignalRequest(PPSession *session) {
    session_->OnSignalingReady();
}

void IceProber::onOutgoingMessage(PPSession *session, const PPMessage& msg) {
    std::cout << "On IceProber::onOutgoingMessage " << std::endl;
}

void IceProber::onStateChanged(PPSession *session) {
    switch(session_->state() ) {
        case cricket::BaseSession::STATE_RECEIVEDINITIATE:
            if (session_->content_name() == content_name_ ) {
                signal_thread_->Post(this, MSG_DO_ACCEPT);
            }
            break; 
        default:
            return;;
    }    
}

void IceProber::onOnLine(bool isOk) {
    if ( isOk ) {
        std::cout << "Connected to server is OK" << std::endl;
    } else {
        std::cout << "Can't connect to server" << std::endl;
    }
}

void IceProber::onOffline() {
    std::cout << "Disconnect to server" << std::endl;
}

void IceProber::onRemoteLogin(const std::string& remote) {
    if ( remote == remote_name_ && (remote_online_== false) ) {
        remote_online_ = true;
        signal_thread_->Post(this, MSG_DO_INITIATE);
    }
}

void IceProber::onRemoteOnline(const std::string &remote) {
    std::cout << "Remote (" << remote << ") is online" << std::endl;
}

void IceProber::onRemoteOffline(const std::string &remote) {
    std::cout << "Remote (" << remote << ") is offline" << std::endl;
}

void IceProber::onRemoteMessage(const std::string &remote, const std::vector<std::string>& msgBody) {
    std::cout << "Remote (" << remote << ") say to me: " << msgBody[0] << std::endl;
}

void IceProber::createSession_s() {
    session_ = new PPSession("iceprober", signal_thread_, worker_thread_, port_allocator_);
    session_->SignalRequestSignaling.connect(this, &IceProber::onSignalRequest);
    session_->SignalOutgoingMessage.connect(this, &IceProber::onOutgoingMessage);
    session_->SignalStateChanged.connect(this, &IceProber::onStateChanged);

    session_->CreateChannel(content_name_, channel_name_);
}

void IceProber::doInitiate_s() {
    session_->Initiate(content_name_);
	
    targetTransport_ = session_->GetTransport(content_name_);
    TransportChannel* channel = targetTransport_->GetChannel(channel_name_);
	if ( channel ) {
		targetChannel_ = channel->GetP2PChannel();
	}
}

void IceProber::doAccept_s() {
    session_->Accept();    
}


