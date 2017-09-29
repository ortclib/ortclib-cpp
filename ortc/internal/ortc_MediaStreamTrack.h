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

#include <ortc/internal/ortc_IMediaDeviceCapture.h>
#include <ortc/internal/ortc_IMediaStreamTrack.h>
#include <ortc/internal/ortc_IMediaStreamTrackChannel.h>
#include <ortc/internal/ortc_IMediaStreamTrackSubscriber.h>

#include <ortc/IRTPTypes.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

#include <zsLib/WeightedMovingAverage.h>

#include <unordered_map>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForSettings);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaStreamTrack);

    ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackAsyncDelegate);

    ZS_DECLARE_INTERACTION_PTR(IMediaChannelForMediaStreamTrack);

    using zsLib::FLOAT;
    using zsLib::INT;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaStream
    #pragma mark

    interaction IMediaStreamTrackForMediaStream
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStream, ForMediaStream);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);

      virtual bool isEnded() const = 0;

      virtual Kinds kind() const = 0;
      virtual String id() const = 0;
      virtual void trace(const char *message) const = 0;
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
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats) = 0;

      virtual void onApplyConstraints(
                                      PromisePtr promise,
                                      TrackConstraintsPtr constraints
                                      ) = 0;
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
                             public internal::IMediaStreamTrack,
                             public IMediaStreamTrackForMediaStream,
                             public IMediaStreamTrackForMediaStreamTrackChannel,
                             public IMediaStreamTrackForMediaStreamTrackSubscriberMedia,
                             public IMediaStreamTrackForMediaStreamTrackSubscriberRTP,
                             public IMediaStreamTrackForMediaDevices,
                             public IMediaStreamTrackForMediaStreamTrackSelector,
                             public IMediaStreamTrackForRTPReceiver,
                             public IMediaStreamTrackForRTPReceiverChannel,
                             public IMediaStreamTrackForRTPSender,
                             public IMediaStreamTrackForRTPSenderChannel,
                             public IWakeDelegate,
                             public zsLib::ITimerDelegate,
                             public zsLib::IPromiseSettledDelegate,
                             public IMediaStreamTrackAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ortc::IMediaStreamTrack;
      friend interaction internal::IMediaStreamTrack;
      friend interaction IMediaStreamTrackFactory;
      friend interaction IMediaStreamTrackForMediaStream;
      friend interaction IMediaStreamTrackForMediaStreamTrackChannel;
      friend interaction IMediaStreamTrackForMediaStreamTrackSubscriberMedia;
      friend interaction IMediaStreamTrackForMediaStreamTrackSubscriberRTP;
      friend interaction IMediaStreamTrackForMediaDevices;
      friend interaction IMediaStreamTrackForMediaStreamTrackSelector;
      friend interaction IMediaStreamTrackForRTPReceiver;
      friend interaction IMediaStreamTrackForRTPReceiverChannel;
      friend interaction IMediaStreamTrackForRTPSender;
      friend interaction IMediaStreamTrackForRTPSenderChannel;

      enum MediaStreamTrackTypes
      {
        MediaStreamTrackType_First,

        MediaStreamTrackType_Capture = MediaStreamTrackType_First,
        MediaStreamTrackType_Receiver,
        MediaStreamTrackType_Selector,

        MediaStreamTrackType_Last = MediaStreamTrackType_Selector,
      };

      static const char *toString(MediaStreamTrackTypes type);
      MediaStreamTrackTypes toMediaStreamTrackType(const char *inputStr);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaStreamTrack, UseMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Constraints, Constraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Settings, Settings);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrack, UseChannel);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberForMediaStreamTrack, UseSubscriber);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberMediaForMediaStreamTrack, UseSubscriberMedia);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrack, UseSubscriberRTP);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceCapture::MediaDeviceCapturePromise, MediaDeviceCapturePromise);

      typedef std::unordered_map<PUID, UseSubscriberPtr> SubscriberMap;
      ZS_DECLARE_PTR(SubscriberMap);

      typedef std::unordered_map<MediaChannelID, UseChannelPtr> ChannelMap;
      ZS_DECLARE_PTR(ChannelMap);

    public:
      struct MediaStreamTrackArguments
      {
        IMessageQueuePtr queue_;
        Kinds kind_;
        MediaStreamTrackTypes type_ {MediaStreamTrackType_First};
        TrackConstraintsPtr constraints_;
      };

      MediaStreamTrack(
                       const make_private &,
                       const MediaStreamTrackArguments &args
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

      static MediaStreamTrackPtr convert(ortc::IMediaStreamTrackPtr object);
      static MediaStreamTrackPtr convert(internal::IMediaStreamTrackPtr object);
      static MediaStreamTrackPtr convert(ForMediaStreamPtr object);
      static MediaStreamTrackPtr convert(ForMediaStreamTrackChannelPtr object);
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSubscriberPtr object);
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSubscriberMediaPtr object);
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSubscriberRTPPtr object);
      static MediaStreamTrackPtr convert(ForMediaDevicesPtr object);
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSelectorPtr object);
      static MediaStreamTrackPtr convert(ForReceiverPtr object);
      static MediaStreamTrackPtr convert(ForReceiverChannelPtr object);
      static MediaStreamTrackPtr convert(ForSenderPtr object);
      static MediaStreamTrackPtr convert(ForSenderChannelPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IStatsProvider
      #pragma mark

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => ortc::IMediaStreamTrack
      #pragma mark

      static void trace(
                        MediaStreamTrackPtr object,
                        const char *message = NULL
                        );

      PUID getID() const override {return id_;}

      IMediaStreamTrackSubscriptionPtr subscribe(IMediaStreamTrackDelegatePtr delegate) override;

      Kinds kind() const override;
      String id() const override;
      String deviceID() const override;
      String label() const override;
      bool enabled() const override;
      void enabled(bool enabled) override;
      bool muted() const override;
      void muted(bool muted) override;
      bool remote() const override;
      States readyState() const override;

      ortc::IMediaStreamTrackPtr clone() const override;

      void stop() override;

      CapabilitiesPtr getCapabilities() const override;
      TrackConstraintsPtr getConstraints() const override;
      SettingsPtr getSettings() const override;

      PromisePtr applyConstraints(const TrackConstraints &constraints) override;

      IMediaStreamTrackMediaSubscriptionPtr subscribeMedia(IMediaStreamTrackSyncMediaDelegatePtr delegate) override;

      IMediaStreamTrackMediaSubscriptionPtr subscribeMedia(IMediaStreamTrackAsyncMediaDelegatePtr delegate) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => internal::IMediaStreamTrack
      #pragma mark
      
      virtual IMediaStreamTrackRTPSubscriptionPtr subscribeRTP(
                                                               const Parameters &rtpEncodingParams,
                                                               IMediaStreamTrackRTPDelegatePtr delegate
                                                               ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForSettings
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStream
      #pragma mark

      bool isEnded() const override;
      // (duplicate) virtual Kinds kind() const = 0;
      // (duplicate) virtual String id() const = 0;
      // (duplicate) virtual ElementPtr toDebug() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackChannel
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriber
      #pragma mark

      void notifySubscriberCancelled(UseSubscriberPtr subscriber) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriberMedia
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriberRTP
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaDevices
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      static ForMediaDevicesPtr createForMediaDevices(
                                                      IMediaStreamTrackTypes::Kinds kind,
                                                      const TrackConstraints &constraints
                                                      );
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSelector
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      static ForMediaDevicesPtr createForMediaStreamTrackSelector(IMediaStreamTrackTypes::Kinds kind);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiver
      #pragma mark

      static ForReceiverPtr createForReceiver(IMediaStreamTrackTypes::Kinds kind);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSender
      #pragma mark
      
      // (duplicate) virtual PUID getID() const = 0;
      // (duplicate) virtual Kinds kind() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
      #pragma mark
      
      // (duplicate) virtual PUID getID() const = 0;
      // (duplicate) virtual Kinds kind() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IWakeDelegate
      #pragma mark

      void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => ITimerDelegate
      #pragma mark

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IPromiseSettledDelegate
      #pragma mark

      void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackAsyncDelegate
      #pragma mark

      void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats) override;

      void onApplyConstraints(
                              PromisePtr promise,
                              TrackConstraintsPtr constraints
                              ) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (internal)
      #pragma mark

      void trace(const char *message) const override;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepDevicePromise();
      bool stepSetupDevice();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (data)
      #pragma mark

      AutoPUID id_;
      MediaStreamTrackWeakPtr thisWeak_;
      MediaStreamTrackPtr gracefulShutdownReference_;

      IMediaStreamTrackDelegateSubscriptions subscriptions_;

      States currentState_ {State_First};

      WORD lastError_ {};
      String lastErrorReason_;

      String trackId_;
      String label_;
      Kinds kind_ {Kind_First};
      bool enabled_ {false};
      bool muted_ {false};
      MediaStreamTrackTypes type_ {MediaStreamTrackType_First};
      String deviceId_;

      CapabilitiesPtr capabilities_;
      TrackConstraintsPtr constraints_;
      SettingsPtr settings_;

      SubscriberMapPtr subscribers_;
      ChannelMapPtr channels_;

      ITimerPtr statsTimer_;
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
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaDevices, ForMediaDevices);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, ForReceiver);

      static IMediaStreamTrackFactory &singleton();

      virtual ForMediaDevicesPtr createForMediaDevices(
                                                       IMediaStreamTrackTypes::Kinds kind,
                                                       const TrackConstraints &constraints
                                                       );

      virtual ForReceiverPtr createForReceiver(IMediaStreamTrackTypes::Kinds kind);

    };

    class MediaStreamTrackFactory : public IFactory<IMediaStreamTrackFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaStreamTrackAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsReportTypes::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::PromisePtr, PromisePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::TrackConstraintsPtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_METHOD_2(onResolveStatsPromise, PromiseWithStatsReportPtr, StatsTypeSet)
ZS_DECLARE_PROXY_METHOD_2(onApplyConstraints, PromisePtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_END()
