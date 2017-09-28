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

#include <ortc/internal/ortc_RTPDecoderAudio.h>
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

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_mediaengine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPDecoderAudioSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudioSettingsDefaults
    #pragma mark

    class RTPDecoderAudioSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPDecoderAudioSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPDecoderAudioSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPDecoderAudioSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPDecoderAudioSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPDecoderAudioSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPDecoderAudioSettingsDefaults()
    {
      RTPDecoderAudioSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderAudioForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPDecoderAudio::PromiseWithRTPDecoderAudioPtr IRTPDecoderAudio::create(
                                                                             const Parameters &parameters,
                                                                             IRTPDecoderDelegatePtr delegate
                                                                             )
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio, UseEngine);
      return UseEngine::createRTPDecoderAudio(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPDecoderAudioForMediaEngine::ForMediaEnginePtr IRTPDecoderAudioForMediaEngine::create(
                                                                                             PromiseWithRTPDecoderAudioPtr promise,
                                                                                             UseMediaEnginePtr mediaEngine,
                                                                                             ParametersPtr parameters,
                                                                                             IRTPDecoderDelegatePtr delegate
                                                                                             )
    {
      return internal::IRTPDecoderAudioFactory::singleton().create(promise, mediaEngine, parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPDecoderAudio::RTPDecoderAudio(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     PromiseWithRTPDecoderAudioPtr promise,
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
      ZS_EVENTING_1(x, i, Detail, RTPDecoderAudioCreate, ol, MediaEngine, Start, puid, id, id_);
    }

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::init()
    {
      asyncThisDelegate_ = IRTPDecoderAysncDelegateProxy::createWeak(thisWeak_.lock());
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPDecoderAudio::~RTPDecoderAudio()
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RTPDecoderAudioDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    RTPDecoderAudioPtr RTPDecoderAudio::create(
                                               PromiseWithRTPDecoderAudioPtr promise,
                                               UseMediaEnginePtr mediaEngine,
                                               ParametersPtr parameters,
                                               IRTPDecoderDelegatePtr delegate
                                               )
    {
      auto pThis(make_shared<RTPDecoderAudio>(make_private{}, IORTCForInternal::queueRTP(), promise, mediaEngine, parameters, delegate));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    RTPDecoderAudioPtr RTPDecoderAudio::convert(ForMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPDecoderAudio, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IRTP
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, RTPDecoderAudioCancel, ol, MediaEngine, Cancel, puid, id, id_);

      AutoRecursiveLock lock(*this);
      innerCancel();
    }

    //-------------------------------------------------------------------------
    RTPDecoderAudio::States RTPDecoderAudio::getState() const
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IRTPDecoder
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::notifyRTPPacket(
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
    void RTPDecoderAudio::notifyRTCPPacket(
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
    #pragma mark RTPDecoderAudio => IRTPDecoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IRTPForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::shutdown()
    {
      ZS_EVENTING_1(x, i, Detail, RTPDecoderAudioShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IRTPDecoderForMediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IRTPDecoderAudioForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, RTPDecoderAudioOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, RTPDecoderAudioOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio => IRTPDecoderAysncDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void RTPDecoderAudio::onRTPDecoderRTPPacket(
                                                ImmutableMediaChannelTracePtr trace,
                                                RTPPacketPtr packet
                                                )
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::onRTPDecoderRTCPPacket(
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
    #pragma mark RTPDecoderAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::innerCancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IRTP::State_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, RTPDecoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

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
    bool RTPDecoderAudio::stepShutdownPendingPromise()
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Debug, RTPDecoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no pending promise");
        return true;
      }

      ZS_EVENTING_2(x, i, Debug, RTPDecoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending promise");

      promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPDecoderAudio::stepShutdownCoder()
    {
      ZS_EVENTING_2(x, i, Debug, RTPDecoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "coder");

#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::step()
    {
      ZS_EVENTING_1(x, i, Trace, RTPDecoderAudioStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, RTPDecoderAudioStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepSetup()) goto not_ready;
      if (!stepResolve()) goto not_ready;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPDecoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPDecoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IRTP::State_Ready);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool RTPDecoderAudio::stepSetup()
    {
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPDecoderAudio::stepResolve()
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Trace, RTPDecoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "promise already resolved");
        return true;
      }

      ZS_EVENTING_2(x, i, Trace, RTPDecoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "resolving promise");

      promise_->resolve(thisWeak_.lock());
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::setState(States state)
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, RTPDecoderAudioSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IRTP::toString(state), string, oldState, IRTP::toString(currentState_));

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
    void RTPDecoderAudio::setError(PromisePtr promise)
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, RTPDecoderAudioSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, RTPDecoderAudioSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    void RTPDecoderAudio::innerNotifyAudioFrame(
                                                ImmutableMediaChannelTracePtr trace,
                                                AudioFramePtr frame
                                                )
    {
      IRTPDecoderDelegatePtr delegate;

      {
        AutoRecursiveLock lock(*this);
        delegate = notifyDelegate_.lock();
      }

      if (!delegate) return;

      delegate->notifyRTPDecoderAudioFrame(traceHelper_.trace(trace), frame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPDecoderAudioFactory &IRTPDecoderAudioFactory::singleton()
    {
      return RTPDecoderAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPDecoderAudioPtr IRTPDecoderAudioFactory::create(
                                                       PromiseWithRTPDecoderAudioPtr promise,
                                                       UseMediaEnginePtr mediaEngine,
                                                       ParametersPtr parameters,
                                                       IRTPDecoderDelegatePtr delegate
                                                       )
    {
      if (this) {}
      return internal::RTPDecoderAudio::create(promise, mediaEngine, parameters, delegate);
    }

  } // internal namespace
}
