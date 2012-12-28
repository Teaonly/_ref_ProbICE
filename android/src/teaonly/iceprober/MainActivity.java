package teaonly.iceprober;
import teaonly.iceprober.*;

import java.io.*; 
import java.net.*;
import java.util.*;

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
    // Definement of immediate function called from JS side
    // 
    // ************************************************************************
    public void DebugPrint(String msg) {
        Log.d(TAG, msg);
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

}
