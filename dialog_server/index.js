// Global function
Array.prototype.remove = function(e) {
  for (var i = 0; i < this.length; i++) {
    if (e == this[i]) { return this.splice(i, 1); }
  }
};


// PeerClass define
function PeerClass(sock) {
    this.pid = null;
    this.xmlWord = "";
    this.sock = sock;
}
var peerObjects = [];



// PeerServer
var PeerServer = {};
PeerServer.onAccept = function(sock) {
    var peer = new PeerClass(sock);    
    peerObjects.push(peer);

    sock.setTimeout(0);
    sock.setEncoding("utf8");
    return peer;
};

PeerServer.onConnected = function(peer) {
    peer.sock.write("Hi\r\n");
    return;
}
PeerServer.onClose = function(peer, hasError) {
    return;
}
PeerServer.onData = function(peer, d) {

    for(var i = 0; i < d.length; i++) {
        peer.xmlWord += d.substr(i,1);

        if(d.charAt(i) == '>') {
            peer.sock.write("You say:" + peer.xmlWord);
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

}).listen(1080);        


