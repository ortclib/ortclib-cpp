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
  //
  // IMediaDevices
  //
  
  interaction IMediaDevicesTypes : public IMediaStreamTrackTypes
  {
    ZS_DECLARE_STRUCT_PTR(SupportedConstraints);
    ZS_DECLARE_STRUCT_PTR(Device);
    ZS_DECLARE_STRUCT_PTR(DeviceList);
    ZS_DECLARE_STRUCT_PTR(MediaStreamTrackList);

    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Settings, Settings);

    struct SettingsList : public std::list<Settings>,
                          public Any
    {
    };

    ZS_DECLARE_PTR(SettingsList);

    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<MediaStreamTrackList>, PromiseWithMediaStreamTrackList);
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<DeviceList>, PromiseWithDeviceList);
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<SettingsList>, PromiseWithSettingsList);

    enum DeviceKinds
    {
      DeviceKind_First,

      DeviceKind_AudioInput = DeviceKind_First,
      DeviceKind_AudioOutput,
      DeviceKind_VideoInput,

      DeviceKind_Last = DeviceKind_VideoInput,
    };

    static const char *toString(DeviceKinds kind) noexcept;
    static DeviceKinds toDeviceKind(const char *deviceKindStr) noexcept(false); // throws InvalidParameters
    static Kinds toKind(DeviceKinds kind) noexcept;

    static bool isAudio(DeviceKinds kind) noexcept;
    static bool isVideo(DeviceKinds kind) noexcept;

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

      SupportedConstraints() noexcept {}
      SupportedConstraints(const SupportedConstraints &op2) noexcept {(*this) = op2;}
      SupportedConstraints(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaDevices::Device
    //

    struct Device
    {
       DeviceKinds mKind {DeviceKind_First};

      String mLabel;
      String mDeviceID;
      String mGroupID;

      SupportedConstraints mSupportedConstraints;

      Device() noexcept {}
      Device(const Device &op2) noexcept {(*this) = op2;}
      Device(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "device") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaDevices::MediaStreamTrackList
    //

    struct MediaStreamTrackList : public std::list<IMediaStreamTrackPtr>,
                                  public Any
    {
      static MediaStreamTrackListPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaDevices::DeviceList
    //

    struct DeviceList : public std::list<Device>,
                        public Any
    {
      static DeviceListPtr convert(AnyPtr any) noexcept;

      DeviceList() noexcept {}
      DeviceList(const DeviceList &op2) noexcept {(*this) = op2;}
      DeviceList(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "devices") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaDevices
  //
  
  interaction IMediaDevices : public IMediaDevicesTypes
  {
    static ElementPtr toDebug() noexcept;

    static SupportedConstraintsPtr getSupportedConstraints() noexcept;

    static PromiseWithDeviceListPtr enumerateDevices() noexcept;
    static PromiseWithSettingsListPtr enumerateDefaultModes(const char *deviceID) noexcept;

    static PromiseWithMediaStreamTrackListPtr getUserMedia(const Constraints &constraints = Constraints()) noexcept;

    static IMediaDevicesSubscriptionPtr subscribe(IMediaDevicesDelegatePtr delegate) noexcept;

    virtual ~IMediaDevices() noexcept {} // make polymorphic
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaDevicesDelegate
  //

  interaction IMediaDevicesDelegate
  {
    virtual void onMediaDevicesChanged() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaDevicesSubscription
  //

  interaction IMediaDevicesSubscription
  {
    virtual PUID getID() const noexcept = 0;

    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IMediaDevicesDelegate)
ZS_DECLARE_PROXY_METHOD(onMediaDevicesChanged)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaDevicesDelegate, ortc::IMediaDevicesSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaDevicesChanged)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
