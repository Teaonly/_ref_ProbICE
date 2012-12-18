#include <iostream>
#include "talk/base/socketstream.h"
#include "talk/base/asyncsocket.h"

#include "peer.h"

#define _DEBUG_ 1

Peer::Peer(const std::string &server, const unsigned short port, const std::string &id, talk_base::Thread *worker_thread) {
    server_address_ = server;
    server_port_ = port;
    id_ = id;
    isOnline_ = true;
    sock_ = NULL;

    worker_thread_ = worker_thread;
}

Peer::~Peer() {
    if ( sock_ ) {
        sock_->Close();
        delete sock_;
    }
}

void Peer::OnMessage(talk_base::Message *msg) { 
    switch (msg->message_id) {
        case MSG_START:
            onStart_w();
    }
}

void Peer::Start() {
    worker_thread_->Post(this, MSG_START); 
}

int Peer::SendMessage(const std::string &to, const std::string &msg) {
    if ( !isOnline_ ) {
        return -1;
    }
    return 0;
}

void Peer::onStart_w() {

    // Creating socket 
    talk_base::Thread* pth = talk_base::Thread::Current();
    sock_ = pth->socketserver()->CreateAsyncSocket(SOCK_STREAM);
    sock_->SignalConnectEvent.connect(this, &Peer::onConnectEvent);
    sock_->SignalReadEvent.connect(this, &Peer::onReadEvent);
    sock_->SignalCloseEvent.connect(this, &Peer::onCloseEvent); 
    
    xmlBuffer.clear();

    // Connect to server
    talk_base::SocketAddress addr(server_address_, server_port_);
    if (sock_->Connect(addr) < 0 &&  !sock_->IsBlocking() ) {
         sock_->Close();
         delete sock_;
         sock_ = NULL;

         SignalOnline(false);
    }
}

void Peer::onConnectEvent(talk_base::AsyncSocket* socket) {

#ifdef _DEBUG_ 
    std::cout << "Socket is connected to server" << std::endl;
#endif

    if ( sock_->GetState() == talk_base::Socket::CS_CONNECTED) {
        isOnline_ = true;
        
        std::string loginMessage = "<login:" + id_ + ">";
        sock_->Send(loginMessage.c_str(), loginMessage.size() );
    
        SignalOnline(true);
    }
}

void Peer::onCloseEvent(talk_base::AsyncSocket* socket, int err) {

#ifdef _DEBUG_ 
    std::cout << "Dectected socket is closed by server or can't connect to server" << std::endl;
#endif

    sock_->Close();
    delete sock_;
    sock_ = NULL;

    if ( isOnline_ == true) {
        isOnline_ = false;
        SignalOffline();
    }
}

void Peer::onReadEvent(talk_base::AsyncSocket* socket) {
    unsigned char temp[2048];

    int ret = sock_->Recv(temp, sizeof(temp) - 1);
    if ( ret > 0) {
#ifdef _DEBUG_
        temp[ret] = 0;
        std::cout << "Get string from server = " << temp << std::endl;
#endif
        for(int i = 0;i < ret; i++) {
            xmlBuffer.push_back( temp[i] );
            if ( temp[i] == '>') {
                processXML();        
                xmlBuffer.clear();
            }
        }
    }
}

void Peer::processXML() {
    std::vector<std::string > words;
    
    words.clear();
    std::string currentWord = "";
    for(int i = 1; i < (int)xmlBuffer.size(); i++) {
        if ( xmlBuffer[i] == ':' || xmlBuffer[i] == '>') {
            words.push_back(currentWord);
            currentWord = "";
        } else {
            currentWord = currentWord.append(1, xmlBuffer[i]);
        }
    }

    if ( words.size() > 0) {
        if ( words[0] == "online" ) {
            SignalRemoteOnline(words[1]);
        } else if ( words[0] == "offline" ) {
            SignalRemoteOffline(words[1]);
        } else if ( words[0] == "login" ) {
            SignalRemoteLogin(words[1]);
        } else if ( words[0] == "message" ) {
            SignalRemoteMessage(words[1], words[2]);
        }
    }

}

