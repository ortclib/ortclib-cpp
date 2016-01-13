ortclib
=======

ORTC (Object RTC) iOS / Android C++ Library for WebRTC

Introduction: http://www.slideshare.net/ErikLagerway/ortc-lib-introduction

This SDK library implementation of the ORTC specification that will enable mobile end points to talk to a WebRTC enabled browser.

The source code is available under a FreeBSD style license:
https://github.com/openpeer/ortclib/blob/master/license.txt

Hookflash provides professional services to aid in the integration or expansion of this Library in your project, contact Hookflash for more information: sales@hookflash.com
========

This code is under heavy development and written mostly in C++. Mobile wrappers for  Objective-C are currently underway, Java Android will follow.

From your terminal, please clone the "opios" git repository:
git clone --recursive https://github.com/openpeer/ortc-lib.git

This repository will yield the ORTC Lib and dependency libraries.

Directory structure:

- ortc-lib/                        		contains the project files for building the ORTC Lib framework
- ortc-lib/ortc/          				contains the ORTC Lib header files
- ortc-lib/ortc/cpp/    				contains the C++ files
- ortc-lib/ortc/internal/  				contains the internal interface implementation

How to Build:

1) Build boost, from your terminal:
<br />
<pre>
<code>
pushd ortc-lib/libs/boost/
./boost.sh
popd
</code>
</pre>
<br />
2) Build curl, from your terminal:
<br />
<pre>
<code>
pushd ortc-lib/libs/curl-build-scripts/
./build_curl
popd
</code>
</pre>
Note :
- If building curl on your system fails for some architectures, you may need to explicitly specify the correct versions of OS X SDK and iOS SDK, to match the actual SDK versions available with the installed version of XCode. Please, check the details on command-line parameters of build_curl (you can get them with "build_curl <span>-</span><span>-</span>help").

<br />
3) From X-code, load:

- ortc-lib/projects/xcode/ortclib.xcworkspace (workspace)
- orct-lib/projects/xcode/ortclib/ortclib.xcodeproj (project)
- orct-lib/projects/xcode/ortclib_ios/ortclib_ios.xcodeproj (project)

ORTC LIB ANDROID BUILD :
------------------------
1. ortc-lib and its dependendent submodules can be built using the script "buildall_android.sh".
commands from parent directort "ortc-lib" is:
<pre>
<code>
cd ./projects/android
./buildall_android.sh
</code>
</pre>

Notes :
- NDK absolute path for "android-ndk-r8e" is required for build.
- After successful build, submodule built libs can be found at : ./libs/build/android directory
- ortc-lib library and ortclibtest shared lib can be found at : ./ortc/build/android and ./ortc/build/android-test directories resp.

2. For building individual modules use module script. For example boost :
./libs/boost/projects/build_boost_android.sh
<pre>
<code>
cd ./libs/boost/projects
./build_boost_android.sh
</code>
</pre>

Note :
- NDK absolute path for "android-ndk-r8e" is required for build.

known Issues in android build :
- Linking webrtc libraries while building ortclib test shared libary.

For more information on ORTC, please visit:
http://ortc.org/
