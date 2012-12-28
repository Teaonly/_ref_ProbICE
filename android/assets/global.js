/*********************************************************
 * Global var define
 *********************************************************/
var config = {};
config.ajaxURL = "http://127.0.0.1:7910";

/*********************************************************
 * Top level event functions:
 *      called from Java side   
 *********************************************************/
function onBackPressed() {

}

function onPrintString(msg) {
    page_index.onPrintString(msg);    
}

/*********************************************************
 * Top level ajax functions:
 *      1. Java run in single thread.
 *      2. The Java handler can be blocked.
 *      3. Some none block or none in new thread should 
 *         Java via Helper object. 
 *********************************************************/

/*
function doConnect(ip, port) {
     $.ajax({
        type: "GET",
        url:  config.ajaxURL + "/cgi/connect",
        data: "ip=" + ip + "&port=" + port,
        cache: false,
    });
}
*/

