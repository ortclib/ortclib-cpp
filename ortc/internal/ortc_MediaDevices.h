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
    #pragma mark
    #pragma mark IMediaDevicesAsyncDelegate
    #pragma mark

    interaction IMediaDevicesAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::Constraints, Constraints)
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList)
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr, PromiseWithMediaStreamTrackListPtr)

      virtual void onEnumerateDevices(PromiseWithDeviceListPtr promise) = 0;

      virtual void onGetUserMedia(PromiseWithMediaStreamTrackListPtr promise, ConstraintsPtr constraints) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDevices
    #pragma mark
    
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
      static const char *toString(States state);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::Constraints, Constraints)
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList)
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithMediaStreamTrackList, PromiseWithMediaStreamTrackList)

    public:
      MediaDevices(
                   const make_private &,
                   IMessageQueuePtr queue
                   );

    protected:
      MediaDevices(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

      static MediaDevicesPtr create();
      static MediaDevicesPtr singleton();

    public:
      virtual ~MediaDevices();

      static MediaDevicesPtr convert(IMediaDevicesPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDevices => IMediaDevices
      #pragma mark

      static ElementPtr singletonToDebug();

      static SupportedConstraintsPtr getSupportedConstraints();

      static PromiseWithDeviceListPtr enumerateDevices();

      static PromiseWithMediaStreamTrackListPtr getUserMedia(const Constraints &constraints = Constraints());

      static IMediaDevicesSubscriptionPtr subscribe(IMediaDevicesDelegatePtr delegate);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDevices => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDevices => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDevices => IMediaDevicesAsyncDelegate
      #pragma mark

      virtual void onEnumerateDevices(PromiseWithDeviceListPtr promise) override;

      virtual void onGetUserMedia(PromiseWithMediaStreamTrackListPtr promise, ConstraintsPtr constraints) override;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MessageQueueManager => ISingletonManagerDelegate
      #pragma mark

      virtual void notifySingletonCleanup() override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDevices => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepBogusDoSomething();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDevices => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IMediaDevicesFactory
    #pragma mark

    interaction IMediaDevicesFactory
    {
      typedef IMediaDevicesTypes::Constraints Constraints;
      typedef IMediaDevicesTypes::SupportedConstraintsPtr SupportedConstraintsPtr;
      typedef IMediaDevicesTypes::PromiseWithDeviceListPtr PromiseWithDeviceListPtr;
      typedef IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr PromiseWithMediaStreamTrackListPtr;

      static IMediaDevicesFactory &singleton();

      virtual MediaDevicesPtr create();

      virtual ElementPtr singletonToDebug();

      virtual SupportedConstraintsPtr getSupportedConstraints();

      virtual PromiseWithDeviceListPtr enumerateDevices();

      virtual PromiseWithMediaStreamTrackListPtr getUserMedia(const Constraints &constraints = Constraints());

      virtual IMediaDevicesSubscriptionPtr subscribe(IMediaDevicesDelegatePtr delegate);
    };

    class MediaDevicesFactory : public IFactory<IMediaDevicesFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaDevicesAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaDevicesTypes::ConstraintsPtr, ConstraintsPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaDevicesAsyncDelegate::PromiseWithDeviceListPtr, PromiseWithDeviceListPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaDevicesAsyncDelegate::PromiseWithMediaStreamTrackListPtr, PromiseWithMediaStreamTrackListPtr)
ZS_DECLARE_PROXY_METHOD_1(onEnumerateDevices, PromiseWithDeviceListPtr)
ZS_DECLARE_PROXY_METHOD_2(onGetUserMedia, PromiseWithMediaStreamTrackListPtr, ConstraintsPtr)
ZS_DECLARE_PROXY_END()

