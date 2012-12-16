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
    PPACTION_SESSION_INITIATE,
    PPACTION_SESSION_ACCEPT,
    PPACTION_SESSION_REJECT,
    PPACTION_SESSION_TERMINATE,
    PPACTION_SESSION_INFO,
    PPACTION_TRANSPORT_INFO,
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

struct P2PInfo{
    std::string content_name;
    std::vector<cricket::Candidate >  candidates_;    
};

#endif
