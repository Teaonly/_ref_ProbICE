#ifndef _PPSESSION_H_
#define _PPSESSION_H_

#include "talk/base/sigslot.h"
#include "talk/base/thread.h"
#include "talk/p2p/base/session.h"

namespace teaonly {

class PPSession : public cricket::BaseSession {
public:
    PPSession(talk_base::Thread* signal_thread,
              talk_base::Thread* woker_thread,
              cricket::PortAllocator* port_allocator);
    virtual ~PPSession();

};

}
#endif
