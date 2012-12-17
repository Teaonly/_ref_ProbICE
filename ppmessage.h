#ifndef _PPMESSAGE_H_
#define _PPMESSAGE_H_

#include <string>
#include <vector>
#include <map>

#include "talk/base/basictypes.h"
#include "talk/p2p/base/candidate.h"
#include "talk/p2p/base/constants.h"
#include "talk/p2p/base/parsing.h"
#include "talk/p2p/base/sessiondescription.h"  // Needed to delete contents.
#include "talk/xmllite/xmlelement.h"

namespace cricket {
class Candidate;
}

enum {
    PPMSG_SESSION_INITIATE,
    PPMSG_SESSION_ACCEPT,
    PPMSG_SESSION_REJECT,
    PPMSG_SESSION_TERMINATE,
    PPMSG_SESSION_INFO,
    PPMSG_TRANSPORT_INFO,
    PPMSG_SESSION_ACK,
    PPMSG_TRANSPORT_ACK,
};

class PPMessage {
public:
    PPMessage() {
        argvs.clear();
    }
    ~PPMessage() {
    }
    unsigned int id;
    unsigned int type;
    std::string from;
    std::string to;
    std::vector<std::string> argvs; 
};

struct P2PInfo {
    std::string content_name;
    std::vector<cricket::Candidate >  candidates_;    
};

#endif
