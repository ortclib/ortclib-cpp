ortclib
=======

ORTC (Object RTC) iOS / Android C++ Library for WebRTC

This SDK library implementation of the ORTC specification that will enable mobile end points to talk to a WebRTC enabled browser.

The source code is available under a FreeBSD style license:
https://github.com/openpeer/ortclib/blob/master/license.txt

The code is under heavy development and written mostly in C++. Mobile wrappers for  Objective-C are currently underway, Java Android will follow.

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
pushd ortc-lib/libs/curl/
./build_ios.sh
popd
</code>
</pre>
<br />
3) From X-code, load:

- ortc-lib/projects/xcode/ortclib.xcworkspace (workspace)
- orct-lib/projects/xcode/ortclib/ortclib.xcodeproj (project)
- orct-lib/projects/xcode/ortclib_ios/ortclib_ios.xcodeproj (project)


For more information on ORTC, please visit:
http://ortc.org/
