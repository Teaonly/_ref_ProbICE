// Global function
Array.prototype.remove = function(e) {
  for (var i = 0; i < this.length; i++) {
    if (e == this[i]) { return this.splice(i, 1); }
  }
};


// PeerClass
function PeerClass(sock) {
    this.pid = null;
    this.xmlWord = "";
    this.sid = sock.remoteAddress + sock.remotePort;
    this.sock = sock;
}
var peerObjects = {};


// PeerServer
var PeerServer = {};

PeerServer.processDialog = function(peer) {
    var wordBegin = peer.xmlWord.indexOf("<");
    var wordEnd = peer.xmlWord.indexOf(">");
    var sentence = peer.xmlWord.substring(wordBegin+1, wordEnd);
    
    var words = sentence.split(":");
    if ( words[0] == "login") {
        peer.pid = words[1];

        // kickoff the new one with same id
        if ( peerObjects.hasOwnProperty(peer.pid) ) {
            peer.sock.end();
            return false;
        }

        // send the online list 
        for (var p in peerObjects) {
            var pid = peerObjects[p].pid;
            var xml = "<online:" + pid + ">";
            peer.sock.write(xml);
        }     
       
        //tell to all the others 
        var xml = "<login:" + peer.pid + ">";
        for(var p in peerObjects) {
            peerObjects[p].sock.write(xml);
        } 

        //added to hash
        peerObjects[peer.pid] = peer;         
        console.log("New user " + peer.pid);

    } else if ( words[0] == "send") {
        if ( peer.pid == null)                  //login first
            return false;
        
        var remote = words[1];

        if ( peerObjects.hasOwnProperty(remote) ) {
            var remotePeer = peerObjects[remote];
            var sndMessage = "<message:" + peer.pid;
            for ( var i = 2; i < words.length; i++) {
                //remotePeer.sock.write("<message:" + peer.pid + ":" + content + ">");
                sndMessage = sndMessage + ":" + words[i];
            }
            sndMessage = sndMessage + ">"
            remotePeer.sock.write(sndMessage);
        }
    }   

    return true;
}

PeerServer.onAccept = function(sock) {
    var peer = new PeerClass(sock);    

    sock.setTimeout(0);
    sock.setEncoding("utf8");
    return peer;
};

PeerServer.onConnected = function(peer) {
}

PeerServer.onClose = function(peer, hasError) {
    if ( peerObjects.hasOwnProperty(peer.pid) ) {
        
        // We delete *REAL* user only
        if ( peer.sid != peerObjects[peer.pid].sid ) {
            return;
        }

        console.log("Delete user " + peer.pid);
        // remove from hash
        delete peerObjects[peer.pid];
        var xml = "<offline:" + peer.pid + ">";

        // tell the others
        for (var p in peerObjects) {
            peerObjects[p].sock.write(xml);
        }     
    }
}

PeerServer.onData = function(peer, d) {
    for(var i = 0; i < d.length; i++) {
        peer.xmlWord += d.substr(i,1);

        if(d.charAt(i) == '>') {
            PeerServer.processDialog(peer);
            peer.xmlWord = "";
        }
    };
    return; 
}

// create the server
var net = require("net");
net.createServer(function(sock) {
    var peer = PeerServer.onAccept(sock);

    sock.addListener("connect", function(){
        PeerServer.onConnected(peer);
    });

    sock.addListener("close", function(){
        PeerServer.onClose(peer);
    });

    sock.addListener("data", function(d){
        PeerServer.onData(peer, d);
    });

}).listen(1979);        


