package teaonly.iceprober;
import teaonly.iceprober.*;
import teaonly.task.*;

import java.io.*; 
import java.net.*;
import java.util.*;

import android.net.*;
import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.View;
import android.webkit.WebView; 

public class MainActivity extends Activity
{
    private static final String TAG = "TEAONLY"; 
    private WebView webview;
    private TeaServer teaServer;
    private NativeAgentTask nativeAgentTask;
   
    private LocalSocket jSock, nSock; 
    private LocalServerSocket lss;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        setContentView(R.layout.main);
        
        // setup Tea server
	    try {
		    teaServer = new TeaServer(7910, "/android_asset");
	    } catch (IOException e) {
            //Log.d(TAG, "Error building internal http server!");     
            e.printStackTrace();
		    teaServer = null;
	    }
	    if ( teaServer != null) {
            teaServer.registerCGI("/cgi/StartProber", doStartProber);
        }
        
        // setup event sockets and backgroundd listener
        initEventSocket( "IceProber" );
        nativeAgentTask = new NativeAgentTask();
        nativeAgentTask.setListener(nativeAgentListener);
        nativeAgentTask.execute();                 
 
	    //setup webView
	    webview = (WebView)findViewById(R.id.webview);
        webview.setOnLongClickListener(new View.OnLongClickListener() {
                public boolean onLongClick(View v) {
                return true;
                }
        });
	    webview.getSettings().setJavaScriptEnabled(true);
	    webview.setVerticalScrollBarEnabled(false);
	    webview.addJavascriptInterface(this, "App");
	    webview.loadUrl("file:///android_asset/index.html");
       
        //Load jni libraries
        System.loadLibrary("jingle");
        System.loadLibrary("app");
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
         super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onPause() {
        super.onPause();
        finish(); 
    }
    
    // ************************************************************************
    // Definement of internal help functions
    // 
    // ************************************************************************
    private void initEventSocket(String localAddress) {
       try {
            jSock = new LocalSocket();
            
            lss = new LocalServerSocket(localAddress);
            jSock.connect(new LocalSocketAddress(localAddress));
            jSock.setReceiveBufferSize(1000);
            jSock.setSendBufferSize(1000);

            nSock = lss.accept();
            nSock.setReceiveBufferSize(1000);
            nSock.setSendBufferSize(1000);
           
        } catch ( IOException ex) {
            ex.printStackTrace();
        }
    }
    

    // ************************************************************************
    // Definement of immediate function called from JS side
    // 
    // ************************************************************************
    public void DebugPrint(String msg) {
        Log.d(TAG, msg);
    } 

    public void StartNative(String server, String local, String remote) {
        nativeMain(this, server, local, remote);    
    }

    // ************************************************************************
    // Definement of CGI function from GUI (broswer side), run in single thread
    // 
    // ************************************************************************
    private TeaServer.CommonGatewayInterface doStartProber = new TeaServer.CommonGatewayInterface () {
        //@Override
        public String run(Properties parms) {
            return "OK";
        }

        //@Override 
        public InputStream streaming(Properties parms) {
            return null;
        }
    };
    // end of CGI function  

    // ************************************************************************
    // Definement of handler of native message over LocalSocket 
    // 
    // ************************************************************************
    private TaskListener nativeAgentListener = new TaskAdapter() { 
        @Override
        public String getName() {
            return "NativeAgent";
        }   

        @Override
        public void onProgressUpdate(GenericTask task, Object param) {
            String xmlMessage = (String) param;
            DebugPrint(xmlMessage);
        }   
    };
    
    private class NativeAgentTask extends GenericTask {
        @Override 
        protected TaskResult _doInBackground(TaskParams... params) {
            byte[] receiveData = new byte[1024*16]; 
            int recvOffset = 0;

            while(true) {
                try {
                    int ret = jSock.getInputStream().read( receiveData, recvOffset, 1);
                    if ( ret < 0)
                        break;
                    if ( ret == 0)
                        continue;
                    if ( receiveData[recvOffset] == (byte)(0) ) {
                        String xmlBuffer = new String(receiveData, 0, recvOffset);      // not recvOffset + 1, we don't need '\0'
                        publishProgress(xmlBuffer);
                        recvOffset = 0;
                    } else {
                        recvOffset++;
                    }
                } catch (IOException ex) {
                    ex.printStackTrace();
                    break;
                }
            }
           
            return TaskResult.OK;            
        }
    }; 

    // ************************************************************************
    // Definement of native functions
    // 
    // ************************************************************************
    static private native int nativeMain(MainActivity obj, String server, String local, String remote);
    
}
