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

#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <webrtc/voice_engine/include/voe_network.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpreceiver) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPReceiverChannelAudioSettingsDefaults);

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
    #pragma mark RTPReceiverChannelAudioSettingsDefaults
    #pragma mark

    class RTPReceiverChannelAudioSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPReceiverChannelAudioSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPReceiverChannelAudioSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPReceiverChannelAudioSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPReceiverChannelAudioSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPReceiverChannelAudioSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        //      UseSettings::setUInt(ORTC_SETTING_RTP_RECEIVER_CHANNEL_AUDIO_, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPReceiverChannelAudioSettingsDefaults()
    {
      RTPReceiverChannelAudioSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelAudioForRTPReceiverChannel::toDebug(ForRTPReceiverChannelPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr IRTPReceiverChannelAudioForRTPReceiverChannel::create(
                                                                                     RTPReceiverChannelPtr receiverChannel,
                                                                                     MediaStreamTrackPtr track,
                                                                                     const Parameters &params
                                                                                     )
    {
      return internal::IRTPReceiverChannelAudioFactory::singleton().create(receiverChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelAudioForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object)->toDebug();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiverChannelAudio::toString(States state)
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
    RTPReceiverChannelAudio::RTPReceiverChannelAudio(
                                                     const make_private &,
                                                     IMessageQueuePtr queue,
                                                     UseChannelPtr receiverChannel,
                                                     UseMediaStreamTrackPtr track,
                                                     const Parameters &params
                                                     ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mReceiverChannel(receiverChannel),
      mTrack(track),
      mParameters(make_shared<Parameters>(params))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!receiverChannel)
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::init()
    {
      AutoRecursiveLock lock(*this);

      mTransport = Transport::create(mThisWeak.lock());

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::~RTPReceiverChannelAudio()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForReceiverChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForRTPReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForRTPMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::notifyTransportState(ISecureTransportTypes::States state)
    {
      IRTPReceiverChannelAudioAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::notifyUpdate(ParametersPtr params)
    {
      IRTPReceiverChannelAudioAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(params);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::handlePacket(RTPPacketPtr packet)
    {
      UseChannelResourcePtr channelResource;

      {
        AutoRecursiveLock lock(*this);
        if (isShuttingDown() || isShutdown())
          return false;
        channelResource = mChannelResource;
        bool shouldQueue = false;
        if (!channelResource)
          shouldQueue = true;
        if (mQueuedRTP.size() > 0)
          shouldQueue = true;
        if (shouldQueue) {
          mQueuedRTP.push(packet);
          if (1 == mQueuedRTP.size())
            IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
          return true;
        }
      }

      if (!channelResource) return true;
      return channelResource->handlePacket(*packet);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::handlePacket(RTCPPacketPtr packet)
    {
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        if (mQueuedRTP.size() > 0) {
          mQueuedRTCP.push(packet);
          return true;
        }
        channelResource = mChannelResource;
      }

      if (!channelResource) return false;
      return channelResource->handlePacket(*packet);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioForRTPReceiverChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::create(
                                                               RTPReceiverChannelPtr receiverChannel,
                                                               MediaStreamTrackPtr track,
                                                               const Parameters &params
                                                               )
    {
      RTPReceiverChannelAudioPtr pThis(make_shared<RTPReceiverChannelAudio>(make_private {}, IORTCForInternal::queueORTC(), receiverChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    int32_t RTPReceiverChannelAudio::getAudioSamples(
                                                     const size_t numberOfSamples,
                                                     const uint8_t numberOfChannels,
                                                     void *audioSamples,
                                                     size_t& numberOfSamplesOut
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
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelMediaBaseForRTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onTimer(ITimerPtr timer)
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
    #pragma mark RTPReceiverChannelAudio => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_LOG_DEBUG(log("promise settled") + ZS_PARAM("promise", promise->getID()))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onReceiverChannelAudioDeliverPackets()
    {
      RTPPacketQueue rtpPackets;
      RTCPPacketQueue rtcpPackets;
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        rtpPackets = mQueuedRTP;
        rtcpPackets = mQueuedRTCP;
        mQueuedRTP = RTPPacketQueue();
        mQueuedRTCP = RTCPPacketQueue();
        channelResource = mChannelResource;
      }

      if (!channelResource)
        return;

      while (rtpPackets.size() > 0) {
        auto packet = rtpPackets.front();
        channelResource->handlePacket(*packet);
        rtpPackets.pop();
      }

      while (rtcpPackets.size() > 0) {
        auto packet = rtcpPackets.front();
        channelResource->handlePacket(*packet);
        rtcpPackets.pop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onSecureTransportState(ISecureTransport::States state)
    {
      ZS_LOG_TRACE(log("notified secure transport state") + ZS_PARAM("state", ISecureTransport::toString(state)))

      AutoRecursiveLock lock(*this);

      mTransportState = state;

      if (mChannelResource)
        mChannelResource->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onUpdate(ParametersPtr params)
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
    #pragma mark RTPReceiverChannelAudio => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::SendRtcp(const uint8_t* packet, size_t length)
    {
      auto channel = mReceiverChannel.lock();
      if (!channel) return false;
      return channel->sendPacket(RTCPPacket::create(packet, length));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::Transport::Transport(
                                                  const make_private &,
                                                  RTPReceiverChannelAudioPtr outer
                                                  ) :
      mOuter(outer)
    {
    }
        
    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::TransportPtr RTPReceiverChannelAudio::Transport::create(RTPReceiverChannelAudioPtr outer)
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
    #pragma mark RTPReceiverChannelAudio::Transport => webrtc::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::Transport::SendRtp(
                                                     const uint8_t* packet,
                                                     size_t length,
                                                     const webrtc::PacketOptions& options
                                                     )
    {
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::Transport::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPReceiverChannelAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelAudio::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiverChannelAudio");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelAudio::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiverChannelAudio::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPReceiverChannelAudio");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto receiverChannel = mReceiverChannel.lock();
      IHelper::debugAppend(resultEl, "receiver channel", receiverChannel ? receiverChannel->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepChannelPromise()) goto not_ready;
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
    bool RTPReceiverChannelAudio::stepChannelPromise()
    {
      if (mChannelResourceLifetimeHolderPromise) {
        ZS_LOG_TRACE(log("already setup channel promise"))
          return true;
      }

      if (mQueuedRTP.size() < 1) {
        ZS_LOG_TRACE(log("cannot setup channel promise until first packet is received"))
          return false;
      }

      auto packet = mQueuedRTP.front();

      mChannelResourceLifetimeHolderPromise = UseMediaEngine::setupChannel(
                                                                           mThisWeak.lock(),
                                                                           mTransport,
                                                                           MediaStreamTrack::convert(mTrack),
                                                                           mParameters,
                                                                           packet
                                                                           );

      mChannelResourceLifetimeHolderPromise->thenWeak(mThisWeak.lock());

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::stepSetupChannel()
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

      IRTPReceiverChannelAudioAsyncDelegateProxy::create(mThisWeak.lock())->onReceiverChannelAudioDeliverPackets();

      mChannelResource->notifyTransportState(mTransportState);

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

      mQueuedRTP = RTPPacketQueue();
      mQueuedRTCP = RTCPPacketQueue();

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
    void RTPReceiverChannelAudio::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPReceiverChannelAudioPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPReceiverChannelAudioStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IRTPReceiverChannelAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPReceiverChannelAudioFactory &IRTPReceiverChannelAudioFactory::singleton()
    {
      return RTPReceiverChannelAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr IRTPReceiverChannelAudioFactory::create(
                                                                       RTPReceiverChannelPtr receiverChannel,
                                                                       MediaStreamTrackPtr track,
                                                                       const Parameters &params
                                                                       )
    {
      if (this) {}
      return internal::RTPReceiverChannelAudio::create(receiverChannel, track, params);
    }

  } // internal namespace
}
