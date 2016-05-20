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
#include <webrtc/base/logging.h>
#include <webrtc/system_wrappers/include/trace.h>
#include <webrtc/audio/audio_send_stream.h>
#include <webrtc/audio/audio_receive_stream.h>
#include <webrtc/video/video_send_stream.h>
#include <webrtc/video/video_receive_stream.h>
#include <webrtc/video/call_stats.h>
#include <webrtc/modules/congestion_controller/include/congestion_controller.h>
#include <webrtc/modules/utility/include/process_thread.h>
#include <webrtc/voice_engine/include/voe_base.h>
#include <webrtc/video/vie_remb.h>
//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineRegistration)

    // resource based interfaces
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineDeviceResource)
    ZS_DECLARE_INTERACTION_PTR(IRTPMediaEngineChannelResource)
    ZS_DECLARE_INTERACTION_PROXY(IRTPMediaEngineChannelResourceAsyncDelegate)
    ZS_DECLARE_INTERACTION_PTR(IChannelResourceForRTPMediaEngine)

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
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<IRTPMediaEngineChannelResource>, PromiseWithRTPMediaEngineChannelResource)
    
    ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)


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
    #pragma mark IRTPMediaEngineChannelResource
    #pragma mark

    interaction IRTPMediaEngineChannelResource : public Any
    {
      virtual PUID getID() const = 0;

      virtual PromisePtr shutdown() = 0;

      virtual void notifyTransportState(ISecureTransportTypes::States state) = 0;

      virtual void notifyUpdate(ParametersPtr params) = 0;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineChannelResourceAsyncDelegate
    #pragma mark

    interaction IRTPMediaEngineChannelResourceAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      virtual void onSecureTransportState(ISecureTransport::States state) = 0;
      virtual void onUpdate(ParametersPtr params) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IChannelResourceForRTPMediaEngine
    #pragma mark

    interaction IChannelResourceForRTPMediaEngine
    {
      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineAudioReceiverChannelResource
    #pragma mark

    interaction IRTPMediaEngineAudioReceiverChannelResource : public IRTPMediaEngineChannelResource
    {
      virtual bool handlePacket(const RTPPacket &packet) = 0;
      virtual bool handlePacket(const RTCPPacket &packet) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineAudioSenderChannelResource
    #pragma mark

    interaction IRTPMediaEngineAudioSenderChannelResource : public IRTPMediaEngineChannelResource
    {
      virtual bool handlePacket(const RTCPPacket &packet) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineVideoReceiverChannelResource
    #pragma mark

    interaction IRTPMediaEngineVideoReceiverChannelResource : public IRTPMediaEngineChannelResource
    {
      virtual bool handlePacket(const RTPPacket &packet) = 0;
      virtual bool handlePacket(const RTCPPacket &packet) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineVideoSenderChannelResource
    #pragma mark

    interaction IRTPMediaEngineVideoSenderChannelResource : public IRTPMediaEngineChannelResource
    {
      virtual bool handlePacket(const RTCPPacket &packet) = 0;
      virtual void sendVideoFrame(const webrtc::VideoFrame& videoFrame) = 0;
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
      ZS_DECLARE_TYPEDEF_PTR(webrtc::Transport, Transport)

      ElementPtr toDebug(ForRTPReceiverChannelMediaBasePtr object);

      static PromiseWithRTPMediaEnginePtr create();

      static PromiseWithRTPMediaEngineDeviceResourcePtr getDeviceResource(const char *deviceID);

      static PromiseWithRTPMediaEngineChannelResourcePtr setupChannel(
                                                                      UseReceiverChannelMediaBasePtr channel,
                                                                      TransportPtr transport,
                                                                      MediaStreamTrackPtr track,
                                                                      ParametersPtr parameters,
                                                                      RTPPacketPtr packet
                                                                      );

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
      
      virtual webrtc::VoiceEngine *getVoiceEngine() = 0;
      virtual rtc::scoped_refptr<webrtc::AudioState> getAudioState() = 0;
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
      ZS_DECLARE_TYPEDEF_PTR(webrtc::Transport, Transport)

      ElementPtr toDebug(ForRTPSenderChannelMediaBasePtr object);

      static PromiseWithRTPMediaEnginePtr create();

      static PromiseWithRTPMediaEngineDeviceResourcePtr getDeviceResource(const char *deviceID);

      static PromiseWithRTPMediaEngineChannelResourcePtr setupChannel(
                                                                      UseSenderChannelMediaBasePtr channel,
                                                                      TransportPtr transport,
                                                                      MediaStreamTrackPtr track,
                                                                      ParametersPtr parameters
                                                                      );

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

      virtual webrtc::VoiceEngine *getVoiceEngine() = 0;
      virtual rtc::scoped_refptr<webrtc::AudioState> getAudioState() = 0;
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
    #pragma mark IRTPMediaEngineForChannelResource
    #pragma mark

    interaction IRTPMediaEngineForChannelResource
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForChannelResource, ForChannelResource)

      virtual void notifyResourceGone(IChannelResourceForRTPMediaEngine &channel) = 0;
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
    
    class RTPMediaEngine : public Any,
                           public Noop,
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
                           public IRTPMediaEngineForChannelResource,
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
      friend interaction IRTPMediaEngineForChannelResource;

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPMediaEngine, UseMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(webrtc::Transport, Transport)

      ZS_DECLARE_CLASS_PTR(BaseResource)
      ZS_DECLARE_CLASS_PTR(DeviceResource)
      ZS_DECLARE_CLASS_PTR(ChannelResource)
      ZS_DECLARE_CLASS_PTR(AudioReceiverChannelResource)
      ZS_DECLARE_CLASS_PTR(AudioSenderChannelResource)
      ZS_DECLARE_CLASS_PTR(VideoReceiverChannelResource)
      ZS_DECLARE_CLASS_PTR(VideoSenderChannelResource)

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
      typedef std::map<PUID, ChannelResourceWeakPtr> ChannelResourceWeakMap;
      typedef std::list<ChannelResourcePtr> ChannelResourceList;

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

      PromiseWithRTPMediaEngineChannelResourcePtr setupChannel(
                                                               UseReceiverChannelMediaBasePtr channel,
                                                               TransportPtr transport,
                                                               MediaStreamTrackPtr track,
                                                               ParametersPtr parameters,
                                                               RTPPacketPtr packet
                                                               );

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelAudio
      #pragma mark

      virtual webrtc::VoiceEngine *getVoiceEngine() override;

      virtual rtc::scoped_refptr<webrtc::AudioState> getAudioState() override;

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

      PromiseWithRTPMediaEngineChannelResourcePtr setupChannel(
                                                               UseSenderChannelMediaBasePtr channel,
                                                               TransportPtr transport,
                                                               MediaStreamTrackPtr track,
                                                               ParametersPtr parameters
                                                               );

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelAudio
      #pragma mark

      // (duplicate) virtual webrtc::VoiceEngine *getVoiceEngine() = 0;

      // (duplicate) virtual rtc::scoped_refptr<webrtc::AudioState> getAudioState() = 0;

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
      #pragma mark RTPMediaEngine => IRTPMediaEngineForChannelResource
      #pragma mark

      virtual void notifyResourceGone(IChannelResourceForRTPMediaEngine &resource) override;

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

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine => (friend ChannelResource)
      #pragma mark

      void shutdownChannelResource(ChannelResourcePtr channelResource);

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
      bool stepSetupChannels();
      bool stepCloseChannels();

      void cancel();
      void stepCancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    public:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::WebRtcTraceCallback
      #pragma mark

      class WebRtcTraceCallback : public webrtc::TraceCallback
      {
      public:
        virtual void Print(webrtc::TraceLevel level, const char* message, int length);
      };

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
        virtual ~BaseResource();

        void notifyPromisesResolve();
        void notifyPromisesReject();

        std::shared_ptr<Promise> createPromise() { return internalSetupPromise(Promise::create(delegateQueue())); }

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
        virtual ~DeviceResource();

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
      #pragma mark RTPMediaEngine::ChannelResource
      #pragma mark

      class ChannelResource : public BaseResource,
                              public IChannelResourceForRTPMediaEngine,
                              public IRTPMediaEngineChannelResourceAsyncDelegate
      {
        typedef std::list<PromisePtr> PromiseList;

        ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForChannelResource, UseEngine)

      public:
        ChannelResource(
                        const make_private &priv,
                        IMessageQueuePtr queue,
                        IRTPMediaEngineRegistrationPtr registration
                        ) : BaseResource(priv, queue, registration) {}

        virtual ~ChannelResource();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::ChannelResource => IRTPMediaEngineChannelResource
        #pragma mark

        virtual PUID getID() const { return mID; }
        virtual PromisePtr shutdown();
        virtual void notifyTransportState(ISecureTransportTypes::States state);
        virtual void notifyUpdate(ParametersPtr params);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::ChannelResource => (friend RTPMediaEngine)
        #pragma mark

        virtual void notifySetup() = 0;
        virtual void notifyShutdown() = 0;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::ChannelResource => (internal friend derived)
        #pragma mark

        bool isShuttingDown() const {return mShuttingDown;}
        bool isShutdown() const {return mShutdown;}
        void notifyPromisesShutdown();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::ChannelResource => (internal)
        #pragma mark

        PromisePtr getShutdownPromise();

      protected:

        bool mShuttingDown {false};
        bool mShutdown {false};
        PromiseList mShutdownPromises;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::AudioReceiverChannelResource
      #pragma mark

      class AudioReceiverChannelResource : public IRTPMediaEngineAudioReceiverChannelResource,
                                           public ChannelResource,
                                           public webrtc::BitrateObserver
      {
      public:
        friend class RTPMediaEngine;

      public:
        AudioReceiverChannelResource(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     IRTPMediaEngineRegistrationPtr registration,
                                     TransportPtr transport,
                                     MediaStreamTrackPtr track,
                                     ParametersPtr parameters,
                                     RTPPacketPtr packet
                                     );
        virtual ~AudioReceiverChannelResource();

        static AudioReceiverChannelResourcePtr create(
                                                      IRTPMediaEngineRegistrationPtr registration,
                                                      TransportPtr transport,
                                                      MediaStreamTrackPtr track,
                                                      ParametersPtr parameters,
                                                      RTPPacketPtr packet
                                                      );

      protected:
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineChannelResource
        #pragma mark

        virtual PUID getID() const override {return ChannelResource::getID();}

        virtual PromisePtr shutdown() override {return ChannelResource::shutdown();}

        virtual void notifyTransportState(ISecureTransportTypes::States state) override { return ChannelResource::notifyTransportState(state); }

        virtual void notifyUpdate(ParametersPtr params) override { return ChannelResource::notifyUpdate(params); }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
        #pragma mark

        virtual void onSecureTransportState(ISecureTransport::States state) override;

        virtual void onUpdate(ParametersPtr params) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineAudioReceiverChannelResource
        #pragma mark

        virtual bool handlePacket(const RTPPacket &packet) override;
        virtual bool handlePacket(const RTCPPacket &packet) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioReceiverChannelResource => webrtc::BitrateObserver
        #pragma mark

        virtual void OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs) override;

      protected:
        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioReceiverChannelResource => friend RTPMediaEngine
        #pragma mark

        virtual void notifySetup() override;
        virtual void notifyShutdown() override;

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioReceiverChannelResource => (internal)
        #pragma mark

        int getChannel() const;
        webrtc::CodecInst getAudioCodec(
                                        webrtc::VoiceEngine *voiceEngine,
                                        String payloadName
                                        );

      protected:
        int mChannel{};

        TransportPtr mTransport;
        std::atomic<ISecureTransport::States> mTransportState { ISecureTransport::State_Pending };

        UseMediaStreamTrackPtr mTrack;

        ParametersPtr mParameters;

        RTPPacketPtr mInitPacket;

        rtc::scoped_ptr<webrtc::ProcessThread> mModuleProcessThread;
        rtc::scoped_ptr<webrtc::ProcessThread> mPacerThread;
        rtc::scoped_ptr<webrtc::AudioReceiveStream> mReceiveStream;
        rtc::scoped_ptr<webrtc::Clock> mClock;
        webrtc::VieRemb mRemb;
        rtc::scoped_ptr<webrtc::CallStats> mCallStats;
        rtc::scoped_ptr<webrtc::CongestionController> mCongestionController;
        rtc::scoped_ptr<webrtc::BitrateAllocator> mBitrateAllocator;

        webrtc::AudioDeviceModule* mAudioDeviceModule;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::AudioSenderChannelResource
      #pragma mark

      class AudioSenderChannelResource : public IRTPMediaEngineAudioSenderChannelResource,
                                         public ChannelResource,
                                         public webrtc::BitrateObserver
      {
      public:
        friend class RTPMediaEngine;

      public:
        AudioSenderChannelResource(
                                   const make_private &,
                                   IMessageQueuePtr queue,
                                   IRTPMediaEngineRegistrationPtr registration,
                                   TransportPtr transport,
                                   MediaStreamTrackPtr track,
                                   ParametersPtr parameters
                                   );
        virtual ~AudioSenderChannelResource();

        static AudioSenderChannelResourcePtr create(
                                                    IRTPMediaEngineRegistrationPtr registration,
                                                    TransportPtr transport,
                                                    MediaStreamTrackPtr track,
                                                    ParametersPtr parameters
                                                    );

      protected:
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineChannelResource
        #pragma mark

        virtual PUID getID() const override {return ChannelResource::getID();}

        virtual PromisePtr shutdown() override {return ChannelResource::shutdown();}

        virtual void notifyTransportState(ISecureTransportTypes::States state) override { return ChannelResource::notifyTransportState(state); }

        virtual void notifyUpdate(ParametersPtr params) override { return ChannelResource::notifyUpdate(params); }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
        #pragma mark

        virtual void onSecureTransportState(ISecureTransport::States state) override;

        virtual void onUpdate(ParametersPtr params) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineAudioSenderChannelResource
        #pragma mark

        virtual bool handlePacket(const RTCPPacket &packet) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioSenderChannelResource => webrtc::BitrateObserver
        #pragma mark

        virtual void OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs) override;

      protected:
        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioSenderChannelResource => friend RTPMediaEngine
        #pragma mark

        virtual void notifySetup() override;
        virtual void notifyShutdown() override;

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::AudioSenderChannelResource => (internal)
        #pragma mark

        webrtc::CodecInst getAudioCodec(
                                        webrtc::VoiceEngine *voiceEngine,
                                        String payloadName
                                        );

      protected:
        std::atomic<size_t> mAccessFromNonLockedMethods {};

        int mChannel{};

        TransportPtr mTransport;
        std::atomic<ISecureTransport::States> mTransportState { ISecureTransport::State_Pending };

        UseMediaStreamTrackPtr mTrack;

        ParametersPtr mParameters;

        rtc::scoped_ptr<webrtc::ProcessThread> mModuleProcessThread;
        rtc::scoped_ptr<webrtc::ProcessThread> mPacerThread;
        rtc::scoped_ptr<webrtc::AudioSendStream> mSendStream;
        rtc::scoped_ptr<webrtc::Clock> mClock;
        webrtc::VieRemb mRemb;
        rtc::scoped_ptr<webrtc::CongestionController> mCongestionController;
        rtc::scoped_ptr<webrtc::BitrateAllocator> mBitrateAllocator;

        webrtc::AudioDeviceModule* mAudioDeviceModule;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::VideoReceiverChannelResource
      #pragma mark

      class VideoReceiverChannelResource : public IRTPMediaEngineVideoReceiverChannelResource,
                                           public ChannelResource,
                                           public webrtc::BitrateObserver
      {
      public:
        friend class RTPMediaEngine;

        class ReceiverVideoRenderer : public webrtc::VideoRenderer
        {
        public:
          void setMediaStreamTrack(UseMediaStreamTrackPtr videoTrack);

          virtual void RenderFrame(
                                   const webrtc::VideoFrame& video_frame,
                                   int time_to_render_ms
                                   ) override;

          virtual bool IsTextureSupported() const override;

        private:
          UseMediaStreamTrackPtr mVideoTrack;
        };

      public:
        VideoReceiverChannelResource(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     IRTPMediaEngineRegistrationPtr registration,
                                     TransportPtr transport,
                                     MediaStreamTrackPtr track,
                                     ParametersPtr parameters,
                                     RTPPacketPtr packet
                                     );
        virtual ~VideoReceiverChannelResource();

        static VideoReceiverChannelResourcePtr create(
                                                      IRTPMediaEngineRegistrationPtr registration,
                                                      TransportPtr transport,
                                                      MediaStreamTrackPtr track,
                                                      ParametersPtr parameters,
                                                      RTPPacketPtr packet
                                                      );

      protected:
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineChannelResource
        #pragma mark

        virtual PUID getID() const override {return ChannelResource::getID();}

        virtual PromisePtr shutdown() override {return ChannelResource::shutdown();}

        virtual void notifyTransportState(ISecureTransportTypes::States state) override { return ChannelResource::notifyTransportState(state); }

        virtual void notifyUpdate(ParametersPtr params) override { return ChannelResource::notifyUpdate(params); }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
        #pragma mark

        virtual void onSecureTransportState(ISecureTransport::States state) override;

        virtual void onUpdate(ParametersPtr params) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineVideoReceiverChannelResource
        #pragma mark

        virtual bool handlePacket(const RTPPacket &packet) override;
        virtual bool handlePacket(const RTCPPacket &packet) override;

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoReceiverChannelResource => friend RTPMediaEngine
        #pragma mark

        virtual void notifySetup() override;
        virtual void notifyShutdown() override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoReceiverChannelResource => webrtc::BitrateObserver
        #pragma mark

        virtual void OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs) override;

      protected:
        std::atomic<size_t> mAccessFromNonLockedMethods {};

        TransportPtr mTransport;
        std::atomic<ISecureTransport::States> mTransportState { ISecureTransport::State_Pending };

        UseMediaStreamTrackPtr mTrack;

        ParametersPtr mParameters;

        RTPPacketPtr mInitPacket;

        rtc::scoped_ptr<webrtc::ProcessThread> mModuleProcessThread;
        rtc::scoped_ptr<webrtc::ProcessThread> mPacerThread;
        rtc::scoped_ptr<webrtc::VideoReceiveStream> mReceiveStream;
        rtc::scoped_ptr<webrtc::Clock> mClock;
        webrtc::VieRemb mRemb;
        rtc::scoped_ptr<webrtc::CallStats> mCallStats;
        rtc::scoped_ptr<webrtc::CongestionController> mCongestionController;
        rtc::scoped_ptr<webrtc::BitrateAllocator> mBitrateAllocator;
        ReceiverVideoRenderer mReceiverVideoRenderer;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPMediaEngine::VideoSenderChannelResource
      #pragma mark

      class VideoSenderChannelResource : public IRTPMediaEngineVideoSenderChannelResource,
                                         public ChannelResource,
                                         public webrtc::BitrateObserver
      {
      public:
        friend class RTPMediaEngine;

        union VideoEncoderSettings {
          webrtc::VideoCodecVP8 mVp8;
          webrtc::VideoCodecVP9 mVp9;
          webrtc::VideoCodecH264 mH264;
        };

      public:
        VideoSenderChannelResource(
                                   const make_private &,
                                   IMessageQueuePtr queue,
                                   IRTPMediaEngineRegistrationPtr registration,
                                   TransportPtr transport,
                                   MediaStreamTrackPtr track,
                                   ParametersPtr parameters
                                   );
        virtual ~VideoSenderChannelResource();

        static VideoSenderChannelResourcePtr create(
                                                    IRTPMediaEngineRegistrationPtr registration,
                                                    TransportPtr transport,
                                                    MediaStreamTrackPtr track,
                                                    ParametersPtr parameters
                                                    );

      protected:
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineChannelResource
        #pragma mark

        virtual PUID getID() const override {return ChannelResource::getID();}

        virtual PromisePtr shutdown() override {return ChannelResource::shutdown();}

        virtual void notifyTransportState(ISecureTransportTypes::States state) override { return ChannelResource::notifyTransportState(state); }

        virtual void notifyUpdate(ParametersPtr params) override { return ChannelResource::notifyUpdate(params); }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
        #pragma mark

        virtual void onSecureTransportState(ISecureTransport::States state) override;

        virtual void onUpdate(ParametersPtr params) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineVideoSenderChannelResource
        #pragma mark

        virtual bool handlePacket(const RTCPPacket &packet) override;
        virtual void sendVideoFrame(const webrtc::VideoFrame& videoFrame) override;

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoSenderChannelResource => friend RTPMediaEngine
        #pragma mark

        virtual void notifySetup() override;
        virtual void notifyShutdown() override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPMediaEngine::VideoSenderChannelResource => webrtc::BitrateObserver
        #pragma mark

        virtual void OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs) override;

      protected:
        std::atomic<size_t> mAccessFromNonLockedMethods {};

        TransportPtr mTransport;
        std::atomic<ISecureTransport::States> mTransportState { ISecureTransport::State_Pending };

        UseMediaStreamTrackPtr mTrack;

        ParametersPtr mParameters;

        rtc::scoped_ptr<webrtc::ProcessThread> mModuleProcessThread;
        rtc::scoped_ptr<webrtc::ProcessThread> mPacerThread;
        rtc::scoped_ptr<webrtc::VideoSendStream> mSendStream;
        rtc::scoped_ptr<webrtc::Clock> mClock;
        webrtc::VieRemb mRemb;
        rtc::scoped_ptr<webrtc::CallStats> mCallStats;
        rtc::scoped_ptr<webrtc::CongestionController> mCongestionController;
        rtc::scoped_ptr<webrtc::BitrateAllocator> mBitrateAllocator;
        VideoEncoderSettings mVideoEncoderSettings;
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

      ChannelResourceWeakMap mChannelResources;
      ChannelResourceList mPendingSetupChannelResources;
      ChannelResourceList mPendingCloseChannelResources;

      rtc::scoped_refptr<webrtc::AudioState> mAudioState;
      rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> mVoiceEngine;

      rtc::scoped_ptr<WebRtcTraceCallback> mTraceCallback;
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

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPMediaEngineChannelResourceAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
ZS_DECLARE_PROXY_METHOD_1(onSecureTransportState, States)
ZS_DECLARE_PROXY_METHOD_1(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPMediaEngineAsyncDelegate)
ZS_DECLARE_PROXY_END()
