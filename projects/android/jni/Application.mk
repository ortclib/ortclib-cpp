# The ARMv7 is significanly faster due to the use of the hardware FPU
#APP_ABI := armeabi-v7a
#APP_PLATFORM := android-8
#APP_MODULES := zslib_android
#APP_BUILD_SCRIPT := $(NDK_PROJECT_PATH)/Android.mk
#$(warning $(NDK_PROJECT_PATH))
#APP_STL:=stlport_static
NDK_TOOLCHAIN_VERSION=4.7
APP_PROJECT_PATH := $(shell pwd)
APP_BUILD_SCRIPT := $(APP_PROJECT_PATH)/Android.mk
$(warning $(APP_PROJECT_PATH))
#APP_STL:=stlport_static
APP_MODULES := ortc_android

