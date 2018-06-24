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

#include <ortc/internal/ortc_RTPEncoderVideo.h>
#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>



#ifdef __GNUC__
#error MOVE THIS TO PROJECT SETTING RATHER THAN PUTTING ON INDIVIDUAL FILES
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#endif //__GNUC__

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_engine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPEncoderVideoSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideoSettingsDefaults
    //

    class RTPEncoderVideoSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPEncoderVideoSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPEncoderVideoSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<RTPEncoderVideoSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPEncoderVideoSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<RTPEncoderVideoSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPEncoderVideoSettingsDefaults() noexcept
    {
      RTPEncoderVideoSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPEncoderVideoForMediaEngine
    //

    //-------------------------------------------------------------------------
    IRTPEncoderVideo::PromiseWithRTPEncoderVideoPtr IRTPEncoderVideo::create(
                                                                             const Parameters &parameters,
                                                                             IRTPEncoderDelegatePtr delegate
                                                                             ) noexcept
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo, UseEngine);
      return UseEngine::createRTPEncoderVideo(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPEncoderVideo
    //

    //-------------------------------------------------------------------------
    IRTPEncoderVideoForMediaEngine::ForMediaEnginePtr IRTPEncoderVideoForMediaEngine::create(
                                                                                             PromiseWithRTPEncoderVideoPtr promise,
                                                                                             UseMediaEnginePtr mediaEngine,
                                                                                             ParametersPtr parameters,
                                                                                             IRTPEncoderDelegatePtr delegate
                                                                                             ) noexcept
    {
      return internal::IRTPEncoderVideoFactory::singleton().create(promise, mediaEngine, parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaEngine
    //
    
    //-------------------------------------------------------------------------
    RTPEncoderVideo::RTPEncoderVideo(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     PromiseWithRTPEncoderVideoPtr promise,
                                     UseMediaEnginePtr mediaEngine,
                                     ParametersPtr parameters,
                                     IRTPEncoderDelegatePtr delegate
                                     ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mediaEngine_(mediaEngine),
      promise_(promise),
      parameters_(parameters),
      notifyDelegate_(delegate_),
      delegate_(IRTPEncoderDelegateProxy::createWeak(delegate)),
      traceHelper_(id_)
    {
      ZS_EVENTING_1(x, i, Detail, RTPEncoderVideoCreate, ol, MediaEngine, Start, puid, id, id_);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::init() noexcept
    {
      asyncThisDelegate_ = IRTPEncoderAysncDelegateProxy::createWeak(thisWeak_.lock());
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPEncoderVideo::~RTPEncoderVideo() noexcept
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RTPEncoderVideoDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    RTPEncoderVideoPtr RTPEncoderVideo::create(
                                               PromiseWithRTPEncoderVideoPtr promise,
                                               UseMediaEnginePtr mediaEngine,
                                               ParametersPtr parameters,
                                               IRTPEncoderDelegatePtr delegate
                                               ) noexcept
    {
      auto pThis(make_shared<RTPEncoderVideo>(make_private{}, IORTCForInternal::queueRTP(), promise, mediaEngine, parameters, delegate));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    RTPEncoderVideoPtr RTPEncoderVideo::convert(ForMediaEnginePtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(RTPEncoderVideo, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTP
    //

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::cancel() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, RTPEncoderVideoCancel, ol, MediaEngine, Cancel, puid, id, id_);

      AutoRecursiveLock lock(*this);
      innerCancel();
    }

    //-------------------------------------------------------------------------
    RTPEncoderVideo::States RTPEncoderVideo::getState() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTPEncoder
    //

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::notifyVideoFrame(
                                           ImmutableMediaChannelTracePtr trace,
                                           VideoFramePtr frame
                                           ) noexcept
    {
      try {
        asyncThisDelegate_->onRTPEncoderVideoFrame(trace, frame);
      } catch (const IRTPEncoderAysncDelegateProxy::Exceptions::DelegateGone &) {
        // should never happen
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTPEncoderVideo
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTPForMediaEngine
    //

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::shutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, RTPEncoderVideoShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTPEncoderForMediaEngine
    //
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTPEncoderVideoForMediaEngine
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, RTPEncoderVideoOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IPromiseSettledDelegate
    //

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, RTPEncoderVideoOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => IRTPEncoderAysncDelegate
    //
    
    //-------------------------------------------------------------------------
    void RTPEncoderVideo::onRTPEncoderVideoFrame(
                                                 ImmutableMediaChannelTracePtr trace,
                                                 VideoFramePtr frame
                                                 )
    {
#pragma ZS_BUILD_NOTE("TODO","Implement onRTPEncoderVideoFrame")
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo => (internal)
    //

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::innerCancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IRTP::State_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, RTPEncoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

        // perform any graceful asynchronous shutdown processes needed and
        // re-attempt shutdown again later if needed.

        if (!stepShutdownPendingPromise()) return;
        if (!stepShutdownCoder()) return;
      }

      //.......................................................................
      // final cleanup (hard shutdown)

      setState(IRTP::State_Shutdown);

      stepShutdownPendingPromise();
      stepShutdownCoder();

      auto engine = mediaEngine_.lock();
      if (engine) {
        engine->notifyRTPShutdownStateChanged();
      }

      // make sure to cleanup any final reference to self
      gracefulShutdownReference_.reset();

      mediaEngine_.reset();
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderVideo::stepShutdownPendingPromise() noexcept
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Debug, RTPEncoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no pending promise");
        return true;
      }

      ZS_EVENTING_2(x, i, Debug, RTPEncoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending promise");

      promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderVideo::stepShutdownCoder() noexcept
    {
      ZS_EVENTING_2(x, i, Debug, RTPEncoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "coder");

#pragma ZS_BUILD_NOTE("TODO","Implement stepShutdownCoder")

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::step() noexcept
    {
      ZS_EVENTING_1(x, i, Trace, RTPEncoderVideoStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, RTPEncoderVideoStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepSetup()) goto not_ready;
      if (!stepResolve()) goto not_ready;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPEncoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPEncoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IRTP::State_Ready);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderVideo::stepSetup() noexcept
    {
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderVideo::stepResolve() noexcept
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Trace, RTPEncoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "promise already resolved");
        return true;
      }

      ZS_EVENTING_2(x, i, Trace, RTPEncoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "resolving promise");

      promise_->resolve(thisWeak_.lock());
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::setState(States state) noexcept
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, RTPEncoderVideoSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IRTP::toString(state), string, oldState, IRTP::toString(currentState_));

      currentState_ = state;

      auto pThis = thisWeak_.lock();

      if ((delegate_) &&
          (pThis)) {
        try {
          delegate_->onRTPEncoderStateChanged(thisWeak_.lock(), state);
        } catch (IRTPEncoderDelegateProxy::Exceptions::DelegateGone &) {
        }
      }

//      MediaEnginePtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaEngineStateChanged(pThis, currentState_);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::setError(PromisePtr promise) noexcept
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::setError(WORD errorCode, const char *inReason) noexcept
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, RTPEncoderVideoSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, RTPEncoderVideoSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::innerNotifyRTP(
                                         ImmutableMediaChannelTracePtr trace,
                                         RTPPacketPtr packet
                                         ) noexcept
    {
      IRTPEncoderDelegatePtr delegate;

      {
        AutoRecursiveLock lock(*this);
        delegate = notifyDelegate_.lock();
      }

      if (!delegate) return;

      delegate->notifyRTPEncoderRTPPacket(traceHelper_.trace(trace), packet);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderVideo::innerNotifyRTCP(
                                          ImmutableMediaChannelTracePtr trace,
                                          RTCPPacketPtr packet
                                          ) noexcept
    {
      IRTPEncoderDelegatePtr delegate;

      {
        AutoRecursiveLock lock(*this);
        delegate = notifyDelegate_.lock();
      }

      if (!delegate) return;

      delegate->notifyRTPEncoderRTCPPacket(traceHelper_.trace(trace), packet);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPEncoderVideoFactory
    //

    //-------------------------------------------------------------------------
    IRTPEncoderVideoFactory &IRTPEncoderVideoFactory::singleton() noexcept
    {
      return RTPEncoderVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPEncoderVideoPtr IRTPEncoderVideoFactory::create(
                                                       PromiseWithRTPEncoderVideoPtr promise,
                                                       UseMediaEnginePtr mediaEngine,
                                                       ParametersPtr parameters,
                                                       IRTPEncoderDelegatePtr delegate
                                                       ) noexcept
    {
      if (this) {}
      return internal::RTPEncoderVideo::create(promise, mediaEngine, parameters, delegate);
    }

  } // internal namespace
}


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif //__GNUC__
