var page_index = {};


page_index.start = function() {
    var server = $("#server").val();
    var local = $("#local").val();
    var remote = $("#remote").val();

    App.StartNative(server, local, remote);
    
    $("#server").textinput('disable'); 
    $("#local").textinput('disable');       
    $("#remote").textinput('disable');      
    
    $("#start").button('disable').button('refresh');
}

page_index.onPrintString = function(msg) {
    //$("#message").html(msg);
    var newMessage = $("#message").val() + msg + "\r\n";
    $("#message").val(newMessage);
}

$("#page_index").live("pageinit",function() {
    $("#message").prop("disabled", true); 
    

    $("#start").bind("click", page_index.start);
});


