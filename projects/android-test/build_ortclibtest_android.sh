#!/bin/sh
echo "Please enter your android ndk path:"
echo "For example:/home/astro/android-ndk-r8e"
read Input
echo "You entered:$Input"

echo "----------------- Exporting the android-ndk path ----------------"

#Set path
export PATH=$PATH:$Input:$Input/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin

#create install directories
mkdir -p ./../../ortc/build/android-test

#ortc-lib module build
echo "------------------- Building ortc-lib for ANDROID platform ---------------"
pushd `pwd`

rm -rf ./obj

export ANDROIDNDK_PATH=$Input
export NDK_PROJECT_PATH=`pwd`
ndk-build APP_PLATFORM=android-9
popd

echo "-------- Installing ortc-lib -----"
cp -r ./obj/local/armeabi/lib* ./../../ortc/build/android-test/

#clean
rm -rf ./obj
