#include <iostream>
#include "prober.h"

IceProber::IceProber() {
    signal_thread_ = NULL;
    worker_thread_ = NULL;
}

IceProber::~IceProber() {

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

void IceProber::onRemoteOnline(const std::string &remote) {
    std::cout << "Remote (" << remote << ") is online" << std::endl;
}

void IceProber::onRemoteOffline(const std::string &remote){
    std::cout << "Remote (" << remote << ") is offline" << std::endl;
}

void IceProber::onRemoteMessage(const std::string &remote, const std::string &msg) {
    std::cout << "Remote (" << remote << ") say to me: " << msg << std::endl;
}

void IceProber::OnMessage(talk_base::Message *msg) { 
}

