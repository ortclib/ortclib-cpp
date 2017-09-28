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

#include <ortc/internal/ortc_RTPEncoderAudio.h>
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
    ZS_DECLARE_CLASS_PTR(RTPEncoderAudioSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudioSettingsDefaults
    #pragma mark

    class RTPEncoderAudioSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPEncoderAudioSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPEncoderAudioSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPEncoderAudioSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPEncoderAudioSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPEncoderAudioSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPEncoderAudioSettingsDefaults()
    {
      RTPEncoderAudioSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPEncoderAudioForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPEncoderAudio::PromiseWithRTPEncoderAudioPtr IRTPEncoderAudio::create(
                                                                             const Parameters &parameters,
                                                                             IRTPEncoderDelegatePtr delegate
                                                                             )
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio, UseEngine);
      return UseEngine::createRTPEncoderAudio(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPEncoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPEncoderAudioForMediaEngine::ForMediaEnginePtr IRTPEncoderAudioForMediaEngine::create(
                                                                                             PromiseWithRTPEncoderAudioPtr promise,
                                                                                             UseMediaEnginePtr mediaEngine,
                                                                                             ParametersPtr parameters,
                                                                                             IRTPEncoderDelegatePtr delegate
                                                                                             )
    {
      return internal::IRTPEncoderAudioFactory::singleton().create(promise, mediaEngine, parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPEncoderAudio::RTPEncoderAudio(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     PromiseWithRTPEncoderAudioPtr promise,
                                     UseMediaEnginePtr mediaEngine,
                                     ParametersPtr parameters,
                                     IRTPEncoderDelegatePtr delegate
                                     ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      promise_(promise),
      mediaEngine_(mediaEngine),
      parameters_(parameters),
      notifyDelegate_(delegate_),
      delegate_(IRTPEncoderDelegateProxy::createWeak(delegate)),
      traceHelper_(id_)
    {
      ZS_EVENTING_1(x, i, Detail, RTPEncoderAudioCreate, ol, MediaEngine, Start, puid, id, id_);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::init()
    {
      asyncThisDelegate_ = IRTPEncoderAysncDelegateProxy::createWeak(thisWeak_.lock());
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPEncoderAudio::~RTPEncoderAudio()
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RTPEncoderAudioDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    RTPEncoderAudioPtr RTPEncoderAudio::create(
                                               PromiseWithRTPEncoderAudioPtr promise,
                                               UseMediaEnginePtr mediaEngine,
                                               ParametersPtr parameters,
                                               IRTPEncoderDelegatePtr delegate
                                               )
    {
      auto pThis(make_shared<RTPEncoderAudio>(make_private{}, IORTCForInternal::queueRTP(), promise, mediaEngine, parameters, delegate));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    RTPEncoderAudioPtr RTPEncoderAudio::convert(ForMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPEncoderAudio, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTP
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, RTPEncoderAudioCancel, ol, MediaEngine, Cancel, puid, id, id_);

      AutoRecursiveLock lock(*this);
      innerCancel();
    }

    //-------------------------------------------------------------------------
    RTPEncoderAudio::States RTPEncoderAudio::getState() const
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTPEncoder
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::notifyAudioFrame(
                                           ImmutableMediaChannelTracePtr trace,
                                           AudioFramePtr frame
                                           )
    {
      try {
        asyncThisDelegate_->onRTPEncoderAudioFrame(trace, frame);
      } catch (const IRTPEncoderAysncDelegateProxy::Exceptions::DelegateGone &) {
        // should never happen
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTPEncoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTPForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::shutdown()
    {
      ZS_EVENTING_1(x, i, Detail, RTPEncoderAudioShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTPEncoderForMediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTPEncoderAudioForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, RTPEncoderAudioOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, RTPEncoderAudioOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio => IRTPEncoderAysncDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void RTPEncoderAudio::onRTPEncoderAudioFrame(
                                                 ImmutableMediaChannelTracePtr trace,
                                                 AudioFramePtr frame
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
    #pragma mark RTPEncoderAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::innerCancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IRTP::State_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, RTPEncoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

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
    bool RTPEncoderAudio::stepShutdownPendingPromise()
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Debug, RTPEncoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no pending promise");
        return true;
      }

      ZS_EVENTING_2(x, i, Debug, RTPEncoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending promise");

      promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderAudio::stepShutdownCoder()
    {
      ZS_EVENTING_2(x, i, Debug, RTPEncoderAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "coder");

#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::step()
    {
      ZS_EVENTING_1(x, i, Trace, RTPEncoderAudioStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, RTPEncoderAudioStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepSetup()) goto not_ready;
      if (!stepResolve()) goto not_ready;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPEncoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, RTPEncoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IRTP::State_Ready);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderAudio::stepSetup()
    {
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPEncoderAudio::stepResolve()
    {
      if (!promise_) {
        ZS_EVENTING_2(x, i, Trace, RTPEncoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "promise already resolved");
        return true;
      }

      ZS_EVENTING_2(x, i, Trace, RTPEncoderAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "resolving promise");

      promise_->resolve(thisWeak_.lock());
      promise_.reset();
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::setState(States state)
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, RTPEncoderAudioSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IRTP::toString(state), string, oldState, IRTP::toString(currentState_));

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
    void RTPEncoderAudio::setError(PromisePtr promise)
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, RTPEncoderAudioSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, RTPEncoderAudioSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    void RTPEncoderAudio::innerNotifyRTP(
                                         ImmutableMediaChannelTracePtr trace,
                                         RTPPacketPtr packet
                                         )
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
    void RTPEncoderAudio::innerNotifyRTCP(
                                          ImmutableMediaChannelTracePtr trace,
                                          RTCPPacketPtr packet
                                          )
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
    #pragma mark
    #pragma mark IRTPEncoderAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPEncoderAudioFactory &IRTPEncoderAudioFactory::singleton()
    {
      return RTPEncoderAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPEncoderAudioPtr IRTPEncoderAudioFactory::create(
                                                       PromiseWithRTPEncoderAudioPtr promise,
                                                       UseMediaEnginePtr mediaEngine,
                                                       ParametersPtr parameters,
                                                       IRTPEncoderDelegatePtr delegate
                                                       )
    {
      if (this) {}
      return internal::RTPEncoderAudio::create(promise, mediaEngine, parameters, delegate);
    }

  } // internal namespace
}
