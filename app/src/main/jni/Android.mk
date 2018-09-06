LOCAL_PATH := $(call my-dir)

LIB1 := pj-arm-unknown-linux-androideabi
LIB2 := pjlib-util-arm-unknown-linux-androideabi
LIB3 := pjnath-arm-unknown-linux-androideabi
LIB4 := pjsip-arm-unknown-linux-androideabi
LIB5 := pjsip-ua-arm-unknown-linux-androideabi

#自己的编译模块
include $(CLEAR_VARS)
LOCAL_MODULE    := MySip
LOCAL_SRC_FILES := jnisip.c main.c


LOCAL_LDLIBS    +=  -L$(SYSROOT)/lib -llog -lm
LOCAL_LDFLAGS   += -L $(LOCAL_PATH)/lib -l$(LIB5) -l$(LIB4) -l$(LIB1) -l$(LIB2) -l$(LIB3)
LOCAL_CFLAGS    := -g
#这里引入第三方编译模块
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
include $(BUILD_SHARED_LIBRARY)


