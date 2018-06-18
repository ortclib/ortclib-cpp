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

#include <ortc/internal/ortc_MediaDeviceRenderAudio.h>
#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_engine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaDeviceRenderAudioSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudioSettingsDefaults
    //

    class MediaDeviceRenderAudioSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaDeviceRenderAudioSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaDeviceRenderAudioSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<MediaDeviceRenderAudioSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaDeviceRenderAudioSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<MediaDeviceRenderAudioSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaDeviceRenderAudioSettingsDefaults() noexcept
    {
      MediaDeviceRenderAudioSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDeviceRenderAudioForMediaEngine
    //

    //-------------------------------------------------------------------------
    IMediaDeviceRenderAudioForMediaEngine::ForMediaEnginePtr IMediaDeviceRenderAudioForMediaEngine::create(
                                                                                                           UseMediaEnginePtr mediaEngine,
                                                                                                           const String &deviceID
                                                                                                           ) noexcept
    {
      return internal::IMediaDeviceRenderAudioFactory::singleton().create(mediaEngine, deviceID);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaEngine
    //
    
    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::MediaDeviceRenderAudio(
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
      ZS_EVENTING_2(x, i, Detail, MediaDeviceRenderAudioCreate, ol, MediaEngine, Start, puid, id, id_, string, deviceID, deviceID_);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::init() noexcept
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::~MediaDeviceRenderAudio() noexcept
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    MediaDeviceRenderAudioPtr MediaDeviceRenderAudio::create(
                                                             UseMediaEnginePtr mediaEngine,
                                                             const String &deviceID
                                                             ) noexcept
    {
      auto pThis(make_shared<MediaDeviceRenderAudio>(make_private{}, IORTCForInternal::queueMediaDevices(), mediaEngine, deviceID));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudioPtr MediaDeviceRenderAudio::convert(ForMediaEnginePtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaDeviceRenderAudio, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio => (for Media)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::notifyMediaStateChanged() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, MediaDeviceRenderAudioNotifyMediaStateChanged, ol, MediaEngine, Event, puid, id, id_);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::notifyMediaFailure(
                                                     MediaPtr media,
                                                     WORD errorCode,
                                                     const char *inReason
                                                     ) noexcept
    {
      ZS_EVENTING_2(x, e, Debug, MediaDeviceRenderAudioNotifyMediaFailure, ol, MediaEngine, Event, puid, id, id_, puid, mediaId, media->getID());

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
    // MediaDeviceRenderAudio => (for MediaSubscribers)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::notifySusbcriberGone() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, MediaDeviceRenderAudioNotifySubscriberGone, ol, MediaEngine, Event, puid, id, id_);

      recheckMode_ = true;
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::notifyAudioFrame(
                                                  ImmutableMediaChannelTracePtr trace,
                                                  AudioFramePtr frame
                                                  ) noexcept
    {
      MediaPtr media;

      {
        AutoRecursiveLock lock(*this);
        media = media_;
      }

      if (!media) return;

      media->notifyAudioFrame(trace, frame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio => IMediaDeviceForMediaEngine
    //

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::isDeviceIdle() noexcept
    {
      AutoRecursiveLock lock(*this);

      if (isShutdown()) return true;

      if (pendingSubscribers_.size() > 0) return false;
      if (subscribers_->size() > 0) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::States MediaDeviceRenderAudio::getState() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio => IMediaDeviceRenderForMediaEngine
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::mediaDeviceRenderSubscribe(
                                                            MediaDeviceRenderPromisePtr promise,
                                                            MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                            TrackConstraintsPtr constraints,
                                                            IMediaDeviceRenderDelegatePtr delegate
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
    // MediaDeviceRenderAudio => IMediaDeviceRenderAudioForMediaEngine
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceRenderAudioOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio => IPromiseSettledDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

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
    // MediaDeviceRenderAudio => (internal)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::cancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IMediaDevice::State_ShuttingDown);

      deviceModesPromise_.reset();

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, MediaDeviceRenderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

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
    bool MediaDeviceRenderAudio::stepShutdownPendingRequests() noexcept
    {
      ZS_EVENTING_2(x, i, Debug, MediaDeviceRenderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending requests");

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
    bool MediaDeviceRenderAudio::stepShutdownSubscribers() noexcept
    {
      if (subscribers_->size() < 1) return true;

      ZS_EVENTING_2(x, i, Debug, MediaDeviceRenderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "subscribers");

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
    bool MediaDeviceRenderAudio::stepShutdownMedia() noexcept
    {
      ZS_EVENTING_2(x, i, Debug, MediaDeviceRenderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media");

      if (!media_) return true;

      media_->shutdown();
      bool shutdown = media_->isShutdown();
      if (shutdown) {
        media_.reset();
      }
      return shutdown;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::step() noexcept
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceRenderAudioStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaDeviceRenderAudioStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
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
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    reinitializing:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "reinitializing");
        setState(IMediaDevice::State_Reinitializing);
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IMediaDevice::State_Active);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::stepMediaReinitializationShutdown() noexcept
    {
      if (!media_) goto no_media_to_shutdown;

      if (media_->isShuttingDown()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "waiting for media shutdown to reinialize");
        return false;
      }

      if (media_->isShutdown()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media shutdown");
        media_.reset();
        return true;
      }

    no_media_to_shutdown:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no media to shutdown");
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::stepDiscoverModes() noexcept
    {
      if (!deviceModesPromise_) {
        deviceModesPromise_ = IMediaDevices::enumerateDefaultModes(deviceID_);

        if (!deviceModesPromise_) {
          setError(UseHTTP::HTTPStatusCode_ExpectationFailed, (String("unable to discover modes for an audio media device: ") + deviceID_).c_str());
          cancel();
          return false;
        }
        deviceModesPromise_->thenWeak(thisWeak_.lock());
      }

      if (!deviceModesPromise_->isSettled()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "waiting for device modes to be known");
        return false;
      }

      if (deviceModesPromise_->isRejected()) {
        setError(deviceModesPromise_);
        cancel();
        return false;
      }

      auto settingsList = deviceModesPromise_->value();

      if (!settingsList) {
        setError(UseHTTP::HTTPStatusCode_ExpectationFailed, (String("no modes found for audio media device: ") + deviceID_).c_str());
        cancel();
        return false;
      }

      ZS_EVENTING_3(x, i, Debug, MediaDeviceRenderAudioStepModesFound, ol, MediaEngine, Step,
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
    bool MediaDeviceRenderAudio::stepFigureOutMode() noexcept
    {
      {
        if (recheckMode_) goto mode_needed;
        if (!media_) goto mode_needed;
        if (pendingSubscribers_.size() > 0) goto mode_needed;
        return true;
      }

    mode_needed:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "figure out mode");

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
            info.promise_->resolve(IMediaDeviceRenderPtr(subscriber));
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
            ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "optimal device mode has not changed");
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
    bool MediaDeviceRenderAudio::stepWaitForMediaDevice() noexcept
    {
      if (media_->isShutdown()) {
        media_.reset();
      }

      if (!media_) {
        if (!requiredSettings_) {
          ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no media device mode required");
          return false;
        }
        media_ = Media::create(getAssociatedMessageQueue(), thisWeak_.lock(), deviceID_, requiredSettings_);
        media_->notifySubscribersChanged(subscribers_);
      }

      if (!media_->isReady()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media device is not ready");
        return false;
      }

      ZS_EVENTING_2(x, i, Trace, MediaDeviceRenderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media device is ready");
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::setState(States state) noexcept
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceRenderAudioSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IMediaDevice::toString(state), string, oldState, IMediaDevice::toString(currentState_));

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
    void MediaDeviceRenderAudio::setError(PromisePtr promise) noexcept
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::setError(WORD errorCode, const char *inReason) noexcept
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaDeviceRenderAudioSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaDeviceRenderAudioSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::Media
    //

    //-------------------------------------------------------------------------
    const char *MediaDeviceRenderAudio::Media::toString(MediaStates state) noexcept
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
    MediaDeviceRenderAudio::Media::Media(
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
      settings_(settings)
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceRenderAudioMediaCreate, ol, MediaEngine, Start, puid, id, id_, string, deviceID, deviceID_);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::init() noexcept
    {
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::Media::~Media() noexcept
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceRenderAudioMediaDestroy, ol, MediaEngine, Stop, puid, id, id_, string, deviceID, deviceID_);

      thisWeak_.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::MediaPtr MediaDeviceRenderAudio::Media::create(
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
    // MediaDeviceRenderAudio::Media => (for MediaDeviceRenderAudio)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioMediaShutdown, ol, MediaEngine, Close, puid, id, id_);

      auto pThis = thisWeak_.lock();
      postClosure([pThis] {
        AutoRecursiveLock lock(*pThis);
        pThis->cancel();
      });
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::Media::isReady() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return MediaState_Ready == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::Media::isShuttingDown() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return MediaState_ShuttingDown == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::Media::isShutdown() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return MediaState_Shutdown == currentState_;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::notifySubscribersChanged(MediaSubscriberMapPtr subscribers) noexcept
    {
      AutoRecursiveLock lock(*this);
      subscribers_ = subscribers;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::notifyAudioFrame(
                                                         ImmutableMediaChannelTracePtr trace,
                                                         AudioFramePtr frame
                                                         ) noexcept
    {
#pragma ZS_BUILD_NOTE("TODO","Implement notify audio frame")
    }

    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::Media => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::onWake()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioMediaWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::Media => (internal)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::cancel() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioMediaCancel, ol, MediaEngine, Cancel, puid, id, id_);

      if (isShutdown()) return;

      setState(MediaState_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
#pragma ZS_BUILD_NOTE("NEEDED?","Implement graceful shutdown of render audio")
      }

      setState(MediaState_Shutdown);

#pragma ZS_BUILD_NOTE("TODO","Implement hard shutdown of render audio")

      auto outer = outer_.lock();
      if (outer) {
        outer->notifyMediaStateChanged();
      }

      gracefulShutdownReference_.reset();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::step() noexcept
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceRenderAudioMediaStep, ol, MediaEngine, Step, puid, id, id_);

#pragma ZS_BUILD_NOTE("TODO","Implement render audio step")
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaDeviceRenderAudioMediaStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      // if (!stepDoSomething()) return;

      goto ready;

    ready:
      {
        setState(MediaState_Ready);
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::setState(MediaStates state) noexcept
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceRenderAudioMediaSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, toString(state), string, oldState, toString(currentState_));

      currentState_ = state;

      if (MediaState_Ready == state) {
        auto outer = outer_.lock();
        if (outer) {
          outer->notifyMediaStateChanged();
        }
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::setError(PromisePtr promise) noexcept
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::Media::setError(WORD errorCode, const char *inReason) noexcept
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaDeviceRenderAudioMediaSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaDeviceRenderAudioMediaSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);

      auto pThis = thisWeak_.lock();
      if (pThis) {
        auto outer = outer_.lock();
        if (outer) {
          outer->notifyMediaFailure(pThis, lastError_, lastErrorReason_);
        }
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber
    //

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::MediaSubscriber::MediaSubscriber(
                                                             const make_private &,
                                                             IMessageQueuePtr queue,
                                                             UseOuterPtr outer,
                                                             TrackConstraintsPtr constraints,
                                                             IMediaDeviceRenderDelegatePtr delegate
                                                             ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      outer_(outer),
      constraints_(constraints),
      notifyDelegate_(delegate_),
      delegate_(IMediaDeviceRenderDelegateProxy::createWeak(delegate)),
      traceHelper_(id_)
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceRenderAudioMediaSubscriberCreate, ol, MediaEngine, Start, puid, id, id_, puid, outerId, outer->getID());
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::MediaSubscriber::init() noexcept
    {
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::MediaSubscriber::~MediaSubscriber() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioMediaSubscriberDestroy, ol, MediaEngine, Stop, puid, id, id_);

      thisWeak_.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudio::MediaSubscriberPtr MediaDeviceRenderAudio::MediaSubscriber::create(
                                                                                               IMessageQueuePtr queue,
                                                                                               UseOuterPtr outer,
                                                                                               TrackConstraintsPtr constraints,
                                                                                               IMediaDeviceRenderDelegatePtr delegate
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
    // MediaDeviceRenderAudio::MediaSubscriber => (for MediaDeviceRenderAudio)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::MediaSubscriber::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceRenderAudioMediaSubscriberShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceRenderAudio::MediaSubscriber::isShutdown() const noexcept
    {
      return IMediaDevice::State_Shutdown == getState();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::MediaSubscriber::notifyStateChanged(States state) noexcept
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
    // MediaDeviceRenderAudio::MediaSubscriber => (for Media)
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber => IMediaDeviceRender
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber => IMediaDeviceRenderAudio
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::MediaSubscriber::notifyAudioFrame(
                                                                   ImmutableMediaChannelTracePtr trace,
                                                                   AudioFramePtr frame
                                                                   ) noexcept
    {
      auto outer = outer_.lock();
      if (!outer) return;

      outer->notifyAudioFrame(traceHelper_.trace(trace), frame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber => IMediaDevice
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::MediaSubscriber::cancel() noexcept
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
    IMediaDevice::States MediaDeviceRenderAudio::MediaSubscriber::getState() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return lastReportedState_;
    }

    //---------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber => IMediaDeviceRender
    //

    //---------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber => IMediaDeviceRenderAudio
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceRenderAudio::MediaSubscriber => (internal)
    //

    //-------------------------------------------------------------------------
    void MediaDeviceRenderAudio::MediaSubscriber::setState(States state) noexcept
    {
      if (state == lastReportedState_) return;
      if (IMediaDevice::State_Shutdown == lastReportedState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceRenderAudioMediaSubscriberSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IMediaDevice::toString(state), string, oldState, IMediaDevice::toString(lastReportedState_));

      lastReportedState_ = state;

      auto pThis = thisWeak_.lock();
      if (!pThis) return;
      if (!delegate_) return;

      try {
        delegate_->onMediaDeviceRenderStateChanged(pThis, state);
      } catch (const IMediaDeviceRenderDelegateProxy::Exceptions::DelegateGone &) {
        delegate_.reset();
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDeviceRenderAudioFactory
    //

    //-------------------------------------------------------------------------
    IMediaDeviceRenderAudioFactory &IMediaDeviceRenderAudioFactory::singleton() noexcept
    {
      return MediaDeviceRenderAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    MediaDeviceRenderAudioPtr IMediaDeviceRenderAudioFactory::create(
                                                                     UseMediaEnginePtr mediaEngine,
                                                                     const String &deviceID
                                                                     ) noexcept
    {
      if (this) {}
      return internal::MediaDeviceRenderAudio::create(mediaEngine, deviceID);
    }

  } // internal namespace
}
