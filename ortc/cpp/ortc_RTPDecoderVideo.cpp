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

#include <ortc/internal/ortc_RTPDecoderVideo.h>
#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_MediaEngine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPDecoderVideoSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideoSettingsDefaults
    #pragma mark

    class RTPDecoderVideoSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPDecoderVideoSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPDecoderVideoSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPDecoderVideoSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPDecoderVideoSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPDecoderVideoSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPDecoderVideoSettingsDefaults()
    {
      RTPDecoderVideoSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderVideoForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPDecoderVideo::PromiseWithRTPDecoderVideoPtr IRTPDecoderVideo::create(
                                                                             const Parameters &parameters,
                                                                             IRTPDecoderDelegatePtr delegate
                                                                             )
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderVideo, UseEngine);
      return UseEngine::createRTPDecoderVideo(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoderVideo
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPDecoderVideoForMediaEngine::ForMediaEnginePtr IRTPDecoderVideoForMediaEngine::create(
                                                                                             PromiseWithRTPDecoderVideoPtr promise,
                                                                                             UseMediaEnginePtr mediaEngine,
                                                                                             ParametersPtr parameters,
                                                                                             IRTPDecoderDelegatePtr delegate
                                                                                             )
    {
      return internal::IRTPDecoderVideoFactory::singleton().create(promise, mediaEngine, parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPDecoderVideo::RTPDecoderVideo(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     PromiseWithRTPDecoderVideoPtr promise,
                                     UseMediaEnginePtr mediaEngine,
                                     ParametersPtr parameters,
                                     IRTPDecoderDelegatePtr delegate
                                     ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      promise_(promise),
      mediaEngine_(mediaEngine),
      parameters_(parameters),
      notifyDelegate_(delegate_),
      delegate_(IRTPDecoderDelegateProxy::createWeak(delegate)),
      traceHelper_(id_)
    {
      ZS_EVENTING_1(x, i, Detail, RTPDecoderVideoCreate, ol, MediaEngine, Start, puid, id, id_);
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::init()
    {
      asyncThisDelegate_ = IRTPDecoderAysncDelegateProxy::createWeak(thisWeak_.lock());
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPDecoderVideo::~RTPDecoderVideo()
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RTPDecoderVideoDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    RTPDecoderVideoPtr RTPDecoderVideo::create(
                                               PromiseWithRTPDecoderVideoPtr promise,
                                               UseMediaEnginePtr mediaEngine,
                                               ParametersPtr parameters,
                                               IRTPDecoderDelegatePtr delegate
                                               )
    {
      auto pThis(make_shared<RTPDecoderVideo>(make_private{}, IORTCForInternal::queueRTP(), promise, mediaEngine, parameters, delegate));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    RTPDecoderVideoPtr RTPDecoderVideo::convert(ForMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPDecoderVideo, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTP
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, RTPDecoderVideoCancel, ol, MediaEngine, Cancel, puid, id, id_);

      AutoRecursiveLock lock(*this);
      innerCancel();
    }

    //-------------------------------------------------------------------------
    RTPDecoderVideo::States RTPDecoderVideo::getState() const
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTPDecoder
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::notifyRTPPacket(
                                          ImmutableMediaChannelTracePtr trace,
                                          RTPPacketPtr packet
                                          )
    {
      try {
        asyncThisDelegate_->onRTPDecoderRTPPacket(trace, packet);
      } catch (const IRTPDecoderAysncDelegateProxy::Exceptions::DelegateGone &) {
        // should never happen
      }
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::notifyRTCPPacket(
                                           ImmutableMediaChannelTracePtr trace,
                                           RTCPPacketPtr packet
                                           )
    {
      try {
        asyncThisDelegate_->onRTPDecoderRTCPPacket(trace, packet);
      } catch (const IRTPDecoderAysncDelegateProxy::Exceptions::DelegateGone &) {
        // should never happen
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTPDecoderVideo
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTPForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::shutdown()
    {
      ZS_EVENTING_1(x, i, Detail, RTPDecoderVideoShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTPDecoderForMediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTPDecoderVideoForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, RTPDecoderVideoOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, RTPDecoderVideoOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => IRTPDecoderAysncDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void RTPDecoderVideo::onRTPDecoderRTPPacket(
                                                ImmutableMediaChannelTracePtr trace,
                                                RTPPacketPtr packet
                                                )
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::onRTPDecoderRTCPPacket(
                                                 ImmutableMediaChannelTracePtr trace,
                                                 RTCPPacketPtr packet
                                                 )
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderVideo => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::innerCancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IRTP::State_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, RTPDecoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

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
    bool RTPDecoderVideo::stepShutdownPendingPromise()
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Debug, RTPDecoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no pending promise");
        return true;
      }

      ZS_EVENTING_2(x, i, Debug, RTPDecoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending promise");

      promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPDecoderVideo::stepShutdownCoder()
    {
      ZS_EVENTING_2(x, i, Debug, RTPDecoderVideoShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "coder");

#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::step()
    {
      ZS_EVENTING_1(x, i, Trace, RTPDecoderVideoStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, RTPDecoderVideoStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepSetup()) goto not_ready;
      if (!stepResolve()) goto not_ready;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPDecoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPDecoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IRTP::State_Ready);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool RTPDecoderVideo::stepSetup()
    {
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPDecoderVideo::stepResolve()
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Trace, RTPDecoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "promise already resolved");
        return true;
      }

      ZS_EVENTING_2(x, i, Trace, RTPDecoderVideoStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "resolving promise");

      promise_->resolve(thisWeak_.lock());
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::setState(States state)
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, RTPDecoderVideoSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IRTP::toString(state), string, oldState, IRTP::toString(currentState_));

      currentState_ = state;

      auto pThis = thisWeak_.lock();

      if ((delegate_) &&
          (pThis)) {
        try {
          delegate_->onRTPDecoderStateChanged(thisWeak_.lock(), state);
        } catch (IRTPDecoderDelegateProxy::Exceptions::DelegateGone &) {
        }
      }

//      MediaEnginePtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaEngineStateChanged(pThis, currentState_);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::setError(PromisePtr promise)
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, RTPDecoderVideoSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, RTPDecoderVideoSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    void RTPDecoderVideo::innerNotifyVideoFrame(
                                                ImmutableMediaChannelTracePtr trace,
                                                VideoFramePtr frame
                                                )
    {
      IRTPDecoderDelegatePtr delegate;

      {
        AutoRecursiveLock lock(*this);
        delegate = notifyDelegate_.lock();
      }

      if (!delegate) return;

      delegate->notifyRTPDecoderVideoFrame(traceHelper_.trace(trace), frame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderVideoFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPDecoderVideoFactory &IRTPDecoderVideoFactory::singleton()
    {
      return RTPDecoderVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPDecoderVideoPtr IRTPDecoderVideoFactory::create(
                                                       PromiseWithRTPDecoderVideoPtr promise,
                                                       UseMediaEnginePtr mediaEngine,
                                                       ParametersPtr parameters,
                                                       IRTPDecoderDelegatePtr delegate
                                                       )
    {
      if (this) {}
      return internal::RTPDecoderVideo::create(promise, mediaEngine, parameters, delegate);
    }

  } // internal namespace
}
