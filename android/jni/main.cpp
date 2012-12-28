#include <jni.h>
#include <android/log.h>

#include "talk/base/basicdefs.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"
#include "prober.h"
#include "peer.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#define  JNIDEFINE(fname) Java_teaonly_iceprober_MainActivity_##fname
extern "C" {
    JNIEXPORT jint JNICALL JNIDEFINE(nativeMain)(JNIEnv* env, jclass clz, jobject obj, jstring server, jstring local, jstring remote);
}

static std::string convert_jstring(JNIEnv *env, const jstring &js) {
    static char outbuf[1024];
    std::string str;
 
    int len = env->GetStringLength(js);
    env->GetStringUTFRegion(js, 0, len, outbuf);

    str = outbuf;
    return str;
}

static jint get_native_fd(JNIEnv* env, jclass clz, jobject fdesc) {
  jclass clazz;
  jfieldID fid;

  /* get the fd from the FileDescriptor */
  if (!(clazz = env->GetObjectClass(fdesc)) ||
    !(fid = env->GetFieldID(clazz,"descriptor","I"))) return -1;

  /* return the descriptor */
  return env->GetIntField(fdesc,fid);
}

class SimpleConsole : public sigslot::has_slots<> {
public:
    SimpleConsole(int fd):fd_(fd) {
    }
    ~SimpleConsole() {
    }
    
    void OnPrintString(const std::string& msg) {
        send(fd_, msg.c_str(), msg.length(), 0);        
    }

    void OnExit() {
    }
private:
    int fd_;    
}; 


static IceProber* pProber;
static SimpleConsole* console;
static int event_fd;

JNIEXPORT jint JNICALL JNIDEFINE(nativeMain)(JNIEnv* env, jclass clz, jobject obj, jstring jserver, jstring jlocal, jstring jremote) {
    std::string server = convert_jstring(env, jserver);
    std::string local = convert_jstring(env, jlocal);
    std::string remote = convert_jstring(env, jremote); 
    event_fd = get_native_fd(env, clz, obj);

    //talk_base::LogMessage::LogToDebug(talk_base::LS_VERBOSE);
    talk_base::LogMessage::LogToDebug(talk_base::LS_ERROR);
    talk_base::LogMessage::LogTimestamps();
    talk_base::LogMessage::LogThreads();

    pProber = new IceProber("/sdcard/pplog.txt");
    console = new SimpleConsole(event_fd);

    pProber->SignalPrintString.connect( console, &SimpleConsole::OnPrintString);
    pProber->SignalExit.connect( console, &SimpleConsole::OnExit);

    pProber->Login(server, 1979, local, remote);
    
    return 0; 
}


