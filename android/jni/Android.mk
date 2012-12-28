LOCAL_PATH:= $(call my-dir)
MY_LOCAL_PATH = $(LOCAL_PATH)

###########################################################
# building libexpat 
#
include $(CLEAR_VARS)
LOCAL_CFLAGS := -DHAVE_MEMMOVE=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1
LOCAL_MODULE    := libexpat
LOCAL_SRC_FILES := expat/xmlparse.c \
                   expat/xmlrole.c \
                   expat/xmltok.c
include $(BUILD_STATIC_LIBRARY)

###########################################################
# building openssl 
#
include $(LOCAL_PATH)/openssl/Crypto.mk
include $(LOCAL_PATH)/openssl/Ssl.mk


###########################################################
# building libjingle 
#
include $(CLEAR_VARS)
LOCAL_MODULE := libjingle
LOCAL_CPP_EXTENSION := .cc .cpp
LOCAL_CPPFLAGS := -O2 -g3 -Werror -Wall -Wno-switch -Wno-non-virtual-dtor -Wno-ctor-dtor-privacy -fno-rtti -fpic -fno-exceptions 
LOCAL_CPPFLAGS += -DLOGGING=1 -DFEATURE_ENABLE_SSL -DHAVE_OPENSSL_SSL_H=1 -DEXPAT_RELATIVE_PATH -DHASHNAMESPACE=__gnu_cxx  -DHASH_NAMESPACE=__gnu_cxx -DPOSIX -DDISABLE_DYNAMIC_CAST -D_REENTRANT -DLINUX -DDISABLE_EFFECTS=1 -DANDROID -DARCH_CPU_LITTLE_ENDIAN 
LOCAL_C_INCLUDES :=  $(MY_LOCAL_PATH)/expat $(MY_LOCAL_PATH)/openssl/include
include $(MY_LOCAL_PATH)/libjingle_build.mk

LOCAL_WHOLE_STATIC_LIBRARIES := libexpat\
                                libcrypto\
                                libssl

LOCAL_SHARED_LIBRARIES := libcutils\
                          libgnustl\
                          libdl 

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

###########################################################
# building application library 
#
include $(CLEAR_VARS)
LOCAL_MODULE := libapp
LOCAL_CPP_EXTENSION := .cc .cpp
LOCAL_CPPFLAGS := -O2 -g3 -Werror -Wall -Wno-switch -Wno-non-virtual-dtor -Wno-ctor-dtor-privacy -fno-rtti -fpic -fno-exceptions 
LOCAL_CPPFLAGS += -DLOGGING=1 -DFEATURE_ENABLE_SSL -DHAVE_OPENSSL_SSL_H=1 -DEXPAT_RELATIVE_PATH -DHASHNAMESPACE=__gnu_cxx  -DHASH_NAMESPACE=__gnu_cxx -DPOSIX -DDISABLE_DYNAMIC_CAST -D_REENTRANT -DLINUX -DDISABLE_EFFECTS=1 -DANDROID -DARCH_CPU_LITTLE_ENDIAN 
LOCAL_C_INCLUDES :=  $(MY_LOCAL_PATH)/expat $(MY_LOCAL_PATH)/openssl/include $(MY_LOCAL_PATH) 

include $(MY_LOCAL_PATH)/app_build.mk

#LOCAL_WHOLE_STATIC_LIBRARIES := libjingle

LOCAL_SHARED_LIBRARIES := libjingle\
                          libcutils\
                          libgnustl\
                          libdl 

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

