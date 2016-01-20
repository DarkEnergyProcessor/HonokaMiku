LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_ARM_MODE := arm
LOCAL_MODULE := HonokaMiku
LOCAL_SRC_FILES := md5.c CN_Decrypter.cc EN_Decrypter.cc HonokaMiku.cc JP_Decrypter.cc KR_Decrypter.cc TW_Decrypter.cc
LOCAL_CPPFLAGS := -fexceptions
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib

include $(BUILD_EXECUTABLE)