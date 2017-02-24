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

#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpsender) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);

  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPSenderChannelAudioSettingsDefaults);

    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark


    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudioSettingsDefaults
    #pragma mark

    class RTPSenderChannelAudioSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPSenderChannelAudioSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPSenderChannelAudioSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPSenderChannelAudioSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPSenderChannelAudioSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPSenderChannelAudioSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPSenderChannelAudioSettingsDefaults()
    {
      RTPSenderChannelAudioSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr IRTPSenderChannelAudioForRTPSenderChannel::create(
                                                                               RTPSenderChannelPtr senderChannel,
                                                                               MediaStreamTrackPtr track,
                                                                               const Parameters &params
                                                                               )
    {
      return internal::IRTPSenderChannelAudioFactory::singleton().create(senderChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio::ToneInfo
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannelAudio::ToneInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPSenderChannelAudio::ToneInfo");

      IHelper::debugAppend(resultEl, "tones", mTones);
      IHelper::debugAppend(resultEl, "duration", mDuration);
      IHelper::debugAppend(resultEl, "inter tone gap", mInterToneGap);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio
    #pragma mark
    
    //-------------------------------------------------------------------------
    const char *RTPSenderChannelAudio::toString(States state)
    {
      switch (state) {
        case State_Pending:       return "pending";
        case State_Ready:         return "ready";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::RTPSenderChannelAudio(
                                                 const make_private &,
                                                 IMessageQueuePtr queue,
                                                 UseChannelPtr senderChannel,
                                                 UseMediaStreamTrackPtr track,
                                                 const Parameters &params
                                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSenderChannel(senderChannel),
      mTrack(track),
      mParameters(make_shared<Parameters>(params))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!senderChannel)
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::init()
    {
      TransportPtr transport = Transport::create(mThisWeak.lock());

      PromiseWithRTPMediaEngineChannelResourcePtr setupChannelPromise = UseMediaEngine::setupChannel(
                                                                                                     mThisWeak.lock(),
                                                                                                     transport,
                                                                                                     MediaStreamTrack::convert(mTrack),
                                                                                                     mParameters,
                                                                                                     mThisWeak.lock()
                                                                                                     );
      {
        AutoRecursiveLock lock(*this);
        mChannelResourceLifetimeHolderPromise = setupChannelPromise;
        mTransport = transport;
      }
      setupChannelPromise->thenWeak(mThisWeak.lock());

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::~RTPSenderChannelAudio()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForRTPSenderChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForRTPSenderChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForRTPMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onTrackChanged(UseBaseMediaStreamTrackPtr track)
    {
      AutoRecursiveLock lock(*this);
      mTrack = MediaStreamTrack::convert(track);  // NOTE: track might be NULL
#define TODO_HANDLE_CHANGE_IN_TRACK 1
#define TODO_HANDLE_CHANGE_IN_TRACK 2
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::notifyTransportState(ISecureTransportTypes::States state)
    {
      IRTPSenderChannelAudioAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::notifyUpdate(ParametersPtr params)
    {
      IRTPSenderChannelAudioAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(params);
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::handlePacket(RTCPPacketPtr packet)
    {
      UseChannelResourcePtr channelResource;

      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
      }

      if (!channelResource) return false;
      return channelResource->handlePacket(*packet);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      UseChannelResourcePtr channelResource;

      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
      }

      if (!channelResource) {
        promise->reject();
        return;
      }
      return channelResource->requestStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::insertDTMF(
                                           const char *tones,
                                           Milliseconds duration,
                                           Milliseconds interToneGap
                                           )
    {
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;

        if ((!channelResource) ||
            (mPendingTones.size() > 0)) {
          auto info = make_shared<ToneInfo>();
          info->mTones = String(tones);
          info->mDuration = duration;
          info->mInterToneGap = interToneGap;
          mPendingTones.push_back(info);
          return;
        }
      }

      channelResource->insertDTMF(tones, duration, interToneGap);
    }

    //-------------------------------------------------------------------------
    String RTPSenderChannelAudio::toneBuffer() const
    {
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;

        if (mPendingTones.size() > 0) {
          auto info = mPendingTones.front();
          return info->mTones;
        }
      }

      if (!channelResource) return String();

      return channelResource->toneBuffer();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSenderChannelAudio::duration() const
    {
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
        if (mPendingTones.size() > 0) {
          auto info = mPendingTones.front();
          return info->mDuration;
        }
      }

      if (!channelResource) return Milliseconds();

      return channelResource->duration();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSenderChannelAudio::interToneGap() const
    {
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
        if (mPendingTones.size() > 0) {
          auto info = mPendingTones.front();
          return info->mInterToneGap;
        }
      }

      if (!channelResource) return Milliseconds();

      return channelResource->interToneGap();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioForRTPSenderChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::create(
                                                           RTPSenderChannelPtr senderChannel,
                                                           MediaStreamTrackPtr track,
                                                           const Parameters &params
                                                           )
    {
      RTPSenderChannelAudioPtr pThis(make_shared<RTPSenderChannelAudio>(make_private {}, IORTCForInternal::queueORTC(), senderChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    int32_t RTPSenderChannelAudio::sendAudioSamples(
                                                    const void* audioSamples,
                                                    const size_t numberOfSamples,
                                                    const uint8_t numberOfChannels
                                                    )
    {
#define TODO_IMPLEMENT_THIS 1
#define TODO_IMPLEMENT_THIS 2
      return 0;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onWake()
    {
      ZS_LOG_DEBUG(log("wake"));

      ToneInfoList pending;
      UseChannelResourcePtr channelResource;

      {
        AutoRecursiveLock lock(*this);
        step();

        if ((mChannelResource) &&
            (mPendingTones.size() > 0)) {
          pending = mPendingTones;
          mPendingTones.clear();

          auto tempLockoutTones = make_shared<ToneInfo>();
          tempLockoutTones->mTones = ORTC_SENDER_CHANNEL_PENDING_TONE_LOCK_OUT_MAGIC_STRING;

          mPendingTones.push_back(tempLockoutTones);

          channelResource = mChannelResource;
        }
      }

      if (pending.size() > 0)
      {
        while (pending.size() > 0)
        {
          ToneInfoPtr tone = pending.front();
          pending.pop_front();

          if (0 == tone->mTones.compare(ORTC_SENDER_CHANNEL_PENDING_TONE_LOCK_OUT_MAGIC_STRING)) continue;

          channelResource->insertDTMF(tone->mTones, tone->mDuration, tone->mInterToneGap);
        }

        {
          AutoRecursiveLock lock(*this);

          if (mPendingTones.size() > 1) {
            // NOTE: a pending tone was inserted while the current tone was
            // being played out thus attempt to deliver pending tones again
            IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
            return;
          }

          // NOTE: the only possible tone in the tone buffer is the lock out
          /// tone thus clear out the pending tone
          mPendingTones.clear();
        }
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onDTMFSenderToneChanged(
                                                        IDTMFSenderPtr sender,
                                                        String tone
                                                        )
    {
      auto senderChannel = mSenderChannel.lock();

      if (!senderChannel) {
        ZS_LOG_WARNING(Debug, log("cannot foreward tone event (sender channel is gone)"));
        return;
      }

      senderChannel->notifyDTMFSenderToneChanged(tone);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onTimer(ITimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_LOG_DEBUG(log("promise settled") + ZS_PARAM("promise", promise->getID()))

      AutoRecursiveLock lock(*this);
      step();
      
      if (ZS_DYNAMIC_PTR_CAST(PromiseWithRTPMediaEngineChannelResource, promise)) {
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onSecureTransportState(ISecureTransport::States state)
    {
      ZS_LOG_TRACE(log("notified secure transport state") + ZS_PARAM("state", ISecureTransport::toString(state)))

      AutoRecursiveLock lock(*this);

      mTransportState = state;

      if (mChannelResource)
        mChannelResource->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onUpdate(ParametersPtr params)
    {
      ZS_LOG_TRACE(log("on update") + params->toDebug())

      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        mParameters = make_shared<Parameters>(*params);
        channelResource = mChannelResource;
      }

      if (channelResource)
        channelResource->notifyUpdate(params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::SendRtp(
                                        const uint8_t* packet,
                                        size_t length,
                                        const webrtc::PacketOptions& options
                                        )
    {
      auto channel = mSenderChannel.lock();
      if (!channel) return false;
      return channel->sendPacket(RTPPacket::create(packet, length));
    }
    
    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::SendRtcp(const uint8_t* packet, size_t length)
    {
      auto channel = mSenderChannel.lock();
      if (!channel) return false;
      return channel->sendPacket(RTCPPacket::create(packet, length));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio::Transport
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::Transport::Transport(
                                                const make_private &,
                                                RTPSenderChannelAudioPtr outer
                                                ) :
      mOuter(outer)
    {
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::TransportPtr RTPSenderChannelAudio::Transport::create(RTPSenderChannelAudioPtr outer)
    {
      TransportPtr pThis(make_shared<Transport>(make_private{}, outer));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio::Transport => webrtc::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::Transport::SendRtp(
                                                   const uint8_t* packet,
                                                   size_t length,
                                                   const webrtc::PacketOptions& options
                                                   )
    {
      auto outer = mOuter.lock();
      if (!outer) return false;
      return outer->SendRtp(packet, length, options);
    }
    
    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::Transport::SendRtcp(const uint8_t* packet, size_t length)
    {
      auto outer = mOuter.lock();
      if (!outer) return false;
      return outer->SendRtcp(packet, length);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelAudio::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSenderChannelAudio");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelAudio::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannelAudio::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSenderChannelAudio");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto senderChannel = mSenderChannel.lock();
      IHelper::debugAppend(resultEl, "sender channel", senderChannel ? senderChannel->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepSetupChannel()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::stepSetupChannel()
    {
      if (mChannelResource) {
        ZS_LOG_TRACE(log("already setup channel resource"))
        return true;
      }

      if (!mChannelResourceLifetimeHolderPromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for setup channel promise to be set up"))
        return false;
      }

      if (mChannelResourceLifetimeHolderPromise->isRejected()) {
        ZS_LOG_WARNING(Debug, log("media engine rejected channel setup"))
        cancel();
        return false;
      }

      mChannelResource = ZS_DYNAMIC_PTR_CAST(UseChannelResource, mChannelResourceLifetimeHolderPromise->value());

      if (!mChannelResource) {
        ZS_LOG_WARNING(Detail, log("failed to initialize channel resource"))
        cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media channel is setup") + ZS_PARAM("channel", mChannelResource->getID()))

      mChannelResource->notifyTransportState(mTransportState);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (!mCloseChannelPromise) {
        if (mChannelResource) {
          mCloseChannelPromise = mChannelResource->shutdown();
          mCloseChannelPromise->thenWeak(mGracefulShutdownReference);
        }
      }

      if (mGracefulShutdownReference) {
        if (mCloseChannelPromise) {
          if (!mCloseChannelPromise->isSettled()) {
            ZS_LOG_DEBUG(log("waiting for close channel promise"))
            return;
          }
        }
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      mChannelResourceLifetimeHolderPromise.reset();

      mChannelResource.reset();
      mCloseChannelPromise.reset();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderChannelAudioPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderChannelAudioStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }

      mLastError = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderChannelAudioFactory &IRTPSenderChannelAudioFactory::singleton()
    {
      return RTPSenderChannelAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr IRTPSenderChannelAudioFactory::create(
                                                                   RTPSenderChannelPtr senderChannel,
                                                                   MediaStreamTrackPtr track,
                                                                   const Parameters &params
                                                                   )
    {
      if (this) {}
      return internal::RTPSenderChannelAudio::create(senderChannel, track, params);
    }

  } // internal namespace

}
