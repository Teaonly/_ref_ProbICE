#ifndef _PPSESSION_H_
#define _PPSESSION_H_

#include "talk/base/sigslot.h"
#include "talk/base/thread.h"
#include "talk/base/logging.h"
#include "talk/p2p/base/session.h"

using namespace cricket;

// A specific Session using private protocol.
class PPSession : public BaseSession {
 public:
  // Creates or destroys a session.
  PPSession(const std::string& sid, const std::string& content_type, bool isInitialtor);
  ~PPSession();

  // Updates the error state, signaling if necessary.
  virtual void SetError(Error error);

  // When the session needs to send signaling messages, it beings by requesting
  // signaling.  The client should handle this by calling OnSignalingReady once
  // it is ready to send the messages.
  // (These are called only by SessionManager.)
  sigslot::signal1<PPSession*> SignalRequestSignaling;
  void OnSignalingReady() { BaseSession::OnSignalingReady(); }

  // Invoked when we notice that there is no matching channel on our peer.
  sigslot::signal2<PPSession*, const std::string&> SignalChannelGone;

  // Takes ownership of session description.
  // TODO: Add an error argument to pass back to the caller.
  bool Initiate(const SessionDescription* sdesc);

  // When we receive an initiate, we create a session in the
  // RECEIVEDINITIATE state and respond by accepting or rejecting.
  // Takes ownership of session description.
  // TODO: Add an error argument to pass back to the caller.
  bool Accept(const SessionDescription* sdesc);
  bool Reject(const std::string& reason);
  bool Terminate() {
    return TerminateWithReason(STR_TERMINATE_SUCCESS);
  }
  bool TerminateWithReason(const std::string& reason);
  // Fired whenever we receive a terminate message along with a reason
  sigslot::signal2<PPSession*, const std::string&> SignalReceivedTerminateReason;

  // The two clients in the session may also send one another
  // arbitrary XML messages, which are called "info" messages. Sending
  // takes ownership of the given elements.  The signal does not; the
  // parent element will be deleted after the signal.
  bool SendInfoMessage(const XmlElements& elems);
  sigslot::signal2<PPSession*, const buzz::XmlElement*> SignalInfoMessage;

 private:
  // For each transport info, create a transport proxy.  Can fail for
  // incompatible transport types.
  bool CreateTransportProxies(const TransportInfos& tinfos,
                              SessionError* error);
  bool OnRemoteCandidates(const TransportInfos& tinfos,
                          ParseError* error);
 
  // Returns a TransportInfo without candidates for each content name.
  // Uses the transport_type_ of the session.
  TransportInfos GetEmptyTransportInfos(const ContentInfos& contents) const;

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

  virtual void OnMessage(talk_base::Message *pmsg);

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

  // Both versions of SendMessage send a message of the given type to
  // the other client.  Can pass either a set of elements or an
  // "action", which must have a WriteSessionAction method to go along
  // with it.  Sending with an action supports sending a "hybrid"
  // message.  Sending with elements must be sent as Jingle or Gingle.

  // When passing elems, must be either Jingle or Gingle protocol.
  // Takes ownership of action_elems.
  bool SendMessage(ActionType type, const XmlElements& action_elems,
                   SessionError* error);

  // Sends a message back to the other client indicating that we have received
  // and accepted their message.
  void SendAcknowledgementMessage(const buzz::XmlElement* stanza);

  // Once signaling is ready, the session will use this signal to request the
  // sending of each message.  When messages are received by the other client,
  // they should be handed to OnIncomingMessage.
  // (These are called only by SessionManager.)
  sigslot::signal2<PPSession* , const buzz::XmlElement*> SignalOutgoingMessage;
  void OnIncomingMessage(const SessionMessage& msg);

  void OnInitiateAcked();

  // Invoked when an error is found in an incoming message.  This is translated
  // into the appropriate XMPP response by SessionManager.
  sigslot::signal6<BaseSession*,
                   const buzz::XmlElement*,
                   const buzz::QName&,
                   const std::string&,
                   const std::string&,
                   const buzz::XmlElement*> SignalErrorMessage;


  // Handlers for the various types of messages.  These functions may take
  // pointers to the whole stanza or to just the session element.
  bool OnInitiateMessage(const SessionMessage& msg, MessageError* error);
  bool OnAcceptMessage(const SessionMessage& msg, MessageError* error);
  bool OnRejectMessage(const SessionMessage& msg, MessageError* error);
  bool OnInfoMessage(const SessionMessage& msg);
  bool OnTerminateMessage(const SessionMessage& msg, MessageError* error);
  bool OnTransportInfoMessage(const SessionMessage& msg, MessageError* error);

  // Verifies that we are in the appropriate state to receive this message.
  bool CheckState(State state, MessageError* error);

  bool initiate_acked_;
};

#endif
