/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/types.h>

#include <ortc/IMediaDevices.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>
#include <zsLib/Singleton.h>

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PROXY(IMediaDevicesAsyncDelegate);


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDevicesAsyncDelegate
    //

    interaction IMediaDevicesAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::Constraints, Constraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithMediaStreamTrackList, PromiseWithMediaStreamTrackList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, PromiseWithSettingsList);

      virtual void onEnumerateDevices(PromiseWithDeviceListPtr promise) = 0;
      virtual void onEnumerateDefaultModes(PromiseWithSettingsListPtr promise, const char *deviceID) = 0;

      virtual void onGetUserMedia(PromiseWithMediaStreamTrackListPtr promise, ConstraintsPtr constraints) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevices
    //
    
    class MediaDevices : public Noop,
                         public MessageQueueAssociator,
                         public SharedRecursiveLock,
                         public IMediaDevices,
                         public IWakeDelegate,
                         public zsLib::ITimerDelegate,
                         public IMediaDevicesAsyncDelegate,
                         public ISingletonManagerDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaDevices;
      friend interaction IMediaDevicesFactory;

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state) noexcept;

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::Constraints, Constraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithMediaStreamTrackList, PromiseWithMediaStreamTrackList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, PromiseWithSettingsList);

    public:
      MediaDevices(
                   const make_private &,
                   IMessageQueuePtr queue
                   ) noexcept;

    protected:
      MediaDevices(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

      static MediaDevicesPtr create() noexcept;
      static MediaDevicesPtr singleton() noexcept;

    public:
      virtual ~MediaDevices() noexcept;

      static MediaDevicesPtr convert(IMediaDevicesPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaDevices => IMediaDevices
      //

      static ElementPtr singletonToDebug() noexcept;

      static SupportedConstraintsPtr getSupportedConstraints() noexcept;

      static PromiseWithDeviceListPtr enumerateDevices() noexcept;
      static PromiseWithSettingsListPtr enumerateDefaultModes(const char *deviceID) noexcept;

      static PromiseWithMediaStreamTrackListPtr getUserMedia(const Constraints &constraints = Constraints()) noexcept;

      static IMediaDevicesSubscriptionPtr subscribe(IMediaDevicesDelegatePtr delegate) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaDevices => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // MediaDevices => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // MediaDevices => IMediaDevicesAsyncDelegate
      //

      void onEnumerateDevices(PromiseWithDeviceListPtr promise) override;
      void onEnumerateDefaultModes(
                                   PromiseWithSettingsListPtr promise,
                                   const char *deviceID
                                   ) override;

      void onGetUserMedia(PromiseWithMediaStreamTrackListPtr promise, ConstraintsPtr constraints) override;

      //---------------------------------------------------------------------
      //
      // MessageQueueManager => ISingletonManagerDelegate
      //

      void notifySingletonCleanup() noexcept override;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaDevices => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      bool stepBogusDoSomething() noexcept;

      void cancel() noexcept;

      void setState(States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaDevices => (data)
      //

      AutoPUID mID;
      MediaDevicesWeakPtr mThisWeak;
      MediaDevicesPtr mGracefulShutdownReference;

      IMediaDevicesDelegateSubscriptions mSubscriptions;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDevicesFactory
    //

    interaction IMediaDevicesFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::Constraints, Constraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::SupportedConstraints, SupportedConstraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithMediaStreamTrackList, PromiseWithMediaStreamTrackList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, PromiseWithSettingsList);

      static IMediaDevicesFactory &singleton() noexcept;

      virtual MediaDevicesPtr create() noexcept;

      virtual ElementPtr singletonToDebug() noexcept;

      virtual SupportedConstraintsPtr getSupportedConstraints() noexcept;

      virtual PromiseWithDeviceListPtr enumerateDevices() noexcept;
      virtual PromiseWithSettingsListPtr enumerateDefaultModes(const char *deviceID) noexcept;

      virtual PromiseWithMediaStreamTrackListPtr getUserMedia(const Constraints &constraints = Constraints()) noexcept;

      virtual IMediaDevicesSubscriptionPtr subscribe(IMediaDevicesDelegatePtr delegate) noexcept;
    };

    class MediaDevicesFactory : public IFactory<IMediaDevicesFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaDevicesAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaDevicesTypes::ConstraintsPtr, ConstraintsPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaDevicesAsyncDelegate::PromiseWithDeviceListPtr, PromiseWithDeviceListPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaDevicesAsyncDelegate::PromiseWithMediaStreamTrackListPtr, PromiseWithMediaStreamTrackListPtr)
ZS_DECLARE_PROXY_METHOD(onEnumerateDevices, PromiseWithDeviceListPtr)
ZS_DECLARE_PROXY_METHOD(onEnumerateDefaultModes, PromiseWithSettingsListPtr, const char *)
ZS_DECLARE_PROXY_METHOD(onGetUserMedia, PromiseWithMediaStreamTrackListPtr, ConstraintsPtr)
ZS_DECLARE_PROXY_END()

