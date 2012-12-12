TALK_INC_DIR = ./libjingle-0.6.14
EXPAT_INC_DIR = /usr/include
SSL_INC_DIR =  /usr/include/openssl

AR = ar
CC = g++
CFLAGS = -Wall -fno-rtti -fno-exceptions -fvisibility=hidden -ffunction-sections -fdata-sections -gdwarf-2 -g3 -O0 -DHAMMER_TIME=1 -DLOGGING=1 -DFEATURE_ENABLE_SSL -DHASHNAMESPACE=__gnu_cxx -DPOSIX -DDISABLE_DYNAMIC_CAST -DHAVE_OPENSSL_SSL_H=1 -D_REENTRANT -DPOSOIX -D_DEBUG -DEXPAT_RELATIVE_PATH -DLINUX -I$(TALK_INC_DIR) -I$(EXPAT_INC_DIR) -I$(SSL_INC_DIR)
LDFLAGS = -lexpat -lssl -lcrypto -lpthread -lrt

LIB_SRCS =
include libjingle_build.mk
LIB_OBJS = ${LIB_SRCS:.cc=.o}

APP_SRCS =
include app_build.mk
APP_OBJS = ${APP_SRCS:.cpp=.o}

LIB = libjingle.a
TARGET = ProbICE

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(LIB_OBJS)
	$(AR) -cr $@ $(LIB_OBJS)

$(TARGET): $(LIB) $(APP_OBJS)
	$(CC) -o $@ $(APP_OBJS) $(LIB) $(LDFLAGS)

.PHONY: clean all
all: $(TARGET)
clean:
	rm -f $(APP_OBJS)
	rm -f $(TARGET)
	rm -f $(LIB_OBJS)
	rm -f $(LIB)
