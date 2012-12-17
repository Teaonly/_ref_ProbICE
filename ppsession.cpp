#include "talk/base/helpers.h"
#include "ppsession.h"

using namespace cricket;

PPSession::PPSession( const std::string& sid,
        const std::string& content_type,
        const std::string& remote,
        bool isInitialtor,
        talk_base::Thread* signal_thread,
        talk_base::Thread* worker_thread,
        PortAllocator* port_allocator)                     
: BaseSession(signal_thread,
        worker_thread,
        port_allocator,
        sid, content_type, isInitialtor) {
    remote_ = remote; 
    initiate_acked_ = false;
}

PPSession::~PPSession() {

}

void PPSession::OnMessage(talk_base::Message *pmsg) {
    BaseSession::OnMessage(pmsg);
}

void PPSession::OnIncomingMessage(const PPMessage& msg) {
    ASSERT(signaling_thread()->IsCurrent());
    ASSERT(state() == STATE_INIT || msg.from == remote_name());

    bool valid = false;
    switch (msg.type) {
        case PPMSG_SESSION_INITIATE:
            valid = OnInitiateMessage(msg);
            break;
        case PPMSG_SESSION_INFO:
            valid = OnInfoMessage(msg);
            break;
        case PPMSG_SESSION_ACCEPT:
            valid = OnAcceptMessage(msg);
            break;
        case PPMSG_SESSION_REJECT:
            valid = OnRejectMessage(msg);
            break;
        case PPMSG_SESSION_TERMINATE:
            valid = OnTerminateMessage(msg);
            break;
        case PPMSG_TRANSPORT_INFO:
            valid = OnTransportInfoMessage(msg);
            break;

        default:
            valid = false;
    }

    if (!valid) {
        SignalErrorMessage(this, msg);  
    }
}

bool PPSession::Initiate(const SessionDescription* sdesc) {
    ASSERT(signaling_thread()->IsCurrent());
    SessionError error;

    // Only from STATE_INIT
    if (state() != STATE_INIT)
        return false;

    // Setup for signaling.
    set_local_description(sdesc);
    std::vector<P2PInfo> p2pInfos;
    // TODO convert from SessionDescription to P2PInfo

    if( !CreateTransportProxies(p2pInfos)) {
        LOG(LS_ERROR) << "Could not create transports: " << error.text;
        return false;
    }

    if (!SendInitiateMessage(sdesc, &error)) {
        LOG(LS_ERROR) << "Could not send initiate message: " << error.text;
        return false;
    }

    SetState(Session::STATE_SENTINITIATE);

    SpeculativelyConnectAllTransportChannels();
    return true;

}

bool PPSession::Accept(const SessionDescription* sdesc) {
    ASSERT(signaling_thread()->IsCurrent());

    // Only if just received initiate
    if (state() != STATE_RECEIVEDINITIATE)
        return false;

    // Setup for signaling.
    set_local_description(sdesc);

    SessionError error;
    if (!SendAcceptMessage(sdesc, &error)) {
        LOG(LS_ERROR) << "Could not send accept message: " << error.text;
        return false;
    }
    // TODO - Add BUNDLE support to transport-info messages.
    MaybeEnableMuxingSupport();  // Enable transport channel mux if supported.
    SetState(Session::STATE_SENTACCEPT);
    return true;
}

bool PPSession::Reject(const std::string& reason) {
    ASSERT(signaling_thread()->IsCurrent());

    // Reject is sent in response to an initiate or modify, to reject the
    // request
    if (state() != STATE_RECEIVEDINITIATE && state() != STATE_RECEIVEDMODIFY)
        return false;

    SessionError error;
    if (!SendRejectMessage(reason, &error)) {
        LOG(LS_ERROR) << "Could not send reject message: " << error.text;
        return false;
    }

    SetState(STATE_SENTREJECT);
    return true;
}

bool PPSession::TerminateWithReason(const std::string& reason) {
    ASSERT(signaling_thread()->IsCurrent());

    // Either side can terminate, at any time.
    switch (state()) {
        case STATE_SENTTERMINATE:
        case STATE_RECEIVEDTERMINATE:
            return false;

        case STATE_SENTREJECT:
        case STATE_RECEIVEDREJECT:
            // We don't need to send terminate if we sent or received a reject...
            // it's implicit.
            break;

        default:
            SessionError error;
            if (!SendTerminateMessage(reason, &error)) {
                LOG(LS_ERROR) << "Could not send terminate message: " << error.text;
                return false;
            }
            break;
    }

    SetState(STATE_SENTTERMINATE);
    return true;
}

bool PPSession::CreateTransportProxies(std::vector<P2PInfo>& p2pInfos) {
    for (int i = 0; i < (int)p2pInfos.size(); i++) {
        GetOrCreateTransportProxy(p2pInfos[i].content_name);
    }
    return true;
}

TransportInfos PPSession::GetEmptyTransportInfos(
        const ContentInfos& contents) const {
    TransportInfos tinfos;
    for (ContentInfos::const_iterator content = contents.begin();
            content != contents.end(); ++content) {
        tinfos.push_back(
                TransportInfo(content->name, transport_type(), Candidates()));
    }
    return tinfos;
}

bool PPSession::CheckState(State expected) {
    ASSERT(state() == expected);
    if (state() != expected) {
        return false;
    }
    return true;
}

void PPSession::OnInitiateAcked() {
    // TODO: This is to work around server re-ordering
    // messages.  We send the candidates once the session-initiate
    // is acked.  Once we have fixed the server to guarantee message
    // order, we can remove this case.
    if (!initiate_acked_) {
        initiate_acked_ = true;
        SessionError error;
        SendAllUnsentTransportInfoMessages(&error);
    }
}

bool PPSession::OnRemoteCandidates(const std::vector<P2PInfo>& p2pinfos) {
    for (int i = 0; i < (int)p2pinfos.size(); i++ ) {
        const P2PInfo *tinfo = &p2pinfos[i];

        TransportProxy* transproxy = GetTransportProxy(tinfo->content_name);
        if (transproxy == NULL) {
            return false;
        }

        // Must complete negotiation before sending remote candidates, or
        // there won't be any channel impls.
        transproxy->CompleteNegotiation();
        for (Candidates::const_iterator cand = tinfo->candidates_.begin();
                cand != tinfo->candidates_.end(); ++cand) {
            
            ParseError err;
            if (!transproxy->impl()->VerifyCandidate(*cand, &err))
                return false;

            if (!transproxy->impl()->HasChannel(cand->name())) {
                return false;
            }
        }
        transproxy->impl()->OnRemoteCandidates(tinfo->candidates_);
    }

    return true;
}

void PPSession::OnTransportRequestSignaling(Transport* transport) {
    ASSERT(signaling_thread()->IsCurrent());
    SignalRequestSignaling(this);
}

void PPSession::OnTransportConnecting(Transport* transport) {
    // This is an indication that we should begin watching the writability
    // state of the transport.
    OnTransportWritable(transport);
}

void PPSession::OnTransportWritable(Transport* transport) {
    ASSERT(signaling_thread()->IsCurrent());

    // If the transport is not writable, start a timer to make sure that it
    // becomes writable within a reasonable amount of time.  If it does not, we
    // terminate since we can't actually send data.  If the transport is writable,
    // cancel the timer.  Note that writability transitions may occur repeatedly
    // during the lifetime of the session.
    /*
       signaling_thread()->Clear(this, MSG_TIMEOUT);
       if (transport->HasChannels() && !transport->writable()) {
       signaling_thread()->PostDelayed(
       10 * 1000, this, MSG_TIMEOUT);
       }
     */
}

void PPSession::OnTransportCandidatesReady(Transport* transport,
        const Candidates& candidates) {
    ASSERT(signaling_thread()->IsCurrent());
    TransportProxy* transproxy = GetTransportProxy(transport);
    if (transproxy != NULL) {
        if (initiator() && !initiate_acked_) {
            // TODO: This is to work around server re-ordering
            // messages.  We send the candidates once the session-initiate
            // is acked.  Once we have fixed the server to guarantee message
            // order, we can remove this case.
            transproxy->AddUnsentCandidates(candidates);
        } else {
            if (!transproxy->negotiated()) {
                transproxy->AddSentCandidates(candidates);
            }
            SessionError error;
            if (!SendTransportInfoMessage(transproxy, candidates, &error)) {
                LOG(LS_ERROR) << "Could not send transport info message: "
                    << error.text;
                return;
            }
        }
    }
}

void PPSession::OnTransportChannelGone(Transport* transport, const std::string& name) {
    ASSERT(signaling_thread()->IsCurrent());
    SignalChannelGone(this, name);
}

bool PPSession::OnInitiateMessage(const PPMessage& msg) {
    if (!CheckState(STATE_INIT))
        return false;

    std::vector<P2PInfo> p2pInfos;
    // TODO change msg to p2pInfos

    if (!CreateTransportProxies(p2pInfos)) {
        return false;
    }
    set_remote_description( new SessionDescription() );     //FXME empty description??

    SetState(STATE_RECEIVEDINITIATE);

    // Users of Session may listen to state change and call Reject().
    if (state() != STATE_SENTREJECT) {
        if (!OnRemoteCandidates(p2pInfos))
            return false;
    }
    return true;
}

bool PPSession::OnAcceptMessage(const PPMessage& msg) {
    if (!CheckState(STATE_SENTINITIATE))
        return false;

    std::vector<P2PInfo> p2pInfos;
    // TODO change msg to p2pInfos

    OnInitiateAcked();

    set_remote_description(new SessionDescription() );
    MaybeEnableMuxingSupport();  // Enable transport channel mux if supported.
    SetState(STATE_RECEIVEDACCEPT);

    // Users of Session may listen to state change and call Reject().
    if (state() != STATE_SENTREJECT) {
        if (!OnRemoteCandidates(p2pInfos))
            return false;
    }

    return true;
}

bool PPSession::OnRejectMessage(const PPMessage& msg) {
    if (!CheckState(STATE_SENTINITIATE))
        return false;

    SetState(STATE_RECEIVEDREJECT);
    return true;
}

bool PPSession::OnInfoMessage(const PPMessage& msg) {
    SignalInfoMessage(this, msg.argvs[0]);
    return true;
}

bool PPSession::OnTerminateMessage(const PPMessage& msg) {
    SignalReceivedTerminateReason(this, msg.argvs[0]);
    
    SetState(STATE_RECEIVEDTERMINATE);
    return true;
}

bool PPSession::OnTransportInfoMessage(const PPMessage& msg) {
    std::vector<P2PInfo> p2pInfos;
    // TODO change msg to tinfos; 

    if (!OnRemoteCandidates(p2pInfos))
        return false;

    return true;
}


bool PPSession::SendInitiateMessage(const SessionDescription* sdesc,
        SessionError* error) {
    SessionInitiate init;
    init.contents = sdesc->contents();
    init.transports = GetEmptyTransportInfos(init.contents);
    init.groups = sdesc->groups();
    //return SendMessage(ACTION_SESSION_INITIATE, init, error);
    return true;     // FIXME
}

bool PPSession::SendAcceptMessage(const SessionDescription* sdesc,
        SessionError* error) {
    //XmlElements elems;
    //return SendMessage(ACTION_SESSION_ACCEPT, elems, error);
    return true;      // FIXME
}

bool PPSession::SendRejectMessage(const std::string& reason,
        SessionError* error) {
    SessionTerminate term(reason);
    //return SendMessage(ACTION_SESSION_REJECT, term, error);
    return true;      // FIXME
}

bool PPSession::SendTerminateMessage(const std::string& reason,
        SessionError* error) {
    SessionTerminate term(reason);
    //return SendMessage(ACTION_SESSION_TERMINATE, term, error);
    return true;      // FIXME
}

bool PPSession::SendTransportInfoMessage(const TransportInfo& tinfo,
        SessionError* error) {
    //return SendMessage(ACTION_TRANSPORT_INFO, tinfo, error);
    return true;      // FIXME
}

bool PPSession::SendTransportInfoMessage(const TransportProxy* transproxy,
        const Candidates& candidates,
        SessionError* error) {
    return SendTransportInfoMessage(TransportInfo(transproxy->content_name(),
                transproxy->type(),
                candidates),
            error);
}

bool PPSession::SendAllUnsentTransportInfoMessages(SessionError* error) {
    for (TransportMap::const_iterator iter = transport_proxies().begin();
            iter != transport_proxies().end(); ++iter) {
        TransportProxy* transproxy = iter->second;
        if (transproxy->unsent_candidates().size() > 0) {
            if (!SendTransportInfoMessage(
                        transproxy, transproxy->unsent_candidates(), error)) {
                LOG(LS_ERROR) << "Could not send unsent transport info messages: "
                    << error->text;
                return false;
            }
            transproxy->ClearUnsentCandidates();
        }
    }
    return true;
}

bool PPSession::SendMessage(ActionType type, const XmlElements& action_elems,
        SessionError* error) {
    talk_base::scoped_ptr<buzz::XmlElement> stanza(
            new buzz::XmlElement(buzz::QN_IQ));

    //PPMessage msg(current_protocol_, type, id(), initiator_name());
    //msg.to = remote_name();
    //WritePPMessage(msg, action_elems, stanza.get());

    SignalOutgoingMessage(this, stanza.get());
    return true;
}


