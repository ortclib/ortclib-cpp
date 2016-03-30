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

#include <ortc/IMediaStreamTrack.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>

#include <webrtc/video_frame.h>
#include <webrtc/modules/video_capture/include/video_capture.h>
#include <webrtc/modules/video_render/include/video_render.h>
#include <webrtc/modules/audio_device/include/audio_device.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForSettings)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSender)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannel)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelAudio)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelVideo)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiver)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannel)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannelMediaBase)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannelAudio)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannelVideo)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaDevices)

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForMediaStreamTrack)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForMediaStreamTrack)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForMediaStreamTrack)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForMediaStreamTrack)

    ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackAsyncDelegate)
    
    using zsLib::FLOAT;
    using zsLib::INT;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForSettings
    #pragma mark

    interaction IMediaStreamTrackForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IMediaStreamTrackForSettings() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSender
    #pragma mark

    interaction IMediaStreamTrackForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSender, ForSender)

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds)

      virtual PUID getID() const = 0;

      virtual void setSender(IRTPSenderPtr sender) = 0;

      virtual Kinds kind() const = 0;

      virtual void notifyAttachSenderChannel(RTPSenderChannelPtr channel) = 0;
      virtual void notifyDetachSenderChannel(RTPSenderChannelPtr channel) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSenderChannel
    #pragma mark

    interaction IMediaStreamTrackForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannel, ForSenderChannel)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSenderChannelMediaBase
    #pragma mark

    interaction IMediaStreamTrackForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase, ForSenderChannelMediaBase)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSenderChannelAudio
    #pragma mark

    interaction IMediaStreamTrackForRTPSenderChannelAudio : public IMediaStreamTrackForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelAudio, ForSenderChannelAudio)

      virtual webrtc::AudioDeviceModule* getAudioDeviceModule() = 0;

      virtual void start() = 0;
      virtual void stop() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSenderChannelVideo
    #pragma mark

    interaction IMediaStreamTrackForRTPSenderChannelVideo : public IMediaStreamTrackForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelVideo, ForSenderChannelVideo)

      virtual webrtc::AudioDeviceModule* getAudioDeviceModule() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiver
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, ForReceiver)

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds)

      static MediaStreamTrackPtr create(Kinds kind);

      virtual PUID getID() const = 0;

      virtual void setReceiver(IRTPReceiverPtr receiver) = 0;

      virtual void notifyActiveReceiverChannel(RTPReceiverChannelPtr channel) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiverChannel
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannel, ForReceiverChannel)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiverChannelMediabase
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiverChannelMediabase
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannelMediabase, ForReceiverChannelMediaBase)

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiverChannelAudio
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiverChannelAudio : public IMediaStreamTrackForRTPReceiverChannelMediabase
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannelAudio, ForReceiverChannelAudio)

      virtual webrtc::AudioDeviceModule* getAudioDeviceModule() = 0;

      virtual void start() = 0;
      virtual void stop() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiverChannelVideo
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiverChannelVideo : public IMediaStreamTrackForRTPReceiverChannelMediabase
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannelVideo, ForReceiverChannelVideo)

      virtual void renderVideoFrame(const webrtc::VideoFrame& videoFrame) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaDevices
    #pragma mark

    interaction IMediaStreamTrackForMediaDevices
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaDevices, ForMediaDevices)

      virtual ~IMediaStreamTrackForMediaDevices() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPMediaEngine
    #pragma mark

    interaction IMediaStreamTrackForRTPMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPMediaEngine, ForMediaEngine)

      virtual webrtc::AudioDeviceModule* getAudioDeviceModule() = 0;

      virtual void start() = 0;
      virtual void stop() = 0;

      virtual void renderVideoFrame(const webrtc::VideoFrame& videoFrame) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackAsyncDelegate
    #pragma mark

    interaction IMediaStreamTrackAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForMediaStreamTrack, UseReceiverChannel)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForMediaStreamTrack, UseSenderChannel)

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) = 0;

      virtual void onApplyConstraints(
                                      PromisePtr promise,
                                      TrackConstraintsPtr constraints
                                      ) = 0;

      virtual void onSetActiveReceiverChannel(UseReceiverChannelPtr channel) = 0;
      
      virtual void onAttachSenderChannel(UseSenderChannelPtr channel) = 0;
      virtual void onDetachSenderChannel(UseSenderChannelPtr channel) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack
    #pragma mark
    
    class MediaStreamTrack : public Noop,
                             public MessageQueueAssociator,
                             public SharedRecursiveLock,
                             public IMediaStreamTrack,
                             public IMediaStreamTrackForSettings,
                             public IMediaStreamTrackForRTPSender,
                             public IMediaStreamTrackForRTPSenderChannel,
                             public IMediaStreamTrackForRTPSenderChannelAudio,
                             public IMediaStreamTrackForRTPSenderChannelVideo,
                             public IMediaStreamTrackForRTPReceiver,
                             public IMediaStreamTrackForRTPReceiverChannel,
                             public IMediaStreamTrackForRTPReceiverChannelAudio,
                             public IMediaStreamTrackForRTPReceiverChannelVideo,
                             public IMediaStreamTrackForMediaDevices,
                             public IMediaStreamTrackForRTPMediaEngine,
                             public IWakeDelegate,
                             public zsLib::ITimerDelegate,
                             public IMediaStreamTrackAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaStreamTrack;
      friend interaction IMediaStreamTrackFactory;
      friend interaction IMediaStreamTrackForSettings;
      friend interaction IMediaStreamTrackForRTPSender;
      friend interaction IMediaStreamTrackForRTPSenderChannel;
      friend interaction IMediaStreamTrackForRTPSenderChannelAudio;
      friend interaction IMediaStreamTrackForRTPSenderChannelVideo;
      friend interaction IMediaStreamTrackForRTPReceiver;
      friend interaction IMediaStreamTrackForRTPReceiverChannel;
      friend interaction IMediaStreamTrackForRTPReceiverChannelMediaBase;
      friend interaction IMediaStreamTrackForRTPReceiverChannelAudio;
      friend interaction IMediaStreamTrackForRTPReceiverChannelVideo;
      friend interaction IMediaStreamTrackForMediaDevices;
      friend interaction IMediaStreamTrackForRTPMediaEngine;

      ZS_DECLARE_CLASS_PTR(Transport)
      friend class Transport;
      
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForMediaStreamTrack, UseSender)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForMediaStreamTrack, UseSenderChannel)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForMediaStreamTrack, UseReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForMediaStreamTrack, UseReceiverChannel)

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Constraints, Constraints)

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds)
      
      struct VideoCaptureCapabilityWithDistance
      {
        webrtc::VideoCaptureCapability mCapability;
        FLOAT mDistance;
      };

    public:
      MediaStreamTrack(
                       const make_private &,
                       IMessageQueuePtr queue,
                       Kinds kind,
                       bool remote,
                       TrackConstraintsPtr constraints
                       );

    protected:
      MediaStreamTrack(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~MediaStreamTrack();

      static MediaStreamTrackPtr create(
                                        Kinds kind,
                                        bool remote,
                                        TrackConstraintsPtr constraints
                                        );

      static MediaStreamTrackPtr convert(IMediaStreamTrackPtr object);
      static MediaStreamTrackPtr convert(ForSettingsPtr object);
      static MediaStreamTrackPtr convert(ForSenderPtr object);
      static MediaStreamTrackPtr convert(ForSenderChannelPtr object);
      static MediaStreamTrackPtr convert(ForSenderChannelMediaBasePtr object);
      static MediaStreamTrackPtr convert(ForSenderChannelAudioPtr object);
      static MediaStreamTrackPtr convert(ForSenderChannelVideoPtr object);
      static MediaStreamTrackPtr convert(ForReceiverPtr object);
      static MediaStreamTrackPtr convert(ForReceiverChannelPtr object);
      static MediaStreamTrackPtr convert(ForReceiverChannelMediaBasePtr object);
      static MediaStreamTrackPtr convert(ForReceiverChannelAudioPtr object);
      static MediaStreamTrackPtr convert(ForReceiverChannelVideoPtr object);
      static MediaStreamTrackPtr convert(ForMediaDevicesPtr object);
      static MediaStreamTrackPtr convert(ForMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrack
      #pragma mark

      static ElementPtr toDebug(MediaStreamTrackPtr object);

      virtual PUID getID() const override {return mID;}

      virtual IMediaStreamTrackSubscriptionPtr subscribe(IMediaStreamTrackDelegatePtr delegate) override;

      virtual Kinds kind() const override;
      virtual String id() const override;
      virtual String deviceID() const override;
      virtual String label() const override;
      virtual bool enabled() const override;
      virtual void enabled(bool enabled) override;
      virtual bool muted() const override;
      virtual void muted(bool muted) override;
      virtual bool readOnly() const override;
      virtual bool remote() const override;
      virtual States readyState() const override;

      virtual IMediaStreamTrackPtr clone() const override;

      virtual void stop() override;

      virtual CapabilitiesPtr getCapabilities() const override;
      virtual TrackConstraintsPtr getConstraints() const override;
      virtual SettingsPtr getSettings() const override;

      virtual PromisePtr applyConstraints(const TrackConstraints &constraints) override;

      virtual void setMediaElement(void* element) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForSettings
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSender
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      // (duplicate) virtual Kinds kind() const = 0;

      virtual void setSender(IRTPSenderPtr sender) override;

      virtual void notifyAttachSenderChannel(RTPSenderChannelPtr channel) override;
      virtual void notifyDetachSenderChannel(RTPSenderChannelPtr channel) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannelMediaBase
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannelAudio
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannelVideo
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiver
      #pragma mark

      static MediaStreamTrackPtr create(Kinds kind);

      // (duplicate) virtual PUID getID() const = 0;

      virtual void setReceiver(IRTPReceiverPtr receiver) override;

      virtual void notifyActiveReceiverChannel(RTPReceiverChannelPtr channel) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelMediaBase
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelAudio
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelVideo
      #pragma mark

      virtual void renderVideoFrame(const webrtc::VideoFrame& videoFrame) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaDevices
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaEngine
      #pragma mark

      virtual webrtc::AudioDeviceModule* getAudioDeviceModule() override;

      virtual void start() override;
      // (duplicate) virtual void stop() = 0;

      // (duplicate) virtual void renderVideoFrame(const webrtc::VideoFrame& videoFrame) = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackAsyncDelegate
      #pragma mark

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;

      virtual void onApplyConstraints(
                                      PromisePtr promise,
                                      TrackConstraintsPtr constraints
                                      ) override;

      virtual void onSetActiveReceiverChannel(UseReceiverChannelPtr channel) override;

      virtual void onAttachSenderChannel(UseSenderChannelPtr channel) override;
      virtual void onDetachSenderChannel(UseSenderChannelPtr channel) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => friend Transport (video)
      #pragma mark

      virtual void OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame);

      virtual void OnCaptureDelayChanged(const int32_t id, const int32_t delay);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => friend Transport (audio)
      #pragma mark

      virtual int32_t RecordedDataIsAvailable(
                                              const void* audioSamples,
                                              const size_t nSamples,
                                              const size_t nBytesPerSample,
                                              const uint8_t nChannels,
                                              const uint32_t samplesPerSec,
                                              const uint32_t totalDelayMS,
                                              const int32_t clockDrift,
                                              const uint32_t currentMicLevel,
                                              const bool keyPressed,
                                              uint32_t& newMicLevel
                                              );

      virtual int32_t NeedMorePlayData(
                                       const size_t nSamples,
                                       const size_t nBytesPerSample,
                                       const uint8_t nChannels,
                                       const uint32_t samplesPerSec,
                                       void* audioSamples,
                                       size_t& nSamplesOut,
                                       int64_t* elapsed_time_ms,
                                       int64_t* ntp_time_ms
                                       );

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack::Transport
      #pragma mark

      class Transport : public webrtc::VideoCaptureDataCallback,
                        public webrtc::AudioTransport

      {
        struct make_private {};

      protected:
        void init();
        
      public:
        Transport(
                  const make_private &,
                  MediaStreamTrackPtr outer
                  );
        
        ~Transport();
        
        static TransportPtr create(MediaStreamTrackPtr outer);

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrack::Transport => webrtc::VideoCaptureDataCallback
        #pragma mark

        virtual void OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame) override;
        
        virtual void OnCaptureDelayChanged(const int32_t id, const int32_t delay) override;
        
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrack::Transport => webrtc::AudioTransport
        #pragma mark

        virtual int32_t RecordedDataIsAvailable(
                                                const void* audioSamples,
                                                const size_t nSamples,
                                                const size_t nBytesPerSample,
                                                const uint8_t nChannels,
                                                const uint32_t samplesPerSec,
                                                const uint32_t totalDelayMS,
                                                const int32_t clockDrift,
                                                const uint32_t currentMicLevel,
                                                const bool keyPressed,
                                                uint32_t& newMicLevel
                                                ) override;
        
        virtual int32_t NeedMorePlayData(
                                         const size_t nSamples,
                                         const size_t nBytesPerSample,
                                         const uint8_t nChannels,
                                         const uint32_t samplesPerSec,
                                         void* audioSamples,
                                         size_t& nSamplesOut,
                                         int64_t* elapsed_time_ms,
                                         int64_t* ntp_time_ms
                                         ) override;

      private:
        TransportWeakPtr mThisWeak;
        MediaStreamTrackWeakPtr mOuter;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (internal)
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
      
      FLOAT calculateSizeDistance(
                                  ConstrainLongRange width,
                                  ConstrainLongRange height,
                                  webrtc::VideoCaptureCapability capability
                                  );
      FLOAT calculateFrameRateDistance(
                                       ConstrainDoubleRange frameRate,
                                       webrtc::VideoCaptureCapability capability
                                       );
      FLOAT calculateAspectRatioDistance(
                                         ConstrainDoubleRange aspectRatio,
                                         webrtc::VideoCaptureCapability capability
                                         );
      FLOAT calculateFormatDistance(webrtc::VideoCaptureCapability capability);
 
    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (data)
      #pragma mark

      AutoPUID mID;
      MediaStreamTrackWeakPtr mThisWeak;
      MediaStreamTrackPtr mGracefulShutdownReference;

      IMediaStreamTrackDelegateSubscriptions mSubscriptions;

      States mCurrentState {State_Live};

      WORD mLastError {};
      String mLastErrorReason;

      Kinds mKind;
      bool mRemote;
      String mDeviceID;

      UseSenderWeakPtr mSender;
      UseSenderChannelWeakPtr mSenderChannel;
      UseReceiverWeakPtr mReceiver;
      UseReceiverChannelWeakPtr mReceiverChannel;
      
      TransportPtr mTransport;  // keep lifetime of webrtc callback separate from this object

      TrackConstraintsPtr mConstraints;
      webrtc::VideoCaptureModule* mVideoCaptureModule;
      //webrtc::VideoRender* mVideoRenderModule;
      webrtc::VideoRenderCallback* mVideoRendererCallback;
      webrtc::AudioDeviceModule* mAudioDeviceModule;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackFactory
    #pragma mark

    interaction IMediaStreamTrackFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints)

      static IMediaStreamTrackFactory &singleton();

      virtual MediaStreamTrackPtr create(
                                         IMediaStreamTrackTypes::Kinds kind,
                                         bool remote,
                                         TrackConstraintsPtr constraints
                                         );
      virtual MediaStreamTrackPtr create(IMediaStreamTrackTypes::Kinds kind);
    };

    class MediaStreamTrackFactory : public IFactory<IMediaStreamTrackFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaStreamTrackAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::PromisePtr, PromisePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::TrackConstraintsPtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaStreamTrackAsyncDelegate::UseReceiverChannelPtr, UseReceiverChannelPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaStreamTrackAsyncDelegate::UseSenderChannelPtr, UseSenderChannelPtr)
ZS_DECLARE_PROXY_METHOD_1(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD_2(onApplyConstraints, PromisePtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_METHOD_1(onSetActiveReceiverChannel, UseReceiverChannelPtr)
ZS_DECLARE_PROXY_METHOD_1(onAttachSenderChannel, UseSenderChannelPtr)
ZS_DECLARE_PROXY_METHOD_1(onDetachSenderChannel, UseSenderChannelPtr)
ZS_DECLARE_PROXY_END()
