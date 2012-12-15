#ifndef _PPSESSION_H_
#define _PPSESSION_H_

#include "talk/base/sigslot.h"
#include "talk/base/thread.h"
#include "talk/base/logging.h"
#include "talk/p2p/base/session.h"

using namespace cricket;

class PPSession : public BaseSession {
public:
    PPSession(const std::string& sid, const std::string& content_type, bool isInitialtor);
    ~PPSession();

    virtual void OnMessage(talk_base::Message *pmsg);

    sigslot::signal1<PPSession*> SignalRequestSignaling;
    sigslot::signal2<PPSession* , const buzz::XmlElement*> SignalOutgoingMessage;
    sigslot::signal2<PPSession*, const std::string&> SignalReceivedTerminateReason;
    sigslot::signal2<PPSession*, const std::string&> SignalChannelGone;     // Invoked when we notice that there is no matching channel on our peer.
    sigslot::signal2<PPSession*, const buzz::XmlElement*> SignalInfoMessage;
    sigslot::signal6<BaseSession*,
        const buzz::XmlElement*,
        const buzz::QName&,
        const std::string&,
        const std::string&,
        const buzz::XmlElement*> SignalErrorMessage;

    virtual void SetError(Error error);
    void OnSignalingReady() { BaseSession::OnSignalingReady(); }
    void OnIncomingMessage(const SessionMessage& msg);
    bool SendInfoMessage(const XmlElements& elems);
    bool Initiate(const SessionDescription* sdesc);
    bool Accept(const SessionDescription* sdesc);
    bool Reject(const std::string& reason);
    bool Terminate() {
        return TerminateWithReason(STR_TERMINATE_SUCCESS);
    }
    bool TerminateWithReason(const std::string& reason);

private:
    // some inernal help functions
    bool CreateTransportProxies(const TransportInfos& tinfos, SessionError* error);
    TransportInfos GetEmptyTransportInfos(const ContentInfos& contents) const; // Description to transportinfo
    bool CheckState(State state, MessageError* error);
    void OnInitiateAcked();
    bool OnRemoteCandidates(const TransportInfos& tinfos,
            ParseError* error);

    // transport callback
    virtual void OnTransportRequestSignaling(Transport* transport);
    virtual void OnTransportConnecting(Transport* transport);
    virtual void OnTransportWritable(Transport* transport);
    virtual void OnTransportCandidatesReady(Transport* transport,
            const Candidates& candidates);
    virtual void OnTransportSendError(Transport* transport,
            const buzz::XmlElement* stanza,
            const buzz::QName& name,
            const std::string& type,
            const std::string& text,
            const buzz::XmlElement* extra_info);
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
    void SendAcknowledgementMessage(const buzz::XmlElement* stanza);

    // Handlers for the various types of messages.  These functions may take
    // pointers to the whole stanza or to just the session element.
    bool OnInitiateMessage(const SessionMessage& msg, MessageError* error);
    bool OnAcceptMessage(const SessionMessage& msg, MessageError* error);
    bool OnRejectMessage(const SessionMessage& msg, MessageError* error);
    bool OnInfoMessage(const SessionMessage& msg);
    bool OnTerminateMessage(const SessionMessage& msg, MessageError* error);
    bool OnTransportInfoMessage(const SessionMessage& msg, MessageError* error);

    bool initiate_acked_;
};

#endif
