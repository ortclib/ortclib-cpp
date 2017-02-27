/*
 
 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#pragma once

#include <ortc/types.h>
#include <ortc/IMediaStreamTrack.h>

#include <list>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevices
  #pragma mark
  
  interaction IMediaDevicesTypes : public IMediaStreamTrackTypes
  {
    ZS_DECLARE_STRUCT_PTR(SupportedConstraints);
    ZS_DECLARE_STRUCT_PTR(Device);
    ZS_DECLARE_STRUCT_PTR(DeviceList);
    ZS_DECLARE_STRUCT_PTR(MediaStreamTrackList);

    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<MediaStreamTrackList>, PromiseWithMediaStreamTrackList);
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<DeviceList>, PromiseWithDeviceList);

    enum DeviceKinds
    {
      DeviceKind_First,

      DeviceKind_AudioInput = DeviceKind_First,
      DeviceKind_AudioOutput,
      DeviceKind_VideoInput,

      DeviceKind_Last = DeviceKind_VideoInput,
    };

    static const char *toString(DeviceKinds kind);
    static DeviceKinds toDeviceKind(const char *deviceKindStr);
    static Kinds toKind(DeviceKinds kind);

    static bool isAudio(DeviceKinds kind);
    static bool isVideo(DeviceKinds kind);

    struct SupportedConstraints
    {
      bool mWidth {false};
      bool mHeight {false};
      bool mAspectRatio {false};
      bool mFrameRate {false};
      bool mFacingMode {false};
      bool mVolume {false};
      bool mSampleRate {false};
      bool mSampleSize {false};
      bool mEchoCancellation {false};
      bool mLatency {false};
      bool mChannelCount {false};
      bool mDeviceID {false};
      bool mGroupID {false};

      SupportedConstraints() {}
      SupportedConstraints(const SupportedConstraints &op2) {(*this) = op2;}
      SupportedConstraints(ElementPtr elem);

      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDevices::Device
    #pragma mark

    struct Device {
      DeviceKinds mKind {DeviceKind_First};

      String mLabel;
      String mDeviceID;
      String mGroupID;

      SupportedConstraints mSupportedConstraints;

      Device() {}
      Device(const Device &op2) {(*this) = op2;}
      Device(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "device") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDevices::MediaStreamTrackList
    #pragma mark

    struct MediaStreamTrackList : public std::list<IMediaStreamTrackPtr>,
                                  public Any
    {
      static MediaStreamTrackListPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDevices::DeviceList
    #pragma mark

    struct DeviceList : public std::list<Device>,
                        public Any
    {
      static DeviceListPtr convert(AnyPtr any);

      DeviceList() {}
      DeviceList(const DeviceList &op2) {(*this) = op2;}
      DeviceList(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "devices") const;

      ElementPtr toDebug() const;
      String hash() const;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevices
  #pragma mark
  
  interaction IMediaDevices : public IMediaDevicesTypes
  {
    static ElementPtr toDebug();

    static SupportedConstraintsPtr getSupportedConstraints();

    static PromiseWithDeviceListPtr enumerateDevices();

    static PromiseWithMediaStreamTrackListPtr getUserMedia(const Constraints &constraints = Constraints());

    static IMediaDevicesSubscriptionPtr subscribe(IMediaDevicesDelegatePtr delegate);

    virtual ~IMediaDevices() {} // make polymorphic
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevicesDelegate
  #pragma mark

  interaction IMediaDevicesDelegate
  {
    virtual void onMediaDevicesChanged() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevicesSubscription
  #pragma mark

  interaction IMediaDevicesSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IMediaDevicesDelegate)
ZS_DECLARE_PROXY_METHOD_0(onMediaDevicesChanged)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaDevicesDelegate, ortc::IMediaDevicesSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaDevicesChanged)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
