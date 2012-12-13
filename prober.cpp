#include "prober.h"

IceProber::IceProber() {
    signal_thread_ = NULL;
    worker_thread_ = NULL;
}

IceProber::~IceProber() {

}

void IceProber::onOnLine() {

}

void IceProber::onOffline() {

}

void IceProber::onRemoteOnline(std::string &) {

}

void IceProber::onRemoteOffline(std::string &){

}

void IceProber::onRemoteMessage(std::string &, std::string &) {

}


