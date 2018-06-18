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

#include <ortc/internal/ortc_MediaDeviceCaptureVideo.h>
#include <ortc/internal/ortc_MediaEngine.h>
//#include <ortc/internal/ortc_MediaStreamTrack.h>
//#include <ortc/internal/ortc_MediaDeviceCaptureVideo.h>
//#include <ortc/internal/ortc_MediaDeviceCaptureVideo.h>
//#include <ortc/internal/ortc_MediaDeviceRenderVideo.h>
////#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
////#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
////#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
////#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
//#include <ortc/internal/ortc_RTPPacket.h>
//#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>
//
//#include <ortc/IHelper.h>
#include <ortc/services/IHTTP.h>
//
#include <zsLib/ISettings.h>
//#include <zsLib/Singleton.h>
//#include <zsLib/Log.h>
//#include <zsLib/XML.h>
//#include <zsLib/SafeInt.h>
//
//#include <cryptopp/sha.h>
//
//#include <webrtc/base/event_tracer.h>

#if 0
#include <limits>
#include <float.h>
#include <math.h>

#include <ortc/internal/webrtc_pre_include.h>
#include <webrtc/rtc_base/timeutils.h>
#include <webrtc/call/rtc_event_log.h>
#include <webrtc/voice_engine/include/voe_codec.h>
#include <webrtc/voice_engine/include/voe_rtp_rtcp.h>
#include <webrtc/voice_engine/include/voe_network.h>
#include <webrtc/voice_engine/include/voe_hardware.h>
#include <webrtc/system_wrappers/include/cpu_info.h>
#include <webrtc/voice_engine/include/voe_audio_processing.h>
#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h>
#ifdef WINUWP
#include <third_party/h264_winrt/h264_winrt_factory.h>
#endif
#include <ortc/internal/webrtc_post_include.h>
#endif //0


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_engine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaDeviceCaptureVideoSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo (helpers)
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideoSettingsDefaults
    //

    class MediaDeviceCaptureVideoSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaDeviceCaptureVideoSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaDeviceCaptureVideoSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<MediaDeviceCaptureVideoSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaDeviceCaptureVideoSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<MediaDeviceCaptureVideoSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaDeviceCaptureVideoSettingsDefaults() noexcept
    {
      MediaDeviceCaptureVideoSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDeviceCaptureVideoForMediaEngine
    //

    //-------------------------------------------------------------------------
    IMediaDeviceCaptureVideoForMediaEngine::ForMediaEnginePtr IMediaDeviceCaptureVideoForMediaEngine::create(
                                                                                                             UseMediaEnginePtr mediaEngine,
                                                                                                             const String &deviceID
                                                                                                             ) noexcept
    {
      return internal::IMediaDeviceCaptureVideoFactory::singleton().create(mediaEngine, deviceID);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaEngine
    //
    
    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::MediaDeviceCaptureVideo(
                                                     const make_private &,
                                                     IMessageQueuePtr queue,
                                                     UseMediaEnginePtr mediaEngine,
                                                     const String &deviceID
                                                     ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mediaEngine_(mediaEngine),
      deviceID_(deviceID),
      subscribers_(make_shared<MediaSubscriberMap>())
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureVideoCreate, ol, MediaEngine, Start, puid, id, id_, string, deviceID, deviceID_);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::init() noexcept
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::~MediaDeviceCaptureVideo() noexcept
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    MediaDeviceCaptureVideoPtr MediaDeviceCaptureVideo::create(
                                                               UseMediaEnginePtr mediaEngine,
                                                               const String &deviceID
                                                               ) noexcept
    {
      auto pThis(make_shared<MediaDeviceCaptureVideo>(make_private{}, IORTCForInternal::queueMediaDevices(), mediaEngine, deviceID));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideoPtr MediaDeviceCaptureVideo::convert(ForMediaEnginePtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaDeviceCaptureVideo, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => (for Media)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::notifyMediaStateChanged() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, MediaDeviceCaptureVideoNotifyMediaStateChanged, ol, MediaEngine, Event, puid, id, id_);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::notifyMediaFailure(
                                                     MediaPtr media,
                                                     WORD errorCode,
                                                     const char *inReason
                                                     ) noexcept
    {
      ZS_EVENTING_2(x, e, Debug, MediaDeviceCaptureVideoNotifyMediaFailure, ol, MediaEngine, Event, puid, id, id_, puid, mediaId, media->getID());

      auto pThis = thisWeak_.lock();
      String reason(inReason);
      postClosure([pThis, media, errorCode, reason] {
        AutoRecursiveLock lock(*pThis);

        if (media != pThis->media_) return;

        pThis->setError(errorCode, reason);
        pThis->cancel();
      });
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => (for MediaSubscribers)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::notifySusbcriberGone() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, MediaDeviceCaptureVideoNotifySubscriberGone, ol, MediaEngine, Event, puid, id, id_);

      recheckMode_ = true;
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => IMediaDeviceForMediaEngine
    //

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::isDeviceIdle() noexcept
    {
      AutoRecursiveLock lock(*this);

      if (isShutdown()) return true;

      if (pendingSubscribers_.size() > 0) return false;
      if (subscribers_->size() > 0) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::States MediaDeviceCaptureVideo::getState() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => IMediaDeviceCaptureForMediaEngine
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::mediaDeviceCaptureSubscribe(
                                                              MediaDeviceCapturePromisePtr promise,
                                                              MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                              TrackConstraintsPtr constraints,
                                                              IMediaDeviceCaptureDelegatePtr delegate
                                                              ) noexcept
    {    
      auto pThis = thisWeak_.lock();

      PendingSubscriber info;
      info.promise_ = promise;
      info.repaceExistingDeviceObjectID_ = repaceExistingDeviceObjectID;
      info.constraints_ = constraints;
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingSubscribers_.push_back(info);
        pThis->step();
      });
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => IMediaDeviceCaptureVideoForMediaEngine
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureVideoOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => IPromiseSettledDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);

      if (deviceModesPromise_) {
        if (deviceModesPromise_->getID() == promise->getID()) {
          if (promise->isRejected()) {
            setError(promise);
            cancel();
            return;
          }
        }
      }

      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo => (internal)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::cancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IMediaDevice::State_ShuttingDown);

      deviceModesPromise_.reset();

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

        // perform any graceful asynchronous shutdown processes needed and
        // re-attempt shutdown again later if needed.

        if (!stepShutdownPendingRequests()) return;
        if (!stepShutdownSubscribers()) return;
        if (!stepShutdownMedia()) return;
      }

      //.......................................................................
      // final cleanup (hard shutdown)

      setState(IMediaDevice::State_Shutdown);

      stepShutdownPendingRequests();
      stepShutdownSubscribers();
      stepShutdownMedia();

      auto engine = mediaEngine_.lock();
      if (engine) {
        engine->notifyDeviceIsIdleOrShutdownStateChanged();
      }

      // make sure to cleanup any final reference to self
      gracefulShutdownReference_.reset();

      mediaEngine_.reset();
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepShutdownPendingRequests() noexcept
    {
      ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending requests");

      for (auto iter_doNotUse = pendingSubscribers_.begin(); iter_doNotUse != pendingSubscribers_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingSubscribers_.erase(current);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepShutdownSubscribers() noexcept
    {
      if (subscribers_->size() < 1) return true;

      ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "subscribers");

      for (auto iter_doNotUse = subscribers_->begin(); iter_doNotUse != subscribers_->end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto subscriber = (*current).second;

        subscriber->shutdown();
      }

      // empty the subscriber list
      subscribers_ = make_shared<MediaSubscriberMap>();

      if (media_) {
        media_->notifySubscribersChanged(subscribers_);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepShutdownMedia() noexcept
    {
      ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media");

      if (!media_) return true;

      media_->shutdown();
      bool shutdown = media_->isShutdown();
      if (shutdown) {
        media_.reset();
      }
      return shutdown;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::step() noexcept
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureVideoStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureVideoStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepMediaReinitializationShutdown()) goto reinitializing;
      if (!stepDiscoverModes()) goto not_ready;
      if (!stepFigureOutMode()) goto not_ready;
      if (!stepWaitForMediaDevice()) goto not_ready;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    reinitializing:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "reinitializing");
        setState(IMediaDevice::State_Reinitializing);
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IMediaDevice::State_Active);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepMediaReinitializationShutdown() noexcept
    {
      if (!media_) goto no_media_to_shutdown;

      if (media_->isShuttingDown()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "waiting for media shutdown to reinialize");
        return false;
      }

      if (media_->isShutdown()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media shutdown");
        media_.reset();
        return true;
      }

    no_media_to_shutdown:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no media to shutdown");
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepDiscoverModes() noexcept
    {
      if (!deviceModesPromise_) {
        deviceModesPromise_ = IMediaDevices::enumerateDefaultModes(deviceID_);

        if (!deviceModesPromise_) {
          setError(UseHTTP::HTTPStatusCode_ExpectationFailed, (String("unable to discover modes for an video media device: ") + deviceID_).c_str());
          cancel();
          return false;
        }
        deviceModesPromise_->thenWeak(thisWeak_.lock());
      }

      if (!deviceModesPromise_->isSettled()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "waiting for device modes to be known");
        return false;
      }

      if (deviceModesPromise_->isRejected()) {
        setError(deviceModesPromise_);
        cancel();
        return false;
      }

      auto settingsList = deviceModesPromise_->value();

      if (!settingsList) {
        setError(UseHTTP::HTTPStatusCode_ExpectationFailed, (String("no modes found for video media device: ") + deviceID_).c_str());
        cancel();
        return false;
      }

      ZS_EVENTING_3(x, i, Debug, MediaDeviceCaptureVideoStepModesFound, ol, MediaEngine, Step,
        puid, id, id_,
        string, deviceId, deviceID_,
        size_t, totalModes, settingsList->size());

      // copy modes to shared pointer list of settings
      for (auto iter = settingsList->begin(); iter != settingsList->end(); ++iter)
      {
        deviceModes_.push_back(make_shared<UseSettings>(*iter));
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepFigureOutMode() noexcept
    {
      {
        if (recheckMode_) goto mode_needed;
        if (!media_) goto mode_needed;
        if (pendingSubscribers_.size() > 0) goto mode_needed;
        return true;
      }

    mode_needed:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "figure out mode");

        recheckMode_ = false;

        TrackConstraintsList allConstraints;

        for (auto iter = pendingSubscribers_.begin(); iter != pendingSubscribers_.end(); ++iter)
        {
          auto &info = (*iter);
          allConstraints.push_back(info.constraints_);
        }

        for (auto iter = subscribers_->begin(); iter != subscribers_->end(); ++iter) {
          auto &subscriber = (*iter).second;
          allConstraints.push_back(subscriber->getConstraints());
        }

        auto found = obtainBestMode(allConstraints, deviceModes_);

        bool changed = false;

        MediaSubscriberMapPtr replacement(make_shared<MediaSubscriberMap>());
        for (auto iter = subscribers_->begin(); iter != subscribers_->end(); ++iter) {
          auto &subscriber = (*iter).second;
          if (subscriber->isShutdown()) {
            changed = true;
            continue;
          }
          (*replacement)[(*iter).first] = subscriber;
        }

        if (found) {
          for (auto iter = pendingSubscribers_.begin(); iter != pendingSubscribers_.end(); ++iter) {
            auto &info = (*iter);
            auto subscriber = MediaSubscriber::create(getAssociatedMessageQueue(), thisWeak_.lock(), info.constraints_, info.delegate_);
            info.promise_->resolve(IMediaDeviceCapturePtr(subscriber));
            subscriber->notifyStateChanged(currentState_);
            changed = true;
            (*replacement)[subscriber->getID()] = subscriber;
          }
        } else {
          for (auto iter = pendingSubscribers_.begin(); iter != pendingSubscribers_.end(); ++iter) {
            auto &info = (*iter);
            info.promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "over constrained"));
          }
        }
        pendingSubscribers_.clear();

        if (changed) {
          subscribers_ = replacement;
          if (media_) {
            media_->notifySubscribersChanged(subscribers_);
          }
        }

        if (subscribers_->size() < 1) {
          found = UseSettingsPtr();

          auto engine = mediaEngine_.lock();
          if (engine) {
            engine->notifyDeviceIsIdleOrShutdownStateChanged();
          }
        }

        if (found) {
          if (found == requiredSettings_) {
            ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "optimal device mode has not changed");
            return true;
          }
        }

        if (isReady()) {
          setState(IMediaDevice::State_Reinitializing);
        }

        requiredSettings_ = found;
        if (media_) {
          media_->shutdown();
        }
      }
      return false;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::stepWaitForMediaDevice() noexcept
    {
      if (media_->isShutdown()) {
        media_.reset();
      }

      if (!media_) {
        if (!requiredSettings_) {
          ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no media device mode required");
          return false;
        }
        media_ = Media::create(getAssociatedMessageQueue(), thisWeak_.lock(), deviceID_, requiredSettings_);
        media_->notifySubscribersChanged(subscribers_);
      }

      if (!media_->isReady()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media device is not ready");
        return false;
      }

      ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media device is ready");
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::setState(States state) noexcept
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceCaptureVideoSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IMediaDevice::toString(state), string, oldState, IMediaDevice::toString(currentState_));

      currentState_ = state;

      for (auto iter = subscribers_->begin(); iter != subscribers_->end(); ++iter) {
        auto &subscriber = (*iter).second;
        subscriber->notifyStateChanged(currentState_);
      }

//      MediaEnginePtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaEngineStateChanged(pThis, currentState_);
//      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::setError(PromisePtr promise) noexcept
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::setError(WORD errorCode, const char *inReason) noexcept
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureVideoSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureVideoSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::Media
    //

    //-------------------------------------------------------------------------
    const char *MediaDeviceCaptureVideo::Media::toString(MediaStates state) noexcept
    {
      switch (state)
      {
        case MediaState_Pending:        return "pending";
        case MediaState_Ready:          return "ready";
        case MediaState_ShuttingDown:   return "shutting down";
        case MediaState_Shutdown:       return "shutdown";
      }
      return "UNKNOWN";
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::Media::Media(
                                          const make_private &,
                                          IMessageQueuePtr queue,
                                          UseOuterPtr outer,
                                          const String &deviceID,
                                          UseSettingsPtr settings
                                          ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      outer_(outer),
      deviceID_(deviceID),
      settings_(settings),
      trace_(MediaChannelTrace::create(id_))
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureVideoMediaCreate, ol, MediaEngine, Start, puid, id, id_, string, deviceID, deviceID_);

      trace_->mKind = IMediaStreamTrackTypes::Kind_Video;
      trace_->mSettings = settings_;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::init() noexcept
    {
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::Media::~Media() noexcept
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureVideoMediaDestroy, ol, MediaEngine, Stop, puid, id, id_, string, deviceID, deviceID_);

      thisWeak_.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::MediaPtr MediaDeviceCaptureVideo::Media::create(
                                                                             IMessageQueuePtr queue,
                                                                             UseOuterPtr outer,
                                                                             const String &deviceID,
                                                                             UseSettingsPtr settings
                                                                             ) noexcept
    {
      auto pThis(make_shared<Media>(make_private{}, queue, outer, deviceID, settings));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::Media => (for MediaDeviceCaptureVideo)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoMediaShutdown, ol, MediaEngine, Close, puid, id, id_);

      auto pThis = thisWeak_.lock();
      postClosure([pThis] {
        AutoRecursiveLock lock(*pThis);
        pThis->cancel();
      });
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::Media::isReady() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return MediaState_Ready == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::Media::isShuttingDown() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return MediaState_ShuttingDown == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::Media::isShutdown() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return MediaState_Shutdown == currentState_;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::notifySubscribersChanged(MediaSubscriberMapPtr subscribers) noexcept
    {
      AutoRecursiveLock lock(*this);
      subscribers_ = subscribers;
    }

    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::Media => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::onWake()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoMediaWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::Media => (internal)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::cancel() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoMediaCancel, ol, MediaEngine, Cancel, puid, id, id_);

      if (isShutdown()) return;

      setState(MediaState_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
#pragma ZS_BUILD_NOTE("NEEDED?","Implement graceful shutdown of media")
      }

      setState(MediaState_Shutdown);

#pragma ZS_BUILD_NOTE("TODO","Implement hard shutdown of media")

      auto outer = outer_.lock();
      if (outer) {
        outer->notifyMediaStateChanged();
      }

      gracefulShutdownReference_.reset();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::step() noexcept
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureVideoMediaStep, ol, MediaEngine, Step, puid, id, id_);

#pragma ZS_BUILD_NOTE("TODO","Implement media step")
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureVideoMediaStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      // if (!stepDoSomething()) goto return;

      goto ready;

    ready:
      {
        setState(MediaState_Ready);
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::setState(MediaStates state) noexcept
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceCaptureVideoMediaSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, toString(state), string, oldState, toString(currentState_));

      currentState_ = state;

      if (MediaState_Ready == state) {
        auto outer = outer_.lock();
        if (outer) {
          outer->notifyMediaStateChanged();
        }
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::setError(PromisePtr promise) noexcept
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::setError(WORD errorCode, const char *inReason) noexcept
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureVideoMediaSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureVideoMediaSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);

      auto pThis = thisWeak_.lock();
      if (pThis) {
        auto outer = outer_.lock();
        if (outer) {
          outer->notifyMediaFailure(pThis, lastError_, lastErrorReason_);
        }
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::Media::internalNotifySubscribersOfFrame(VideoFramePtr frame) noexcept
    {
      MediaSubscriberMapPtr subscribers;

      ZS_EVENTING_1(x, e, Insane, MediaDeviceCaptureVideoMediaNotifySubscribersOfFrame, ol, MediaEngine, InternalEvent, puid, id, id_);

      {
        AutoRecursiveLock lock(*this);
        subscribers = subscribers_;
      }

      if (!subscribers) return;

      for (auto iter = subscribers->begin(); iter != subscribers->end(); ++iter)
      {
        auto &subscriber = (*iter).second;
        subscriber->notifyFrame(trace_, frame);
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber
    //

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::MediaSubscriber::MediaSubscriber(
                                                              const make_private &,
                                                              IMessageQueuePtr queue,
                                                              UseOuterPtr outer,
                                                              TrackConstraintsPtr constraints,
                                                              IMediaDeviceCaptureDelegatePtr delegate
                                                              ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      outer_(outer),
      constraints_(constraints),
      notifyDelegate_(delegate_),
      delegate_(IMediaDeviceCaptureDelegateProxy::createWeak(delegate)),
      traceHelper_(id_)
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureVideoMediaSubscriberCreate, ol, MediaEngine, Start, puid, id, id_, puid, outerId, outer->getID());
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::MediaSubscriber::init() noexcept
    {
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideo::MediaSubscriber::~MediaSubscriber() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoMediaSubscriberDestroy, ol, MediaEngine, Stop, puid, id, id_);

      thisWeak_.reset();
      cancel();
    }

    MediaDeviceCaptureVideo::MediaSubscriberPtr MediaDeviceCaptureVideo::MediaSubscriber::create(
                                                                                                 IMessageQueuePtr queue,
                                                                                                 UseOuterPtr outer,
                                                                                                 TrackConstraintsPtr constraints,
                                                                                                 IMediaDeviceCaptureDelegatePtr delegate
                                                                                                 ) noexcept
    {
      auto pThis(make_shared<MediaSubscriber>(make_private{}, queue, outer, constraints, delegate));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber => (for MediaDeviceCaptureVideo)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::MediaSubscriber::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureVideoMediaSubscriberShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureVideo::MediaSubscriber::isShutdown() const noexcept
    {
      return IMediaDevice::State_Shutdown == getState();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::MediaSubscriber::notifyStateChanged(States state) noexcept
    {
      auto pThis = thisWeak_.lock();

      postClosure([pThis, state] {
        AutoRecursiveLock lock(*pThis);
        pThis->setState(state);
      });
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber => (for Media)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::MediaSubscriber::notifyFrame(
                                                               ImmutableMediaChannelTracePtr trace,
                                                               VideoFramePtr frame
                                                               ) noexcept
    {
      IMediaDeviceCaptureDelegatePtr delegate;

      {
        AutoRecursiveLock lock(*this);
        delegate = notifyDelegate_.lock();
      }

      if (!delegate) return;
      delegate->notifyMediaDeviceCaptureVideoFrame(traceHelper_.trace(trace), frame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber => IMediaDevice
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::MediaSubscriber::cancel() noexcept
    {
      AutoRecursiveLock lock(*this);

      setState(IMediaDevice::State_ShuttingDown);

      setState(IMediaDevice::State_Shutdown);

      auto outer = outer_.lock();
      if (outer) {
        outer->notifySusbcriberGone();
      }

      delegate_.reset();
      notifyDelegate_.reset();
    }

    //-------------------------------------------------------------------------
    IMediaDevice::States MediaDeviceCaptureVideo::MediaSubscriber::getState() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return lastReportedState_;
    }

    //---------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber => IMediaDeviceCapture
    //

    //---------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber => IMediaDeviceCaptureVideo
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo::MediaSubscriber => (internal)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureVideo::MediaSubscriber::setState(States state) noexcept
    {
      if (state == lastReportedState_) return;
      if (IMediaDevice::State_Shutdown == lastReportedState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceCaptureVideoMediaSubscriberSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IMediaDevice::toString(state), string, oldState, IMediaDevice::toString(lastReportedState_));

      lastReportedState_ = state;

      auto pThis = thisWeak_.lock();
      if (!pThis) return;
      if (!delegate_) return;

      try {
        delegate_->onMediaDeviceCaptureStateChanged(pThis, state);
      } catch (const IMediaDeviceCaptureDelegateProxy::Exceptions::DelegateGone &) {
        delegate_.reset();
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDeviceCaptureVideoFactory
    //

    //-------------------------------------------------------------------------
    IMediaDeviceCaptureVideoFactory &IMediaDeviceCaptureVideoFactory::singleton() noexcept
    {
      return MediaDeviceCaptureVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureVideoPtr IMediaDeviceCaptureVideoFactory::create(
                                                                       UseMediaEnginePtr mediaEngine,
                                                                       const String &deviceID
                                                                       ) noexcept
    {
      if (this) {}
      return internal::MediaDeviceCaptureVideo::create(mediaEngine, deviceID);
    }

  } // internal namespace
}
