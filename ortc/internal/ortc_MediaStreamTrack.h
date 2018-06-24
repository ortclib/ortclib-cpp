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
    //
    // IMediaStreamTrackForMediaStream
    //

    interaction IMediaStreamTrackForMediaStream
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStream, ForMediaStream);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);

      virtual bool isEnded() const noexcept = 0;

      virtual Kinds kind() const noexcept = 0;
      virtual String id() const noexcept = 0;
      virtual void trace(
                         const char *func = NULL,
                         const char *message = NULL
                         ) const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackAsyncDelegate
    //

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
    //
    // MediaStreamTrack
    //
    
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

      static const char *toString(MediaStreamTrackTypes type) noexcept;
      MediaStreamTrackTypes toMediaStreamTrackType(const char *inputStr) noexcept(false);

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
                       ) noexcept;

    protected:
      MediaStreamTrack(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~MediaStreamTrack() noexcept;

      static MediaStreamTrackPtr convert(ortc::IMediaStreamTrackPtr object) noexcept;
      static MediaStreamTrackPtr convert(internal::IMediaStreamTrackPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaStreamPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaStreamTrackChannelPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSubscriberPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSubscriberMediaPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSubscriberRTPPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaDevicesPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForMediaStreamTrackSelectorPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForReceiverPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForReceiverChannelPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForSenderPtr object) noexcept;
      static MediaStreamTrackPtr convert(ForSenderChannelPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => ortc::IMediaStreamTrack
      //

      PUID getID() const noexcept override {return id_;}

      void trace(
                 const char *func = NULL,
                 const char *message = NULL
                 ) const noexcept override;

      IMediaStreamTrackSubscriptionPtr subscribe(IMediaStreamTrackDelegatePtr delegate) noexcept override;

      Kinds kind() const noexcept override;
      String id() const noexcept override;
      String deviceID() const noexcept override;
      String label() const noexcept override;
      bool enabled() const noexcept override;
      void enabled(bool enabled) noexcept override;
      bool muted() const noexcept override;
      void muted(bool muted) noexcept override;
      bool remote() const noexcept override;
      States readyState() const noexcept override;

      ortc::IMediaStreamTrackPtr clone() const noexcept override;

      void stop() noexcept override;

      CapabilitiesPtr getCapabilities() const noexcept override;
      TrackConstraintsPtr getConstraints() const noexcept override;
      SettingsPtr getSettings() const noexcept override;

      PromisePtr applyConstraints(const TrackConstraints &constraints) noexcept override;

      IMediaStreamTrackMediaSubscriptionPtr subscribeMedia(IMediaStreamTrackSyncMediaDelegatePtr delegate) noexcept override;

      IMediaStreamTrackMediaSubscriptionPtr subscribeMedia(IMediaStreamTrackAsyncMediaDelegatePtr delegate) noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => internal::IMediaStreamTrack
      //
      
      virtual IMediaStreamTrackRTPSubscriptionPtr subscribeRTP(
                                                               const Parameters &rtpEncodingParams,
                                                               IMediaStreamTrackRTPDelegatePtr delegate
                                                               ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForSettings
      //

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaStream
      //

      bool isEnded() const noexcept override;
      // (duplicate) virtual Kinds kind() const noexcept = 0;
      // (duplicate) virtual String id() const noexcept= 0;
      // (duplicate) virtual ElementPtr toDebug() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackChannel
      //

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriber
      //

      void notifySubscriberCancelled(UseSubscriberPtr subscriber) noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriberMedia
      //

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriberRTP
      //

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaDevices
      //

      // (duplicate) virtual PUID getID() const = 0;

      static ForMediaDevicesPtr createForMediaDevices(
                                                      IMediaStreamTrackTypes::Kinds kind,
                                                      const TrackConstraints &constraints
                                                      ) noexcept;
      
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSelector
      //

      // (duplicate) virtual PUID getID() const = 0;

      static ForMediaDevicesPtr createForMediaStreamTrackSelector(IMediaStreamTrackTypes::Kinds kind) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForRTPReceiver
      //

      static ForReceiverPtr createForReceiver(IMediaStreamTrackTypes::Kinds kind) noexcept;

      // (duplicate) virtual PUID getID() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
      //

      // (duplicate) virtual PUID getID() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForRTPSender
      //
      
      // (duplicate) virtual PUID getID() const noexcept = 0;
      // (duplicate) virtual Kinds kind() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
      //
      
      // (duplicate) virtual PUID getID() const noexcept = 0;
      // (duplicate) virtual Kinds kind() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => IMediaStreamTrackAsyncDelegate
      //

      void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats) override;

      void onApplyConstraints(
                              PromisePtr promise,
                              TrackConstraintsPtr constraints
                              ) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => (internal)
      //

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      bool stepDevicePromise() noexcept;
      bool stepSetupDevice() noexcept;

      void cancel() noexcept;

      void setState(States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrack => (data)
      //

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
    //
    // IMediaStreamTrackFactory
    //

    interaction IMediaStreamTrackFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaDevices, ForMediaDevices);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, ForReceiver);

      static IMediaStreamTrackFactory &singleton() noexcept;

      virtual ForMediaDevicesPtr createForMediaDevices(
                                                       IMediaStreamTrackTypes::Kinds kind,
                                                       const TrackConstraints &constraints
                                                       ) noexcept;

      virtual ForReceiverPtr createForReceiver(IMediaStreamTrackTypes::Kinds kind) noexcept;

    };

    class MediaStreamTrackFactory : public IFactory<IMediaStreamTrackFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaStreamTrackAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsReportTypes::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::PromisePtr, PromisePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::TrackConstraintsPtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_METHOD(onResolveStatsPromise, PromiseWithStatsReportPtr, StatsTypeSet)
ZS_DECLARE_PROXY_METHOD(onApplyConstraints, PromisePtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_END()
