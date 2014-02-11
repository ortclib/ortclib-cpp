LOCAL_PATH := $(call my-dir)/../../
$(warning $(LOCAL_PATH))
$(warning $(ANDROIDNDK_PATH))
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
$(info $(LOCAL_PATH))
LOCAL_CFLAGS	:= -Wall \
-W \
-O2 \
-pipe \
-fPIC \
-frtti \
-fexceptions \
-D_ANDROID \
$(info $(LOCAL_PATH))

LOCAL_MODULE    := ortc_android

LOCAL_C_INCLUDES:= \
$(LOCAL_PATH) \
$(LOCAL_PATH)/libs/op-services-cpp \
$(LOCAL_PATH)/libs/build/android/boost/include/boost-1_53 \
$(LOCAL_PATH)/libs/zsLib \
$(LOCAL_PATH)/libs \
$(LOCAL_PATH)/libs/build/android/cryptopp/include \
$(LOCAL_PATH)/libs/webrtc/webrtc/voice_engine/include \
$(LOCAL_PATH)/libs/webrtc \
$(LOCAL_PATH)/libs/webrtc/webrtc \
$(LOCAL_PATH)/libs/webrtc/webrtc/video_engine/include \
$(LOCAL_PATH)/libs/webrtc/webrtc/modules/video_capture/include \
$(ANDROIDNDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.7/include \
$(ANDROIDNDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi/include \
$(ANDROIDNDK_PATH)/platforms/android-9/arch-arm/usr/include \

LOCAL_SRC_FILES := ortc/cpp/ortc.cpp \
ortc/cpp/ortc_Factory.cpp \
ortc/cpp/ortc_Helper.cpp \
ortc/cpp/ortc_ICETransport.cpp \
ortc/cpp/ortc_MediaEngine.cpp \
ortc/cpp/ortc_MediaManager.cpp \
ortc/cpp/ortc_MediaStream.cpp \
ortc/cpp/ortc_MediaStreamTrack.cpp \
ortc/cpp/ortc_ORTC.cpp \
ortc/cpp/ortc_RTCConnection.cpp \
ortc/cpp/ortc_RTCDataChannel.cpp \
ortc/cpp/ortc_RTCDTMFTrack.cpp \
ortc/cpp/ortc_RTCSocket.cpp \
ortc/cpp/ortc_RTCStream.cpp \
ortc/cpp/ortc_RTCTrack.cpp \

#$(info $(LOCAL_PATH))

#LOCAL_LDLIBS := -L$(LOCAL_PATH)/libs/build/android/boost/lib -L$(LOCAL_PATH)/libs/build/android/cryptopp -L$(LOCAL_PATH)/libs/build/android/curl -L$(LOCAL_PATH)/libs/build/#android/openssl -L$(LOCAL_PATH)/libs/build/android/op-services-cpp -L$(LOCAL_PATH)/libs/build/android/udns -L$(LOCAL_PATH)/libs/build/android/zsLib

#$(info $(LOCAL_PATH))
#LOCAL_LDLIBS += -lboost_atomic-gcc-mt-1_53 -lboost_chrono-gcc-mt-1_53 -lboost_date_time-gcc-mt-1_53 -lboost_filesystem-gcc-mt-1_53 -lboost_graph-gcc-mt-1_53 -lboost_random-#gcc-mt-1_53 -lboost_regex-gcc-mt-1_53 -lboost_signals-gcc-mt-1_53 -lboost_system-gcc-mt-1_53 -lboost_thread-gcc-mt-1_53 -lcrypto -lcryptopp -lcurl -lhfservices_android -lssl -#ludns_android -lzslib_android

LOCAL_LDLIBS += -lgnustl_static -lsupc++  -L$(ANDROIDNDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi

$(info $(LOCAL_PATH))		
$(info going to build static)
$(info $(ANDROIDNDK_PATH))	
include $(BUILD_STATIC_LIBRARY)

