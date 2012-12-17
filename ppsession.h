#ifndef _PPSESSION_H_
#define _PPSESSION_H_

#include "talk/base/sigslot.h"
#include "talk/base/thread.h"
#include "talk/base/logging.h"
#include "talk/p2p/base/session.h"
#include "ppmessage.h"

using namespace cricket;

class PPSession : public BaseSession {
public:
    PPSession(const std::string& sid, 
              const std::string& content_type,
              const std::string& remote, 
              bool isInitialtor,
              talk_base::Thread* signal_thread,
              talk_base::Thread* worker_thread,
              PortAllocator* port_allocator);
    ~PPSession();

    sigslot::signal1<PPSession*> SignalRequestSignaling;
    sigslot::signal2<PPSession* , const buzz::XmlElement*> SignalOutgoingMessage;
    sigslot::signal2<PPSession*, const std::string&> SignalReceivedTerminateReason;
    sigslot::signal2<PPSession*, const std::string&> SignalChannelGone;     // Invoked when we notice that there is no matching channel on our peer.
    sigslot::signal2<PPSession*, const std::string&> SignalInfoMessage;
    sigslot::signal2<PPSession*, const PPMessage&> SignalErrorMessage;

    virtual void OnMessage(talk_base::Message *pmsg);
    std::string remote_name() {return remote_;}
    void OnSignalingReady() { BaseSession::OnSignalingReady(); }
    void OnIncomingMessage(const PPMessage& msg);
    
    bool Initiate(const SessionDescription* sdesc);
    bool Accept(const SessionDescription* sdesc);
    bool Reject(const std::string& reason);
    bool Terminate() {
        return TerminateWithReason(STR_TERMINATE_SUCCESS);
    }
    bool TerminateWithReason(const std::string& reason);

private:
    // some inernal help functions
    bool CreateTransportProxies(std::vector<P2PInfo>& p2pInfos);
    TransportInfos GetEmptyTransportInfos(const ContentInfos& contents) const; // Description to transportinfo
    bool CheckState(State state);
    void OnInitiateAcked();
    bool OnRemoteCandidates(const std::vector<P2PInfo>& p2pInfos);

    // transport callback
    virtual void OnTransportRequestSignaling(Transport* transport);
    virtual void OnTransportConnecting(Transport* transport);
    virtual void OnTransportWritable(Transport* transport);
    virtual void OnTransportCandidatesReady(Transport* transport,
            const Candidates& candidates);
    virtual void OnTransportChannelGone(Transport* transport,
            const std::string& name);

    // Send various kinds of session messages.
    bool SendInitiateMessage(const SessionDescription* sdesc,
            SessionError* error);
    bool SendAcceptMessage(const SessionDescription* sdesc, SessionError* error);
    bool SendRejectMessage(const std::string& reason, SessionError* error);
    bool SendTerminateMessage(const std::string& reason, SessionError* error);
    bool SendTransportInfoMessage(const TransportInfo& tinfo,
            SessionError* error);
    bool SendTransportInfoMessage(const TransportProxy* transproxy,
            const Candidates& candidates,
            SessionError* error);
    bool SendAllUnsentTransportInfoMessages(SessionError* error);
    bool SendMessage(ActionType type, const XmlElements& action_elems,
            SessionError* error);

    // Handlers for the various types of messages.  These functions may take
    // pointers to the whole stanza or to just the session element.
    bool OnInitiateMessage(const PPMessage& msg);
    bool OnAcceptMessage(const PPMessage& msg);
    bool OnRejectMessage(const PPMessage& msg);
    bool OnInfoMessage(const PPMessage& msg);
    bool OnTerminateMessage(const PPMessage& msg);
    bool OnTransportInfoMessage(const PPMessage& msg);
    
    std::string remote_;
    bool initiate_acked_;
};

#endif
