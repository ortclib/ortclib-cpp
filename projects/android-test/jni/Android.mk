LOCAL_PATH := $(call my-dir)/../..
WEBRTC_LIBS_PATH := ../libs/build/android/webrtc
BOOST_LIBS_PATH := ../libs/build/android/boost/lib

#zLib shared library
include $(CLEAR_VARS)
ZLIB_LIB_PATH := ./android-test/jni
LOCAL_MODULE := z_shared
LOCAL_SRC_FILES := \
    $(ZLIB_LIB_PATH)/libz_shared.so
include $(PREBUILT_SHARED_LIBRARY)

#openssl:begin
include $(CLEAR_VARS)
OPENSSL_LIB_PATH := ../libs/build/android/openssl
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := \
    $(OPENSSL_LIB_PATH)/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
OPENSSL_LIB_PATH := ../libs/build/android/openssl
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := \
    $(OPENSSL_LIB_PATH)/libssl.a
include $(PREBUILT_STATIC_LIBRARY)
#openssl:end

#curl lib
include $(CLEAR_VARS)
CURL_LIB_PATH := ../libs/build/android/curl
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := \
    $(CURL_LIB_PATH)/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

#webrtc libs
include $(CLEAR_VARS)
LOCAL_MODULE := libaudio_coding_module
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libaudio_coding_module.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudio_conference_mixer
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libaudio_conference_mixer.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudio_device
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libaudio_device.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudio_processing
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libaudio_processing.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudio_processing_neon
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libaudio_processing_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libbitrate_controller
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libbitrate_controller.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libchannel_transport
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libchannel_transport.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libCNG
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libCNG.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcommon_audio
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libcommon_audio.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcommon_video
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libcommon_video.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libG711
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libG711.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libG722
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libG722.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libiLBC
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libiLBC.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libisac_neon
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libisac_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libiSAC
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libiSAC.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libiSACFix
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libiSACFix.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libjpeg_turbo
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libjpeg_turbo.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libmedia_file
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libmedia_file.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libNetEq
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libNetEq.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libNetEq4
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libNetEq4.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libopus
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpaced_sender
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libpaced_sender.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libPCM16B
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libPCM16B.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libremote_bitrate_estimator
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libremote_bitrate_estimator.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := librtp_rtcp
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/librtp_rtcp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libsystem_wrappers
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libsystem_wrappers.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvideo_capture_module
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvideo_capture_module.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvideo_coding_utility
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvideo_coding_utility.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvideo_engine_core
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvideo_engine_core.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvideo_processing
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvideo_processing.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvideo_render_module
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvideo_render_module.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvoice_engine_core
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvoice_engine_core.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvpx
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libvpx.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libwebrtc_i420
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libwebrtc_i420.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libwebrtc_opus
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libwebrtc_opus.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libwebrtc_utility
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libwebrtc_utility.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libwebrtc_video_coding
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libwebrtc_video_coding.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libwebrtc_vp8
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libwebrtc_vp8.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libyuv
LOCAL_SRC_FILES := \
    $(WEBRTC_LIBS_PATH)/libyuv.a
include $(PREBUILT_STATIC_LIBRARY)

#hfsservices
include $(CLEAR_VARS)
SERVICES_LIBS_PATH := ../libs/build/android/op-services-cpp
LOCAL_MODULE := libhfservices_android
LOCAL_SRC_FILES := \
    $(SERVICES_LIBS_PATH)/libhfservices_android.a
include $(PREBUILT_STATIC_LIBRARY)

#cryptopp
include $(CLEAR_VARS)
CRYPTOPP_LIBS_PATH := ../libs/build/android/cryptopp
LOCAL_MODULE := libcryptopp
LOCAL_SRC_FILES := \
    $(CRYPTOPP_LIBS_PATH)/libcryptopp.a
include $(PREBUILT_STATIC_LIBRARY)

#udns
include $(CLEAR_VARS)
UDNS_LIBS_PATH := ../libs/build/android/udns
LOCAL_MODULE := libudns_android
LOCAL_SRC_FILES := \
    $(UDNS_LIBS_PATH)/libudns_android.a
include $(PREBUILT_STATIC_LIBRARY)

#ZsLib
include $(CLEAR_VARS)
ZSLIB_LIBS_PATH := ../libs/build/android/zsLib
LOCAL_MODULE := libzslib_android
LOCAL_SRC_FILES := \
    $(ZSLIB_LIBS_PATH)/libzslib_android.a
include $(PREBUILT_STATIC_LIBRARY)

#Boost libs
include $(CLEAR_VARS)
LOCAL_MODULE := libboost_atomic-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_atomic-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_chrono-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_chrono-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_date_time-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_date_time-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_filesystem-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_filesystem-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_graph-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_graph-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_iostreams-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_iostreams-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_program_options-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_program_options-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_random-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_random-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_regex-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_regex-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_signals-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_signals-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_system-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_system-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_thread-gcc-mt-1_53
LOCAL_SRC_FILES := \
    $(BOOST_LIBS_PATH)/libboost_thread-gcc-mt-1_53.a
include $(PREBUILT_STATIC_LIBRARY)

#ortc_android lib
include $(CLEAR_VARS)
ORTC_LIBS_PATH := ../ortc/build/android
LOCAL_MODULE := libortc_android
LOCAL_SRC_FILES := \
    $(ORTC_LIBS_PATH)/libortc_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm

LOCAL_CFLAGS	:= -Wall \
-W \
-O2 \
-pipe \
-fPIC \
-frtti \
-fexceptions \
-D_ANDROID \

LOCAL_MODULE    := ortcnativetest

LOCAL_C_INCLUDES:= \
$(LOCAL_PATH)/.. \
$(LOCAL_PATH)/../ortc/ \
$(LOCAL_PATH)/../ortc/test \
$(LOCAL_PATH)/../libs/op-services-cpp \
$(LOCAL_PATH)/../libs/build/android/curl/include \
$(LOCAL_PATH)/../libs/build/android/boost/include/boost-1_53 \
$(LOCAL_PATH)/../libs/zsLib \
$(LOCAL_PATH)/../libs \
$(LOCAL_PATH)/../libs/build/android/cryptopp/include \
$(LOCAL_PATH)/../libs/webrtc/webrtc/voice_engine/include \
$(LOCAL_PATH)/../libs/webrtc \
$(LOCAL_PATH)/../libs/webrtc/webrtc \
$(LOCAL_PATH)/../libs/webrtc/webrtc/video_engine/include \
$(LOCAL_PATH)/../libs/webrtc/webrtc/modules/video_capture/include \
$(ANDROIDNDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/include \
$(ANDROIDNDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi/include \
$(ANDROIDNDK_PATH)/platforms/android-9/arch-arm/usr/include \

LOCAL_SRC_FILES := \
		./android-test/jni/com_tmz_cc_MainActivity.cpp \
		../ortc/test/TestMediaEngine.cpp \


LOCAL_LDLIBS += $(ANDROIDNDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi/libgnustl_static.a

LOCAL_STATIC_LIBRARIES := \
libaudio_coding_module \
libaudio_conference_mixer \
libaudio_device \
libaudio_processing \
libaudio_processing_neon \
libbitrate_controller \
libchannel_transport \
libCNG \
libcommon_audio \
libcommon_video \
libG711 \
libG722 \
libiLBC \
libisac_neon \
libiSAC \
libiSACFix \
libjpeg_turbo \
libmedia_file \
libNetEq \
libNetEq4 \
libopus \
libpaced_sender \
libPCM16B \
libremote_bitrate_estimator \
librtp_rtcp \
libsystem_wrappers \
libvideo_capture_module \
libvideo_coding_utility \
libvideo_engine_core \
libvideo_processing \
libvideo_render_module \
libvoice_engine_core \
libvpx \
libwebrtc_i420 \
libwebrtc_opus \
libwebrtc_utility \
libwebrtc_video_coding \
libwebrtc_vp8 \
libyuv \
libortc_android \
libhfservices_android \
libcryptopp \
libudns_android \
libzslib_android \
libboost_atomic-gcc-mt-1_53 \
libboost_chrono-gcc-mt-1_53 \
libboost_date_time-gcc-mt-1_53 \
libboost_filesystem-gcc-mt-1_53 \
libboost_graph-gcc-mt-1_53 \
libboost_iostreams-gcc-mt-1_53 \
libboost_program_options-gcc-mt-1_53 \
libboost_random-gcc-mt-1_53 \
libboost_regex-gcc-mt-1_53 \
libboost_signals-gcc-mt-1_53 \
libboost_system-gcc-mt-1_53 \
libboost_thread-gcc-mt-1_53 \
libcurl \
libssl \
libcrypto \

LOCAL_SHARED_LIBRARIES := \
z_shared \

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_EXECUTABLE)

