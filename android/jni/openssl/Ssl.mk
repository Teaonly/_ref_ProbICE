local_c_includes := \
	openssl \
	openssl/include \
	openssl/crypto

local_src_files:= \
	openssl/ssl/bio_ssl.c \
	openssl/ssl/d1_both.c \
	openssl/ssl/d1_enc.c \
	openssl/ssl/d1_lib.c \
	openssl/ssl/d1_pkt.c \
	openssl/ssl/d1_srtp.c \
	openssl/ssl/d1_clnt.c \
    openssl/ssl/d1_meth.c \
    openssl/ssl/d1_srvr.c \
    openssl/ssl/kssl.c \
	openssl/ssl/s23_clnt.c \
	openssl/ssl/s23_lib.c \
	openssl/ssl/s23_meth.c \
	openssl/ssl/s23_pkt.c \
	openssl/ssl/s23_srvr.c \
	openssl/ssl/s2_clnt.c \
	openssl/ssl/s2_enc.c \
	openssl/ssl/s2_lib.c \
	openssl/ssl/s2_meth.c \
	openssl/ssl/s2_pkt.c \
	openssl/ssl/s2_srvr.c \
	openssl/ssl/s3_both.c \
	openssl/ssl/s3_clnt.c \
	openssl/ssl/s3_enc.c \
	openssl/ssl/s3_lib.c \
	openssl/ssl/s3_meth.c \
	openssl/ssl/s3_pkt.c \
	openssl/ssl/s3_srvr.c \
	openssl/ssl/ssl_algs.c \
	openssl/ssl/ssl_asn1.c \
	openssl/ssl/ssl_cert.c \
	openssl/ssl/ssl_ciph.c \
	openssl/ssl/ssl_err.c \
	openssl/ssl/ssl_err2.c \
	openssl/ssl/ssl_lib.c \
	openssl/ssl/ssl_rsa.c \
	openssl/ssl/ssl_sess.c \
	openssl/ssl/ssl_stat.c \
	openssl/ssl/ssl_txt.c \
	openssl/ssl/t1_clnt.c \
	openssl/ssl/t1_enc.c \
	openssl/ssl/t1_lib.c \
	openssl/ssl/t1_meth.c \
	openssl/ssl/t1_reneg.c \
	openssl/ssl/t1_srvr.c \
	openssl/ssl/tls_srp.c

local_c_includes += $(log_c_includes)

local_additional_dependencies := $(LOCAL_PATH)/openssl/android-config.mk $(LOCAL_PATH)/openssl/Ssl.mk

#######################################
# target shared library
include $(CLEAR_VARS)
include $(LOCAL_PATH)/openssl/android-config.mk

ifeq ($(TARGET_ARCH),arm)
LOCAL_NDK_VERSION := 5
LOCAL_SDK_VERSION := 9
endif
LOCAL_SRC_FILES += $(local_src_files)
LOCAL_C_INCLUDES += $(local_c_includes)
LOCAL_SHARED_LIBRARIES += libcrypto $(log_shared_libraries)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libssl
LOCAL_ADDITIONAL_DEPENDENCIES := $(local_additional_dependencies)
include $(BUILD_STATIC_LIBRARY)
