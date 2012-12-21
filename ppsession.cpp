#include <sstream>
#include <iostream>
#include <stdlib.h>
#include "talk/base/helpers.h"
#include "ppsession.h"

using namespace cricket;

const std::string NS_PPSESSION("ppsession");

PPSession::PPSession( const std::string& sid,
        talk_base::Thread* signal_thread,
        talk_base::Thread* worker_thread,
        PortAllocator* port_allocator)                     
: BaseSession(signal_thread,
        worker_thread,
        port_allocator,
        sid, NS_PPSESSION, true) {
    pending_candidates_ = false;
    SignalState.connect(this, &PPSession::onStateChanged);
}

PPSession::~PPSession() {
    
}

void PPSession::OnMessage(talk_base::Message *pmsg) {
    BaseSession::OnMessage(pmsg);
}

void PPSession::OnIncomingMessage(const PPMessage& msg) {
    ASSERT(signaling_thread()->IsCurrent());
    //ASSERT(state() == STATE_INIT);

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

bool PPSession::Initiate(const std::string& content) {
    ASSERT(signaling_thread()->IsCurrent());
    SessionError error;

    // Only from STATE_INIT
    if (state() != STATE_INIT)
        return false;

    // remember the my content_name
    content_name_ = content;                        
    set_local_description(new SessionDescription());
    
    P2PInfo p2pInfo;
    p2pInfo.content_name = content_name_;
    if( !CreateTransportProxies(p2pInfo)) {
        return false;
    }

    PPMessage msg;
    msg.type = PPMSG_SESSION_INITIATE;
    msg.argvs.push_back(content);
    SignalOutgoingMessage(this, msg);
    
    SetState(Session::STATE_SENTINITIATE);
    SpeculativelyConnectAllTransportChannels();
    return true;
}

bool PPSession::Accept() {
    ASSERT(signaling_thread()->IsCurrent());

    // Only if just received initiate
    if (state() != STATE_RECEIVEDINITIATE)
        return false;

    // Setup for signaling.
    set_local_description(new SessionDescription());

    PPMessage msg;  
    msg.type = PPMSG_SESSION_ACCEPT;
    msg.argvs.push_back(content_name_);     //we must make sure content equals recved by initiate
    SignalOutgoingMessage(this, msg);
    
    SetState(Session::STATE_SENTACCEPT);

    SendAllUnsentTransportInfoMessages();
    return true;
}

bool PPSession::Reject(const std::string& reason) {
    ASSERT(signaling_thread()->IsCurrent());

    // Reject is sent in response to an initiate or modify, to reject the
    // request
    if (state() != STATE_RECEIVEDINITIATE && state() != STATE_RECEIVEDMODIFY)
        return false;
    
    PPMessage msg;
    msg.type = PPMSG_SESSION_REJECT;
    SignalOutgoingMessage(this, msg);

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
            PPMessage msg;
            msg.type = PPMSG_SESSION_TERMINATE;
            SignalOutgoingMessage(this, msg);
            break;
    }

    SetState(STATE_SENTTERMINATE);
    return true;
}

bool PPSession::CheckState(State expected) {
    ASSERT(state() == expected);
    if (state() != expected) {
        return false;
    }
    return true;
}

bool PPSession::CreateTransportProxies(const P2PInfo& p2pInfo) {
    pending_candidates_ = true;
    GetOrCreateTransportProxy(p2pInfo.content_name);
    return true;
}


bool PPSession::OnRemoteCandidates(const P2PInfo& p2pinfos) {
    const P2PInfo *tinfo = &p2pinfos;

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
    std::cout << " PPSession::OnTransportCandidatesReady " << std::endl;
    TransportProxy* transproxy = GetTransportProxy(transport);
    if (transproxy != NULL) {
        if (pending_candidates_) {
            transproxy->AddUnsentCandidates(candidates);
            std::cout << " get new candidates" << std::endl;
        } else {
            if (!SendTransportInfoMessage(transproxy, candidates)) {
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

    // remember my content_name_
    content_name_ = msg.argvs[0];
    set_remote_description( new SessionDescription() ); 

    P2PInfo newP2PInfo;
    newP2PInfo.content_name = msg.argvs[0];
    if (!CreateTransportProxies(newP2PInfo)) {
        return false;
    }
    

    SetState(STATE_RECEIVEDINITIATE);

    // Users of Session may listen to state change and call Reject().
    if (state() != STATE_SENTREJECT) {
        if (!OnRemoteCandidates(newP2PInfo))
            return false;
    }
    return true;
}

bool PPSession::OnAcceptMessage(const PPMessage& msg) {
    if (!CheckState(STATE_SENTINITIATE))
        return false;

    P2PInfo newP2PInfo;
    newP2PInfo.content_name = msg.argvs[0];

    SendAllUnsentTransportInfoMessages();

    set_remote_description(new SessionDescription() );
    MaybeEnableMuxingSupport();  // Enable transport channel mux if supported.
    SetState(STATE_RECEIVEDACCEPT);

    // Users of Session may listen to state change and call Reject().
    if (state() != STATE_SENTREJECT) {
        if (!OnRemoteCandidates(newP2PInfo))
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
    P2PInfo p2pInfo;
    p2pInfo.content_name = msg.argvs[0]; 
    
    int nCandidates = (msg.argvs.size() - 1) / 10;
    for (int i = 0; i < nCandidates; i++) {
        cricket::Candidate cand;
        talk_base::SocketAddress addr( msg.argvs[i*10+1+3], atoi(msg.argvs[i*10+1+4].c_str()) );
        cand.set_name( msg.argvs[i*10+1+0] );
        cand.set_protocol( msg.argvs[i*10+1+1] );
        cand.set_preference_str( msg.argvs[i*10+1+2] );
        cand.set_address(addr); //3,4
        cand.set_username(msg.argvs[i*10+1+5]);
        cand.set_password(msg.argvs[i*10+1+6]);
        cand.set_type(msg.argvs[i*10+1+7]);
        cand.set_network_name(msg.argvs[i*10+1+8]);
        cand.set_generation_str(msg.argvs[i*10+1+9]);
		p2pInfo.candidates_.push_back(cand);
    }

    if (!OnRemoteCandidates(p2pInfo))
        return false;

    return true;
}

bool PPSession::SendTransportInfoMessage(const TransportProxy* transproxy, const Candidates& candidates) {
    PPMessage msg;
    msg.type = PPMSG_TRANSPORT_INFO;
    msg.argvs.push_back( transproxy->content_name() );
    
    for( int i = 0; i < (int)candidates.size(); i++) {
        msg.argvs.push_back(candidates[i].name() );
        msg.argvs.push_back(candidates[i].protocol());
        msg.argvs.push_back(candidates[i].preference_str());
        msg.argvs.push_back(candidates[i].address().IPAsString());
        msg.argvs.push_back(candidates[i].address().PortAsString());
        msg.argvs.push_back(candidates[i].username());
        msg.argvs.push_back(candidates[i].password());
        msg.argvs.push_back(candidates[i].type() );
        msg.argvs.push_back(candidates[i].network_name());
        msg.argvs.push_back(candidates[i].generation_str());
    }
    
    SignalOutgoingMessage(this, msg);
    return true;
}

bool PPSession::SendAllUnsentTransportInfoMessages() {
    pending_candidates_ = false;
    for (TransportMap::const_iterator iter = transport_proxies().begin();
            iter != transport_proxies().end(); ++iter) {
        TransportProxy* transproxy = iter->second;
        if (transproxy->unsent_candidates().size() > 0) {
            if (!SendTransportInfoMessage( transproxy, transproxy->unsent_candidates())) {
                return false;
            }
            transproxy->ClearUnsentCandidates();
        }
    }
    return true;
}

void PPSession::onStateChanged(BaseSession* session, BaseSession::State newState) {
    SignalStateChanged(this);
}
