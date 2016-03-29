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
#include <ortc/internal/ortc_ISecureTransport.h>

#include <ortc/IICETransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>

#include "webrtc/base/scoped_ptr.h"
#include "webrtc/voice_engine/include/voe_base.h"

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineRegistration)

    // resource based interfaces
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineDeviceResource)

    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineSetupChannelResult)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineCloseChannelResult)

    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForSingleton)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPReceiverChannel)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPReceiverChannelMediaBase)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPReceiverChannelAudio)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPReceiverChannelVideo)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPSenderChannel)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPSenderChannelMediaBase)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPSenderChannelAudio)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForRTPSenderChannelVideo)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineForMediaStreamTrack)

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPMediaEngine)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelMediaBaseForRTPMediaEngine)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelAudioForRTPMediaEngine)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelVideoForRTPMediaEngine)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelMediaBaseForRTPMediaEngine)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelAudioForRTPMediaEngine)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelVideoForRTPMediaEngine)

    ZS_DECLARE_INTERACTION_PROXY(IRTPMediaEngineAsyncDelegate)

    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<RTPMediaEngine>, PromiseWithRTPMediaEngine)
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<IRTPMediaEngineDeviceResource>, PromiseWithRTPMediaEngineDeviceResource)
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<IRTPMediaEngineSetupChannelResult>, PromiseWithRTPMediaEngineSetupChannelResult)
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<IRTPMediaEngineCloseChannelResult>, PromiseWithRTPMediaEngineCloseChannelResult)


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark VoiceEngineDeleter
    #pragma mark

    struct VoiceEngineDeleter {
      VoiceEngineDeleter() {}
      inline void operator()(webrtc::VoiceEngine* ptr) const {
        webrtc::VoiceEngine::Delete(ptr);
      }
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineRegistration
    #pragma mark

    interaction IRTPMediaEngineRegistration : public Any
    {
      template <typename data_type>
      std::shared_ptr<data_type> engine() const {return ZS_DYNAMIC_PTR_CAST(data_type, getRTPEngine());}

      virtual RTPMediaEnginePtr getRTPEngine() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineDeviceResource
    #pragma mark

    interaction IRTPMediaEngineDeviceResource : public Any
    {
      virtual PUID getID() const = 0;
      virtual String getDeviceID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineSetupChannelResult
    #pragma mark

    interaction IRTPMediaEngineSetupChannelResult : public Any
    {
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineCloseChannelResult
    #pragma mark

    interaction IRTPMediaEngineCloseChannelResult : public Any
    {
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForSettings
    #pragma mark

    interaction IRTPMediaEngineForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPMediaEngineForSettings() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForSingleton
    #pragma mark

    interaction IRTPMediaEngineForSingleton
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForSingleton, ForSingleton)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPReceiverChannel
    #pragma mark

    interaction IRTPMediaEngineForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPReceiverChannel, ForRTPReceiverChannel)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPReceiverChannelMediaBase
    #pragma mark

    interaction IRTPMediaEngineForRTPReceiverChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPReceiverChannelMediaBase, ForRTPReceiverChannelMediaBase)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelMediaBaseForRTPMediaEngine, UseReceiverChannelMediaBase)

      ElementPtr toDebug(ForRTPReceiverChannelMediaBasePtr object);

      static PromiseWithRTPMediaEnginePtr create();

      static PromiseWithRTPMediaEngineDeviceResourcePtr getDeviceResource(const char *deviceID);

      static PromiseWithRTPMediaEngineSetupChannelResultPtr setupChannel(UseReceiverChannelMediaBasePtr channel);

      static PromiseWithRTPMediaEngineCloseChannelResultPtr closeChannel(UseReceiverChannelMediaBasePtr channel);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPReceiverChannelAudio
    #pragma mark

    interaction IRTPMediaEngineForRTPReceiverChannelAudio : public IRTPMediaEngineForRTPReceiverChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPReceiverChannelAudio, ForRTPReceiverChannelAudio)

      virtual rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> getVoiceEngine() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPReceiverChannelVideo
    #pragma mark

    interaction IRTPMediaEngineForRTPReceiverChannelVideo : public IRTPMediaEngineForRTPReceiverChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPReceiverChannelVideo, ForRTPReceiverChannelVideo)
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPSenderChannel
    #pragma mark

    interaction IRTPMediaEngineForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPSenderChannel, ForRTPSenderChannel)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPSenderChannelMediaBase
    #pragma mark

    interaction IRTPMediaEngineForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPSenderChannelMediaBase, ForRTPSenderChannelMediaBase)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelMediaBaseForRTPMediaEngine, UseSenderChannelMediaBase)

      ElementPtr toDebug(ForRTPSenderChannelMediaBasePtr object);

      static PromiseWithRTPMediaEnginePtr create();

      static PromiseWithRTPMediaEngineDeviceResourcePtr getDeviceResource(const char *deviceID);

      static PromiseWithRTPMediaEngineSetupChannelResultPtr setupChannel(UseSenderChannelMediaBasePtr channel);

      static PromiseWithRTPMediaEngineCloseChannelResultPtr closeChannel(UseSenderChannelMediaBasePtr channel);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPSenderChannelAudio
    #pragma mark

    interaction IRTPMediaEngineForRTPSenderChannelAudio : public IRTPMediaEngineForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPSenderChannelAudio, ForRTPSenderChannelAudio)

      virtual rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> getVoiceEngine() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPSenderChannelVideo
    #pragma mark

    interaction IRTPMediaEngineForRTPSenderChannelVideo : public IRTPMediaEngineForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPSenderChannelVideo, ForRTPSenderChannelVideo)
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForMediaStreamTrack
    #pragma mark

    interaction IRTPMediaEngineForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForMediaStreamTrack, ForMediaStreamTrack)

      virtual ~IRTPMediaEngineForMediaStreamTrack() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForDeviceResource
    #pragma mark

    interaction IRTPMediaEngineForDeviceResource
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForDeviceResource, ForDeviceResource)

      virtual void notifyResourceGone(IRTPMediaEngineDeviceResource &device) = 0;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineAsyncDelegate
    #pragma mark

    interaction IRTPMediaEngineAsyncDelegate
    {
      virtual ~IRTPMediaEngineAsyncDelegate() {}
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine
    #pragma mark
    
    class RTPMediaEngine : public Noop,
                           public MessageQueueAssociator,
                           public SharedRecursiveLock,
                           public IRTPMediaEngineForSettings,
                           public IRTPMediaEngineForSingleton,
                           public IRTPMediaEngineForRTPReceiverChannel,
                           public IRTPMediaEngineForRTPReceiverChannelAudio,
                           public IRTPMediaEngineForRTPReceiverChannelVideo,
                           public IRTPMediaEngineForRTPSenderChannel,
                           public IRTPMediaEngineForRTPSenderChannelAudio,
                           public IRTPMediaEngineForRTPSenderChannelVideo,
                           public IRTPMediaEngineForMediaStreamTrack,
                           public IRTPMediaEngineForDeviceResource,
                           public IWakeDelegate,
                           public zsLib::ITimerDelegate,
                           public IRTPMediaEngineAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend class RTPMediaEngineRegistration;

      friend interaction IRTPMediaEngine;
      friend interaction IRTPMediaEngineFactory;
      friend interaction IRTPMediaEngineForSettings;
      friend interaction IRTPMediaEngineForSingleton;
      friend interaction IRTPMediaEngineForRTPReceiverChannel;
      friend interaction IRTPMediaEngineForRTPReceiverChannelMediaBase;
      friend interaction IRTPMediaEngineForRTPReceiverChannelAudio;
      friend interaction IRTPMediaEngineForRTPReceiverChannelVideo;
      friend interaction IRTPMediaEngineForRTPSenderChannel;
      friend interaction IRTPMediaEngineForRTPSenderChannelMediaBase;
      friend interaction IRTPMediaEngineForRTPSenderChannelAudio;
      friend interaction IRTPMediaEngineForRTPSenderChannelVideo;
      friend interaction IRTPMediaEngineForMediaStreamTrack;
      friend interaction IRTPMediaEngineForDeviceResource;

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPMediaEngine, UseMediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelAudioForRTPMediaEngine, UseChannelAudio)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      ZS_DECLARE_CLASS_PTR(BaseResource)
      ZS_DECLARE_CLASS_PTR(DeviceResource)

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithRTPMediaEnginePtr>, PromiseWithRTPMediaEngineList)

      typedef std::map<PUID, DeviceResourceWeakPtr> DeviceResourceMap;
      typedef std::list<DeviceResourceWeakPtr> DeviceResourceList;
      typedef std::map<PUID, PromiseWeakPtr> PendingPromiseMap;
      typedef std::map<PUID, UseReceiverChannelMediaBaseWeakPtr> ReceiverChannelMap;
      typedef std::map<PUID, UseSenderChannelMediaBaseWeakPtr> SenderChannelMap;

    public:
      RTPMediaEngine(
                     const make_private &,
                     IMessageQueuePtr queue,
                     IRTPMediaEngineRegistrationPtr registration
                     );

    protected:
      RTPMediaEngine(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPMediaEngine();

      static RTPMediaEnginePtr convert(ForSettingsPtr object);
      static RTPMediaEnginePtr convert(ForRTPReceiverChannelPtr object);
      static RTPMediaEnginePtr convert(ForRTPReceiverChannelMediaBasePtr object);
      static RTPMediaEnginePtr convert(ForRTPReceiverChannelAudioPtr object);
      static RTPMediaEnginePtr convert(ForRTPReceiverChannelVideoPtr object);
      static RTPMediaEnginePtr convert(ForRTPSenderChannelMediaBasePtr object);
      static RTPMediaEnginePtr convert(ForRTPSenderChannelAudioPtr object);
      static RTPMediaEnginePtr convert(ForRTPSenderChannelVideoPtr object);
      static RTPMediaEnginePtr convert(ForMediaStreamTrackPtr object);
      static RTPMediaEnginePtr convert(ForDeviceResourcePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => RTPMediaEngineRegistration
      #pragma mark

      static PromiseWithRTPMediaEnginePtr createEnginePromise();

      static RTPMediaEnginePtr create(IRTPMediaEngineRegistrationPtr registration);

      void notify(PromiseWithRTPMediaEnginePtr promise);

      void shutdown();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannel
      #pragma mark

      virtual PUID getID() const override {return mID;}

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelMediaBase
      #pragma mark

      PromiseWithRTPMediaEngineDeviceResourcePtr getDeviceResource(const char *deviceID);

      PromiseWithRTPMediaEngineSetupChannelResultPtr setupChannel(UseReceiverChannelMediaBasePtr channel);

      PromiseWithRTPMediaEngineCloseChannelResultPtr closeChannel(UseReceiverChannelMediaBasePtr channel);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelAudio
      #pragma mark

      virtual rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> getVoiceEngine() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelVideo
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannel
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelMediaBase
      #pragma mark

      // (duplicate) PromiseWithRTPMediaEngineDeviceResourcePtr getDeviceResource(const char *deviceID) = 0;

      PromiseWithRTPMediaEngineSetupChannelResultPtr setupChannel(UseSenderChannelMediaBasePtr channel);

      PromiseWithRTPMediaEngineCloseChannelResultPtr closeChannel(UseSenderChannelMediaBasePtr channel);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelAudio
      #pragma mark

      // (duplicate) virtual rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> getVoiceEngine() = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelVideo
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForDeviceResource
      #pragma mark

      virtual void notifyResourceGone(IRTPMediaEngineDeviceResource &resource) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineAsyncDelegate
      #pragma mark

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isReady() const;
      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepSetup();
      bool stepExampleSetupDeviceResources();
      bool stepSetupSenderChannel();
      bool stepCloseSenderChannel();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    public:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::BaseResource
      #pragma mark

      class BaseResource : public Any,
                           public SharedRecursiveLock,
                           public MessageQueueAssociator
      {
      protected:
        struct make_private {};

      public:
        typedef std::list<PromiseWeakPtr> PendingPromiseList;

      public:
        BaseResource(
                     const make_private &,
                     IMessageQueuePtr queue,
                     IRTPMediaEngineRegistrationPtr registration
                     );
        ~BaseResource();

        void notifyReady();
        void notifyRejected();

        template <typename data_type>
        std::shared_ptr<PromiseWith<data_type> > createPromise() {return ZS_DYNAMIC_PTR_CAST(PromiseWith<data_type>, internalSetupPromise(PromiseWith<data_type>::create(delegateQueue())));}

        template <typename self_type>
        std::shared_ptr<self_type> getThis() const {return ZS_DYNAMIC_PTR_CAST(self_type, mThisWeak.lock());}

        template <typename engine_interface>
        std::shared_ptr<engine_interface> getEngine() const {return mMediaEngine.lock();}

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::DeviceResource => (internal)
        #pragma mark

        IMessageQueuePtr delegateQueue();
        PromisePtr internalSetupPromise(PromisePtr promise);
        void internalFixState();

      protected:
        AutoPUID mID;
        BaseResourceWeakPtr mThisWeak;

        bool mNotifiedReady {false};
        bool mNotifiedRejected {false};
        PendingPromiseList mPendingPromises;

        IRTPMediaEngineRegistrationPtr mRegistration;
        RTPMediaEngineWeakPtr mMediaEngine;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::DeviceResource
      #pragma mark

      class DeviceResource : public IRTPMediaEngineDeviceResource,
                             public BaseResource
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForDeviceResource, UseEngine)

      public:
        DeviceResource(
                       const make_private &,
                       IMessageQueuePtr queue,
                       IRTPMediaEngineRegistrationPtr registration,
                       const char *deviceID
                       );
        ~DeviceResource();

        static DeviceResourcePtr create(
                                        IRTPMediaEngineRegistrationPtr registration,
                                        const char *deviceID
                                        );

      protected:
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::DeviceResource => RTPMediaEngine
        #pragma mark

        virtual PUID getID() const {return mID;}

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::DeviceResource => IRTPMediaEngineDeviceResource
        #pragma mark

        // (duplicate) virtual PUID getID() const;
        virtual String getDeviceID() const;

      protected:
        String mDeviceID;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::SetupChannelResult
      #pragma mark

      class SetupChannelResult : public IRTPMediaEngineSetupChannelResult
      {
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::SetupChannelResult
      #pragma mark

      class CloseChannelResult : public IRTPMediaEngineCloseChannelResult
      {
      };


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => (data)
      #pragma mark

      AutoPUID mID;
      RTPMediaEngineWeakPtr mThisWeak;
      RTPMediaEnginePtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      IRTPMediaEngineRegistrationWeakPtr mRegistration;
      PromiseWithRTPMediaEngineList mPendingReady;

      DeviceResourceMap mExampleDeviceResources;
      DeviceResourceList mExamplePendingDeviceResources;

      ReceiverChannelMap mPendingSetupReceiverAudioChannels;
      PendingPromiseMap mPendingSetupReceiverAudioChannelPromises;
      ReceiverChannelMap mPendingSetupReceiverVideoChannels;
      PendingPromiseMap mPendingSetupReceiverVideoChannelPromises;
      SenderChannelMap mPendingSetupSenderAudioChannels;
      PendingPromiseMap mPendingSetupSenderAudioChannelPromises;
      SenderChannelMap mPendingSetupSenderVideoChannels;
      PendingPromiseMap mPendingSetupSenderVideoChannelPromises;

      ReceiverChannelMap mPendingCloseReceiverAudioChannels;
      PendingPromiseMap mPendingCloseReceiverAudioChannelPromises;
      ReceiverChannelMap mPendingCloseReceiverVideoChannels;
      PendingPromiseMap mPendingCloseReceiverVideoChannelPromises;
      SenderChannelMap mPendingCloseSenderAudioChannels;
      PendingPromiseMap mPendingCloseSenderAudioChannelPromises;
      SenderChannelMap mPendingCloseSenderVideoChannels;
      PendingPromiseMap mPendingCloseSenderVideoChannelPromises;

      rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> mVoiceEngine;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineFactory
    #pragma mark

    interaction IRTPMediaEngineFactory
    {
      static IRTPMediaEngineFactory &singleton();

      virtual RTPMediaEnginePtr create(IRTPMediaEngineRegistrationPtr registration);
    };

    class RTPMediaEngineFactory : public IFactory<IRTPMediaEngineFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPMediaEngineAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPMediaEngineAsyncDelegate::RTCPPacketListPtr, RTCPPacketListPtr)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTPPacketPtr, RTPPacketPtr)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPMediaEngineAsyncDelegate::ParametersPtr, ParametersPtr)
//ZS_DECLARE_PROXY_METHOD_1(onSecureTransportState, States)
//ZS_DECLARE_PROXY_METHOD_1(onNotifyPacket, RTPPacketPtr)
//ZS_DECLARE_PROXY_METHOD_1(onNotifyPackets, RTCPPacketListPtr)
//ZS_DECLARE_PROXY_METHOD_1(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()

