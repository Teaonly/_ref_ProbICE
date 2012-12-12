#include "talk/base/helpers.h"
#include "ppsession.h"

namespace teaonly {

PPSession::PPSession(talk_base::Thread* signaling_thread,
                     talk_base::Thread* worker_thread,
                     cricket::PortAllocator* port_allocator)
    : cricket::BaseSession(signaling_thread, worker_thread, port_allocator,
                           talk_base::ToString(talk_base::CreateRandomId()),
                           cricket::NS_JINGLE_RTP, true) {
}
PPSession::~PPSession() {

}


}   // end of namespace
