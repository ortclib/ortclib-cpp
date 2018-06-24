/*

 Copyright (c) 2017, Hookflash Inc. / Optical Tone Ltd.
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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/platform.h>
#include <ortc/internal/ortc_IMediaStreamTrackMonitor.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/ortc.stats.events.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <limits>
#include <float.h>
#include <math.h>



namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_stream_track) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  using zsLib::SingletonManager;
  using zsLib::DOUBLE;

  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaStreamTrackSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrackSettingsDefaults
    //

    class MediaStreamTrackSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaStreamTrackSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaStreamTrackSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<MediaStreamTrackSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaStreamTrackSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<MediaStreamTrackSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
        //      ISettings::setUInt(ORTC_SETTING_MEDIA_STREAM_TRACK_, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaStreamTrackSettingsDefaults() noexcept
    {
      MediaStreamTrackSettingsDefaults::singleton();
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrack
    //
  
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackForMediaStreamTrackChannel
    //
  
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackForRTPReceiver
    //

    //-------------------------------------------------------------------------
    IMediaStreamTrackForRTPReceiver::ForReceiverPtr IMediaStreamTrackForRTPReceiver::createForReceiver(IMediaStreamTrackTypes::Kinds kind) noexcept
    {
      return IMediaStreamTrackFactory::singleton().createForReceiver(kind);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackForRTPReceiverChannel
    //
  
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackForRTPSender
    //
  
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackForRTPSenderChannel
    //
  
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackForMediaStream
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack
    //


    //-------------------------------------------------------------------------
    const char *MediaStreamTrack::toString(MediaStreamTrackTypes type) noexcept
    {
      switch (type) {
        case MediaStreamTrackType_Capture:   return "capture";
        case MediaStreamTrackType_Receiver:  return "receiver";
        case MediaStreamTrackType_Selector:  return "selector";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    MediaStreamTrack::MediaStreamTrackTypes MediaStreamTrack::toMediaStreamTrackType(const char *inputStr) noexcept(false)
    {
      String str(inputStr);
      for (MediaStreamTrack::MediaStreamTrackTypes index = MediaStreamTrack::MediaStreamTrackType_First; index <= MediaStreamTrack::MediaStreamTrackType_Last; index = static_cast<MediaStreamTrack::MediaStreamTrackTypes>(static_cast<std::underlying_type<MediaStreamTrack::MediaStreamTrackTypes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(MediaStreamTrack::toString(index))) return index;
      }
      ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
      return MediaStreamTrack::MediaStreamTrackType_First;
    }

    //-------------------------------------------------------------------------
    MediaStreamTrack::MediaStreamTrack(
                                       const make_private &,
                                       const MediaStreamTrackArguments &args
                                       ) noexcept :
      MessageQueueAssociator(args.queue_),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      subscriptions_(decltype(subscriptions_)::create()),
      trackId_(String(IMediaStreamTrackTypes::toString(args.kind_)) + "_label_" + string(zsLib::createUUID())),
      kind_(args.kind_),
      type_(args.type_),
      constraints_(args.constraints_)
    {
      ZS_EVENTING_4(x, i, Detail, MediaStreamTrackCreate, ol, MediaEngine, Start, 
        puid, id, id_,
        string, traceId, trackId_,
        string, kind, IMediaStreamTrackTypes::toString(kind_),
        string, type, MediaStreamTrack::toString(type_)
      );
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::init() noexcept
    {
      AutoRecursiveLock lock(*this);

      statsTimer_ = ITimer::create(thisWeak_.lock(), Seconds(1));

      capabilities_ = make_shared<Capabilities>();
      settings_ = make_shared<Settings>();

      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaStreamTrack::~MediaStreamTrack() noexcept
    {
      if (isNoop()) return;

      ZS_EVENTING_1(x, i, Detail, MediaStreamTrackDestroy, ol, MediaEngine, Stop, puid, id, id_);

      thisWeak_.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ortc::IMediaStreamTrackPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }


    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(internal::IMediaStreamTrackPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaStreamPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaStreamTrackChannelPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaStreamTrackSubscriberPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaStreamTrackSubscriberMediaPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaStreamTrackSubscriberRTPPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaDevicesPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverChannelPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderChannelPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrack
    //
    
    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr MediaStreamTrack::getStats(const StatsTypeSet &stats) const noexcept
    {
      if (!stats.hasStatType(IStatsReportTypes::StatsType_Track)) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }
      AutoRecursiveLock lock(*this);

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_EVENTING_2(x, w, Debug, MediaStreamTrackWarning, ol, RtpPacket, Info, puid, id, id_, string, message, "can not fetch stats while shutdown / shutting down");

        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      IMediaStreamTrackAsyncDelegateProxy::create(thisWeak_.lock())->onResolveStatsPromise(promise, stats);
      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrack
    //
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::trace(
                                 const char *func,
                                 const char *message
                                 ) const noexcept
    {
      ZS_EVENTING_17(x, i, Basic, MediaStreamTrackTrace, ol, MediaEngine, Info,
        puid, id, id_,
        string, func, func,
        string, message, message,
        bool, gracefulShutdownReference, (bool)gracefulShutdownReference_,
        string, currentState, ortc::IMediaStreamTrackTypes::toString(currentState_),
        word, lastError, lastError_,
        string, lastErrorReason, lastErrorReason_,
        string, trackId, trackId_,
        string, label, label_,
        string, kind, ortc::IMediaStreamTrackTypes::toString(kind_),
        bool, enabled, enabled_,
        bool, muted, muted_,
        string, type, MediaStreamTrack::toString(type_),
        string, deviceId, deviceId_,
        size_t, subscribers, subscribers_ ? subscribers_->size() : 0,
        size_t, channels, channels_ ? channels_->size() : 0,
        puid, statsTimer, statsTimer_ ? statsTimer_->getID() : 0
      );
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackSubscriptionPtr MediaStreamTrack::subscribe(IMediaStreamTrackDelegatePtr originalDelegate) noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaStreamTrackSubscribe, ol, MediaEngine, Info, puid, id, id_);

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return IMediaStreamTrackSubscriptionPtr();

      IMediaStreamTrackSubscriptionPtr subscription = subscriptions_.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IMediaStreamTrackDelegatePtr delegate = subscriptions_.delegate(subscription, true);

      if (delegate) {
        auto pThis = thisWeak_.lock();

#pragma ZS_BUILD_NOTE("TODO","media stream track notify about missed events")
        if (isShutdown()) {
          delegate->onMediaStreamTrackEnded(pThis);
        }
      }

      if (isShutdown()) {
        subscriptions_.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::Kinds MediaStreamTrack::kind() const noexcept
    {
      return kind_;
    }

    //-------------------------------------------------------------------------
    String MediaStreamTrack::id() const noexcept
    {
      return trackId_;
    }

    //-------------------------------------------------------------------------
    String MediaStreamTrack::deviceID() const noexcept
    {
      AutoRecursiveLock lock(*this);

      return deviceId_;
    }


    //-------------------------------------------------------------------------
    String MediaStreamTrack::label() const noexcept
    {
      return label_;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::enabled() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return enabled_;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::enabled(bool enabled) noexcept
    {
      AutoRecursiveLock lock(*this);
      enabled_ = enabled;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::muted() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return muted_;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::muted(bool muted) noexcept
    {
      AutoRecursiveLock lock(*this);
      muted_ = muted;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::remote() const noexcept
    {
      return type_ == MediaStreamTrackType_Receiver;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::States MediaStreamTrack::readyState() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    ortc::IMediaStreamTrackPtr MediaStreamTrack::clone() const noexcept
    {
#pragma ZS_BUILD_NOTE("TODO","media stream track implement clone")
      return IMediaStreamTrackPtr();
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::stop() noexcept
    {
      AutoRecursiveLock lock(*this);
#pragma ZS_BUILD_NOTE("TODO","media stream track implement stop")
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::CapabilitiesPtr MediaStreamTrack::getCapabilities() const noexcept
    {
      return capabilities_;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::TrackConstraintsPtr MediaStreamTrack::getConstraints() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return constraints_;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::SettingsPtr MediaStreamTrack::getSettings() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return settings_;
    }

    //-------------------------------------------------------------------------
    PromisePtr MediaStreamTrack::applyConstraints(const TrackConstraints &inConstraints) noexcept
    {
      PromisePtr promise = Promise::createRejected(IORTCForInternal::queueDelegate());

      auto constraints = TrackConstraints::create(inConstraints);

      IMediaStreamTrackAsyncDelegateProxy::create(thisWeak_.lock())->onApplyConstraints(promise, constraints);

      return promise;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackMediaSubscriptionPtr MediaStreamTrack::subscribeMedia(IMediaStreamTrackSyncMediaDelegatePtr delegate) noexcept
    {
      return IMediaStreamTrackMediaSubscriptionPtr();
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackMediaSubscriptionPtr MediaStreamTrack::subscribeMedia(IMediaStreamTrackAsyncMediaDelegatePtr delegate) noexcept
    {
      return IMediaStreamTrackMediaSubscriptionPtr();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => internal::IMediaStreamTrack
    //


    //-------------------------------------------------------------------------
    IMediaStreamTrackRTPSubscriptionPtr MediaStreamTrack::subscribeRTP(
                                                                       ZS_MAYBE_USED() const Parameters &rtpEncodingParams,
                                                                       ZS_MAYBE_USED() IMediaStreamTrackRTPDelegatePtr delegate
                                                                       ) noexcept
    {
      ZS_MAYBE_USED(rtpEncodingParams);
      ZS_MAYBE_USED(delegate);
#pragma ZS_BUILD_NOTE("TODO","remove ZS_MAYBE_USED when implemented")
      return IMediaStreamTrackRTPSubscriptionPtr();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForSettings
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaStream
    //

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isEnded() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return isShutdown();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackChannel
    //

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriber
    //
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::notifySubscriberCancelled(UseSubscriberPtr subscriber) noexcept
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriberMedia
    //
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSubscriberRTP
    //

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaDevices
    //


    //-------------------------------------------------------------------------
    MediaStreamTrack::ForMediaDevicesPtr MediaStreamTrack::createForMediaDevices(
                                                                                 IMediaStreamTrackTypes::Kinds kind,
                                                                                 const TrackConstraints &constraints
                                                                                 ) noexcept
    {
      MediaStreamTrackArguments args;
      args.queue_ = IORTCForInternal::queueORTC();
      args.kind_ = kind;
      args.type_ = MediaStreamTrackType_Capture;
      args.constraints_ = make_shared<TrackConstraints>(constraints);

      MediaStreamTrackPtr pThis(make_shared<MediaStreamTrack>(make_private{}, args));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForMediaStreamTrackSelector
    //

    //-------------------------------------------------------------------------
    MediaStreamTrack::ForMediaDevicesPtr MediaStreamTrack::createForMediaStreamTrackSelector(ZS_MAYBE_USED() IMediaStreamTrackTypes::Kinds kind) noexcept
    {
      ZS_MAYBE_USED(kind);
#pragma ZS_BUILD_NOTE("TODO","Remove ZS_MAYBE_USED once implemented")
      return ForMediaDevicesPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForRTPReceiver
    //

    //-------------------------------------------------------------------------
    MediaStreamTrack::ForReceiverPtr MediaStreamTrack::createForReceiver(IMediaStreamTrackTypes::Kinds kind) noexcept
    {
      MediaStreamTrackArguments args;
      args.queue_ = IORTCForInternal::queueORTC();
      args.kind_ = kind;
      args.type_ = MediaStreamTrackType_Receiver;

      MediaStreamTrackPtr pThis(make_shared<MediaStreamTrack>(make_private{}, args));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForRTPSender
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onWake()
    {
      ZS_EVENTING_1(x, i, Debug, MediaStreamTrackOnWake, ol, MediaEngine, InternalEvent, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => ITimerDelegate
    //

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onTimer(ITimerPtr timer)
    {
      ZS_EVENTING_2(x, i, Debug, MediaStreamTrackOnTimer, ol, MediaEngine, InternalEvent,
        puid, id, id_,
        puid, timerId, timer->getID()
      );

      AutoRecursiveLock lock(*this);
      if (statsTimer_) {
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IPromiseSettledDelegate
    //

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Debug, MediaStreamTrackOnPromiseSettled, ol, MediaEngine, InternalEvent,
        puid, id, id_,
        puid, promiseId, promise->getID()
      );

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => IMediaStreamTrackAsyncDelegate
    //

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      AutoRecursiveLock lock(*this);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onApplyConstraints(
                                              PromisePtr promise,
                                              TrackConstraintsPtr constraints
                                              )
    {
      AutoRecursiveLock lock(*this);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrack => (internal)
    //

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isShuttingDown() const noexcept
    {
      if (gracefulShutdownReference_) return true;
      return State_Ended == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isShutdown() const noexcept
    {
      if (gracefulShutdownReference_) return false;
      return State_Ended == currentState_;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::step() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, MediaStreamTrackStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_2(x, i, Debug, MediaStreamTrackStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "step forwarding to cancel");
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepDevicePromise()) goto not_ready;
      if (!stepSetupDevice()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Debug, MediaStreamTrackStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Debug, MediaStreamTrackStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
      }
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::stepDevicePromise() noexcept
    {
      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::stepSetupDevice() noexcept
    {
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::cancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      //.......................................................................
      // final cleanup

      setState(State_Ended);

      subscriptions_.clear();
      
      if (statsTimer_) {
        statsTimer_->cancel();
        statsTimer_.reset();
      }

      // make sure to cleanup any final reference to self
      gracefulShutdownReference_.reset();
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setState(States state) noexcept
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Debug, MediaStreamTrackState, ol, MediaEngine, StateEvent, 
        puid, id, id_,
        string, newState, ortc::IMediaStreamTrack::toString(state),
        string, oldState, ortc::IMediaStreamTrack::toString(currentState_)
      );

      currentState_ = state;

//      MediaStreamTrackPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaStreamTrackStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setError(WORD errorCode, const char *inReason) noexcept
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, w, Debug, MediaStreamTrackErrorAgain, ol, MediaEngine, ErrorEvent,
          puid, id, id_,
          word, error, errorCode,
          string, reason, reason
        );
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaStreamTrackError, ol, MediaEngine, ErrorEvent,
        puid, id, id_,
        word, error, errorCode,
        string, reason, reason
      );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackFactory
    //

    //-------------------------------------------------------------------------
    IMediaStreamTrackFactory &IMediaStreamTrackFactory::singleton() noexcept
    {
      return MediaStreamTrackFactory::singleton();
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackFactory::ForMediaDevicesPtr IMediaStreamTrackFactory::createForMediaDevices(
                                                                                                 IMediaStreamTrackTypes::Kinds kind,
                                                                                                 const TrackConstraints &constraints
                                                                                                 ) noexcept
    {      
      if (this) {}
      return internal::MediaStreamTrack::createForMediaDevices(kind, constraints);
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackFactory::ForReceiverPtr IMediaStreamTrackFactory::createForReceiver(IMediaStreamTrackTypes::Kinds kind) noexcept
    {
      if (this) {}
      return internal::MediaStreamTrack::createForReceiver(kind);
    }

  } // internal namespace

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // (helpers)
  //

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message) noexcept
  {
    return Log::Params(message, "ortc::IMediaStreamTrackTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes
  //

  //---------------------------------------------------------------------------
  const char *IMediaStreamTrackTypes::toString(Kinds kind) noexcept
  {
    switch (kind) {
      case Kind_Audio:   return "audio";
      case Kind_Video:   return "video";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Kinds IMediaStreamTrackTypes::toKind(const char *inputStr) noexcept(false)
  {
    String str(inputStr);
    for (IMediaStreamTrackTypes::Kinds index = IMediaStreamTrackTypes::Kind_First; index <= IMediaStreamTrackTypes::Kind_Last; index = static_cast<IMediaStreamTrackTypes::Kinds>(static_cast<std::underlying_type<IMediaStreamTrackTypes::Kinds>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IMediaStreamTrackTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return IMediaStreamTrackTypes::Kind_First;
  }

  //---------------------------------------------------------------------------
  const char *IMediaStreamTrackTypes::toString(States state) noexcept
  {
    switch (state) {
      case State_Live:    return "live";
      case State_Ended:   return "ended";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::States IMediaStreamTrackTypes::toState(const char *inputStr) noexcept(false)
  {
    String str(inputStr);
    for (IMediaStreamTrackTypes::States index = IMediaStreamTrackTypes::State_First; index <= IMediaStreamTrackTypes::State_Last; index = static_cast<IMediaStreamTrackTypes::States>(static_cast<std::underlying_type<IMediaStreamTrackTypes::States>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IMediaStreamTrackTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return IMediaStreamTrackTypes::State_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes::Capabilities
  //

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Capabilities::Capabilities(ElementPtr elem) noexcept
  {
    if (!elem) return;

    {
      ElementPtr entryEl = elem->findFirstChildElement("width");
      if (entryEl) {
        mWidth = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("height");
      if (entryEl) {
        mHeight = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("aspectRatio");
      if (entryEl) {
        mAspectRatio = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("frameRate");
      if (entryEl) {
        mFrameRate = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("facingMode");
      if (entryEl) {
        mFacingMode = CapabilityString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("orientation");
      if (entryEl) {
        mOrientation = CapabilityString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("volume");
      if (entryEl) {
        mVolume = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleRate");
      if (entryEl) {
        mSampleRate = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleSize");
      if (entryEl) {
        mSampleSize = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("echoCancellation");
      if (entryEl) {
        mEchoCancellation = CapabilityBoolean(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("latency");
      if (entryEl) {
        mLatency = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("channelCount");
      if (entryEl) {
        mChannelCount = CapabilityLong(entryEl);
      }
    }

    IHelper::getElementValue(elem, "deviceId", "ortc::IMediaStreamTrackTypes::Capabilities", mDeviceID);
    IHelper::getElementValue(elem, "groupId", "ortc::IMediaStreamTrackTypes::Capabilities", mGroupID);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Capabilities::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    if (mWidth.hasValue()) {
      elem->adoptAsLastChild(mWidth.value().createElement("width"));
    }
    if (mHeight.hasValue()) {
      elem->adoptAsLastChild(mHeight.value().createElement("height"));
    }
    if (mAspectRatio.hasValue()) {
      elem->adoptAsLastChild(mAspectRatio.value().createElement("aspectRatio"));
    }
    if (mFrameRate.hasValue()) {
      elem->adoptAsLastChild(mFrameRate.value().createElement("frameRate"));
    }
    if (mFacingMode.hasValue()) {
      elem->adoptAsLastChild(mFacingMode.value().createElement("facingMode"));
    }
    if (mOrientation.hasValue()) {
      elem->adoptAsLastChild(mOrientation.value().createElement("orientation"));
    }
    if (mVolume.hasValue()) {
      elem->adoptAsLastChild(mVolume.value().createElement("volume"));
    }
    if (mSampleRate.hasValue()) {
      elem->adoptAsLastChild(mSampleRate.value().createElement("sampleRate"));
    }
    if (mSampleSize.hasValue()) {
      elem->adoptAsLastChild(mSampleSize.value().createElement("sampleSize"));
    }
    if (mEchoCancellation.hasValue()) {
      elem->adoptAsLastChild(mEchoCancellation.value().createElement("echoCancellation"));
    }
    if (mLatency.hasValue()) {
      elem->adoptAsLastChild(mLatency.value().createElement("latency"));
    }
    if (mChannelCount.hasValue()) {
      elem->adoptAsLastChild(mChannelCount.value().createElement("channelCount"));
    }

    IHelper::adoptElementValue(elem, "deviceId", mDeviceID, false);
    IHelper::adoptElementValue(elem, "groupId", mGroupID, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::create() noexcept
  {
    return make_shared<Capabilities>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::create(const CapabilitiesPtr &value) noexcept
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::create(const Capabilities &value) noexcept
  {
    return make_shared<Capabilities>(value);
  }
  
  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(Capabilities, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Capabilities::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Capabilities");

    IHelper::debugAppend(resultEl, "width", mWidth.hasValue() ? mWidth.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "height", mHeight.hasValue() ? mHeight.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.hasValue() ? mAspectRatio.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate.hasValue() ? mFrameRate.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "facingMode", mFacingMode.hasValue() ? mFacingMode.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "volume", mVolume.hasValue() ? mVolume.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate.hasValue() ? mSampleRate.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.hasValue() ? mEchoCancellation.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "latency", mLatency.hasValue() ? mLatency.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "channel acount", mChannelCount.hasValue() ? mChannelCount.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "device id", mDeviceID);
    IHelper::debugAppend(resultEl, "group id", mGroupID);
    IHelper::debugAppend(resultEl, "encodingFormat", mEncodingFormat.hasValue() ? mEncodingFormat.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Capabilities::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::Capabilities:");
    hasher->update(mWidth.hasValue() ? mWidth.value().hash() : String());
    hasher->update(":");
    hasher->update(mHeight.hasValue() ? mHeight.value().hash() : String());
    hasher->update(":");
    hasher->update(mAspectRatio.hasValue() ? mAspectRatio.value().hash() : String());
    hasher->update(":");
    hasher->update(mFrameRate.hasValue() ? mFrameRate.value().hash() : String());
    hasher->update(":");
    hasher->update(mFacingMode.hasValue() ? mFacingMode.value().hash() : String());
    hasher->update(":");
    hasher->update(mVolume.hasValue() ? mVolume.value().hash() : String());
    hasher->update(":");
    hasher->update(mSampleRate.hasValue() ? mSampleRate.value().hash() : String());
    hasher->update(":");
    hasher->update(mEchoCancellation.hasValue() ? mEchoCancellation.value().hash()  : String());
    hasher->update(":");
    hasher->update(mLatency.hasValue() ? mLatency.value().hash()  : String());
    hasher->update(":");
    hasher->update(mChannelCount.hasValue() ? mChannelCount.value().hash() : String());
    hasher->update(":");
    hasher->update(mDeviceID);
    hasher->update(":");
    hasher->update(mGroupID);
    hasher->update(":");
    hasher->update(mEncodingFormat.hasValue() ? mEncodingFormat.value().hash() : String());

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes::Settings
  //

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Settings::Settings(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "width", mWidth);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "height", mHeight);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "aspectRatio", mAspectRatio);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "frameRate", mFrameRate);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "facingMode", mFacingMode);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "orientation", mOrientation);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "volume", mVolume);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "sampleRate", mSampleRate);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "sampleSize", mSampleSize);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "echoCancellation", mEchoCancellation);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "latency", mLatency);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "channelCount", mChannelCount);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "deviceId", mDeviceID);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "groupId", mGroupID);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "encodingFormat", mEncodingFormat);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Settings::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "width", mWidth);
    IHelper::adoptElementValue(elem, "height", mHeight);
    IHelper::adoptElementValue(elem, "aspectRatio", mAspectRatio);
    IHelper::adoptElementValue(elem, "frameRate", mFrameRate);
    IHelper::adoptElementValue(elem, "facingMode", mFacingMode);
    IHelper::adoptElementValue(elem, "orientation", mOrientation);
    IHelper::adoptElementValue(elem, "volume", mVolume);
    IHelper::adoptElementValue(elem, "sampleRate", mSampleRate);
    IHelper::adoptElementValue(elem, "sampleSize", mSampleSize);
    IHelper::adoptElementValue(elem, "echoCancellation", mEchoCancellation);
    IHelper::adoptElementValue(elem, "latency", mLatency);
    IHelper::adoptElementValue(elem, "channelCount", mChannelCount);
    IHelper::adoptElementValue(elem, "deviceId", mDeviceID);
    IHelper::adoptElementValue(elem, "groupId", mGroupID);
    IHelper::adoptElementValue(elem, "encodingFormat", mEncodingFormat);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::create() noexcept
  {
    return make_shared<Settings>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::create(const SettingsPtr &value) noexcept
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::create(const Settings &value) noexcept
  {
    return make_shared<Settings>(value);
  }
  
  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(Settings, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Settings::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Settings");

    IHelper::debugAppend(resultEl, "width", mWidth);
    IHelper::debugAppend(resultEl, "height", mHeight);
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio);
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate);
    IHelper::debugAppend(resultEl, "facing mode", mFacingMode);
    IHelper::debugAppend(resultEl, "orientation", mOrientation);
    IHelper::debugAppend(resultEl, "volume", mVolume);
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate);
    IHelper::debugAppend(resultEl, "sample size", mSampleSize);
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation);
    IHelper::debugAppend(resultEl, "latency", mLatency);
    IHelper::debugAppend(resultEl, "channel count", mChannelCount);
    IHelper::debugAppend(resultEl, "device id", mDeviceID);
    IHelper::debugAppend(resultEl, "group id", mGroupID);
    IHelper::debugAppend(resultEl, "encoding format", mEncodingFormat);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Settings::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::Settings:");

    hasher->update(mWidth);
    hasher->update(":");
    hasher->update(mHeight);
    hasher->update(":");
    hasher->update(mAspectRatio);
    hasher->update(":");
    hasher->update(mFrameRate);
    hasher->update(":");
    hasher->update(mFacingMode, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mOrientation, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mVolume);
    hasher->update(":");
    hasher->update(mSampleRate);
    hasher->update(":");
    hasher->update(mSampleSize);
    hasher->update(":");
    hasher->update(mEchoCancellation);
    hasher->update(":");
    hasher->update(mLatency);
    hasher->update(":");
    hasher->update(mChannelCount);
    hasher->update(":");
    hasher->update(mDeviceID, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mGroupID, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mEncodingFormat, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes::ConstraintSet
  //

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintSet::ConstraintSet(ElementPtr elem) noexcept
  {
    if (!elem) return;

    {
      ElementPtr entryEl = elem->findFirstChildElement("width");
      if (entryEl) {
        mWidth = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("height");
      if (entryEl) {
        mHeight = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("aspectRatio");
      if (entryEl) {
        mAspectRatio = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("frameRate");
      if (entryEl) {
        mFrameRate = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("facingMode");
      if (entryEl) {
        mFacingMode = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("orientation");
      if (entryEl) {
        mOrientation = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("volume");
      if (entryEl) {
        mVolume = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleRate");
      if (entryEl) {
        mSampleRate = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleSize");
      if (entryEl) {
        mSampleSize = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("echoCancellation");
      if (entryEl) {
        mEchoCancellation = ConstrainBoolean(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("latency");
      if (entryEl) {
        mLatency = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("channelCount");
      if (entryEl) {
        mChannelCount = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("deviceId");
      if (entryEl) {
        mDeviceID = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("groupId");
      if (entryEl) {
        mGroupID = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("encodingFormat");
      if (entryEl) {
        mEncodingFormat = ConstrainString(entryEl);
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::ConstraintSet::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    elem->adoptAsLastChild(mWidth.createElement("width"));
    elem->adoptAsLastChild(mHeight.createElement("height"));
    elem->adoptAsLastChild(mAspectRatio.createElement("aspectRatio"));
    elem->adoptAsLastChild(mFrameRate.createElement("frameRate"));
    elem->adoptAsLastChild(mFacingMode.createElement("facingMode"));
    elem->adoptAsLastChild(mOrientation.createElement("orientation"));
    elem->adoptAsLastChild(mVolume.createElement("volume"));
    elem->adoptAsLastChild(mSampleRate.createElement("sampleRate"));
    elem->adoptAsLastChild(mSampleSize.createElement("sampleSize"));
    elem->adoptAsLastChild(mEchoCancellation.createElement("echoCancellation"));
    elem->adoptAsLastChild(mLatency.createElement("latency"));
    elem->adoptAsLastChild(mChannelCount.createElement("channelCount"));
    elem->adoptAsLastChild(mDeviceID.createElement("deviceId"));
    elem->adoptAsLastChild(mGroupID.createElement("groupId"));
    elem->adoptAsLastChild(mEncodingFormat.createElement("encodingFormat"));

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintSetPtr IMediaStreamTrackTypes::ConstraintSet::create() noexcept
  {
    return make_shared<ConstraintSet>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintSetPtr IMediaStreamTrackTypes::ConstraintSet::create(const ConstraintSetPtr &value) noexcept
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes:: ConstraintSetPtr IMediaStreamTrackTypes::ConstraintSet::create(const ConstraintSet &value) noexcept
  {
    return make_shared<ConstraintSet>(value);
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::ConstraintSet::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::ConstraintSet");

    IHelper::debugAppend(resultEl, "width", mWidth.toDebug());
    IHelper::debugAppend(resultEl, "height", mHeight.toDebug());
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.toDebug());
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate.toDebug());
    IHelper::debugAppend(resultEl, "facingMode", mFacingMode.toDebug());
    IHelper::debugAppend(resultEl, "volume", mVolume.toDebug());
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate.toDebug());
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.toDebug());
    IHelper::debugAppend(resultEl, "latency", mLatency.toDebug());
    IHelper::debugAppend(resultEl, "channel count", mChannelCount.toDebug());
    IHelper::debugAppend(resultEl, "device id", mDeviceID.toDebug());
    IHelper::debugAppend(resultEl, "group id", mGroupID.toDebug());
    IHelper::debugAppend(resultEl, "encoding format", mEncodingFormat.toDebug());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::ConstraintSet::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::ConstraintSet:");

    hasher->update(mWidth.hash());
    hasher->update(":");
    hasher->update(mHeight.hash());
    hasher->update(":");
    hasher->update(mAspectRatio.hash());
    hasher->update(":");
    hasher->update(mFrameRate.hash());
    hasher->update(":");
    hasher->update(mFacingMode.hash());
    hasher->update(":");
    hasher->update(mVolume.hash());
    hasher->update(":");
    hasher->update(mSampleRate.hash());
    hasher->update(":");
    hasher->update(mEchoCancellation.hash());
    hasher->update(":");
    hasher->update(mLatency.hash());
    hasher->update(":");
    hasher->update(mChannelCount.hash());
    hasher->update(":");
    hasher->update(mDeviceID.hash());
    hasher->update(":");
    hasher->update(mGroupID.hash());
    hasher->update(":");
    hasher->update(mEncodingFormat.hash());

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes::ConstraintSet
  //

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraints::TrackConstraints(ElementPtr elem) noexcept
  {
    if (!elem) return;

    ElementPtr constraintSetsEl = elem->findFirstChildElement("constraintSets");

    ElementPtr constraintSetEl;
    if (constraintSetsEl) {
      constraintSetEl = constraintSetEl->findFirstChildElement("constraintSet");
    } else {
      constraintSetEl = elem->findFirstChildElement("constraintSet");
    }

    while (constraintSetEl) {
      mAdvanced.push_back(make_shared<ConstraintSet>(constraintSetEl));
      constraintSetEl = constraintSetEl->findNextSiblingElement("constraintSet");
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::TrackConstraints::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    ElementPtr constraintSetsEl = Element::create("constraintSets");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto value = (*iter);

      constraintSetsEl->adoptAsLastChild(value->createElement("constraintSet"));
    }

    if (constraintSetsEl->hasChildren()) {
      elem->adoptAsLastChild(constraintSetsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraintsPtr IMediaStreamTrackTypes::TrackConstraints::create() noexcept
  {
    return make_shared<TrackConstraints>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraintsPtr IMediaStreamTrackTypes::TrackConstraints::create(const TrackConstraintsPtr &value) noexcept
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraintsPtr IMediaStreamTrackTypes::TrackConstraints::create(const TrackConstraints &value) noexcept
  {
    auto result = make_shared<TrackConstraints>();

    for (auto iter = value.mAdvanced.begin(); iter != value.mAdvanced.end(); ++iter) {
      auto clone = ConstraintSet::create(*iter);
      result->mAdvanced.push_back(clone);
    }

    return result;
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::TrackConstraints::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::TrackConstraints");

    ElementPtr advancedSet = Element::create("advanced set");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto constraintSet = (*iter);
      IHelper::debugAppend(advancedSet, constraintSet ? constraintSet->toDebug() : ElementPtr());
    }
    if (advancedSet->hasChildren()) {
      IHelper::debugAppend(resultEl, advancedSet);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::TrackConstraints::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::TrackConstraints:");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto constraintSet = (*iter);
      hasher->update(constraintSet ? constraintSet->hash() : String());
      hasher->update(":");
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes::ConstraintSet
  //

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Constraints::Constraints(ElementPtr elem) noexcept
  {
    if (!elem) return;

    {
      ElementPtr entryEl = elem->findFirstChildElement("video");
      if (entryEl) {
        mVideo = make_shared<TrackConstraints>(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("audio");
      if (entryEl) {
        mAudio = make_shared<TrackConstraints>(entryEl);
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Constraints::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    if (mVideo) {
      elem->adoptAsLastChild(mVideo->createElement("video"));
    }
    if (mAudio) {
      elem->adoptAsLastChild(mVideo->createElement("audio"));
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintsPtr IMediaStreamTrackTypes::Constraints::create() noexcept
  {
    return make_shared<Constraints>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintsPtr IMediaStreamTrackTypes::Constraints::create(const ConstraintsPtr &value) noexcept
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintsPtr IMediaStreamTrackTypes::Constraints::create(const Constraints &value) noexcept
  {
    auto result = make_shared<Constraints>();
    result->mVideo = value.mVideo ? TrackConstraints::create(value.mVideo) : TrackConstraintsPtr();
    result->mAudio = value.mAudio ? TrackConstraints::create(value.mAudio) : TrackConstraintsPtr();
    return result;
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Constraints::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Constraints");

    IHelper::debugAppend(resultEl, "video", mVideo ? mVideo->toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "audio", mAudio ? mAudio->toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Constraints::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::Constraints:");

    hasher->update(mVideo ? mVideo->hash() : String());
    hasher->update(":");
    hasher->update(mAudio ? mAudio->hash() : String());

    return hasher->finalizeAsString();
  }
  

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrack
  //

  //---------------------------------------------------------------------------
  IMediaStreamTrackPtr IMediaStreamTrack::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(IMediaStreamTrack, any);
  }

}
