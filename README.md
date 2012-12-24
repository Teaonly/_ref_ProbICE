ProbICE
=======

A tool to probe the ICE p2p progress based on Google's libjingle. 
Creating a user friendly interface to show the process of ICE (RFC5245) peer-to-peer talking and probe 
the issue of blocking p2p.

## how to build ##
 * linux
<pre>    
    #ln -s ./linux/* . 
    #make all
</pre>

 * mac 
<pre>
    #ln -s ./mac/* . 
    #make all
</pre>

 * Windows
 Open the solution file for Visual Studio 2008 in win floder. 


## how to run ##

* start the session server
  There is simple session based chat server developed by Node.js
<pre>
    #cd dialog_server
    #node inde.js
</pre>

* run in two peers
 In local
 <pre>
    #ProbIce <session_server> local_name remote_name
 </pre>
 In remote
 <Pre>
    #ProbIce <session_server> local_name remote_name
 </Pre>
The ICE progress will auto started, the total ICE internal state will report to STDOUT, you can save the STDOUT to a txt file.

