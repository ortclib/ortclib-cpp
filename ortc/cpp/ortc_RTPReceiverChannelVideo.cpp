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

#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
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

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpreceiver) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPReceiverChannelVideoSettingsDefaults);

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
    #pragma mark DTMFSenderSettingsDefaults
    #pragma mark

    class RTPReceiverChannelVideoSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPReceiverChannelVideoSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPReceiverChannelVideoSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPReceiverChannelVideoSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPReceiverChannelVideoSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPReceiverChannelVideoSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPReceiverChannelVideoSettingsDefaults()
    {
      RTPReceiverChannelVideoSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelVideoForRTPReceiverChannel::toDebug(ForRTPReceiverChannelPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr IRTPReceiverChannelVideoForRTPReceiverChannel::create(
                                                                                     RTPReceiverChannelPtr receiverChannel,
                                                                                     MediaStreamTrackPtr track,
                                                                                     const Parameters &params
                                                                                     )
    {
      return internal::IRTPReceiverChannelVideoFactory::singleton().create(receiverChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelVideoForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiverChannelVideo::toString(States state)
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
    RTPReceiverChannelVideo::RTPReceiverChannelVideo(
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
    void RTPReceiverChannelVideo::init()
    {
      AutoRecursiveLock lock(*this);

      mTransport = Transport::create(mThisWeak.lock());

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::~RTPReceiverChannelVideo()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForReceiverChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForRTPReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForRTPMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::notifyTransportState(ISecureTransportTypes::States state)
    {
      IRTPReceiverChannelVideoAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::notifyUpdate(ParametersPtr params)
    {
      IRTPReceiverChannelVideoAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(params);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::handlePacket(RTPPacketPtr packet)
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

      if (!channelResource) return false;
      return channelResource->handlePacket(*packet);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::handlePacket(RTCPPacketPtr packet)
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
    void RTPReceiverChannelVideo::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
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
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoForRTPReceiverChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::create(
                                                               RTPReceiverChannelPtr receiverChannel,
                                                               MediaStreamTrackPtr track,
                                                               const Parameters &params
                                                               )
    {
      RTPReceiverChannelVideoPtr pThis(make_shared<RTPReceiverChannelVideo>(make_private {}, IORTCForInternal::queueORTC(), receiverChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onWake()
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
    #pragma mark RTPReceiverChannelVideo => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onTimer(ITimerPtr timer)
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
    #pragma mark RTPReceiverChannelVideo => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onPromiseSettled(PromisePtr promise)
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
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onReceiverChannelVideoDeliverPackets()
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
    void RTPReceiverChannelVideo::onSecureTransportState(ISecureTransport::States state)
    {
      ZS_LOG_TRACE(log("notified secure transport state") + ZS_PARAM("state", ISecureTransport::toString(state)))

      AutoRecursiveLock lock(*this);

      mTransportState = state;

      if (mChannelResource)
        mChannelResource->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onUpdate(ParametersPtr params)
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
    #pragma mark RTPReceiverChannelVideo => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPReceiverChannelVideo::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::Transport::Transport(
                                                  const make_private &,
                                                  RTPReceiverChannelVideoPtr outer
                                                  ) :
      mOuter(outer)
    {
    }
        
    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::TransportPtr RTPReceiverChannelVideo::Transport::create(RTPReceiverChannelVideoPtr outer)
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
    #pragma mark RTPReceiverChannelVideo::Transport => webrtc::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::Transport::SendRtp(
                                                     const uint8_t* packet,
                                                     size_t length,
                                                     const webrtc::PacketOptions& options
                                                     )
    {
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::Transport::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPReceiverChannelVideo => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelVideo::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiverChannelVideo");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelVideo::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiverChannelVideo::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPReceiverChannelVideo");

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
    bool RTPReceiverChannelVideo::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::step()
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
    bool RTPReceiverChannelVideo::stepChannelPromise()
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
    bool RTPReceiverChannelVideo::stepSetupChannel()
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

      IRTPReceiverChannelVideoAsyncDelegateProxy::create(mThisWeak.lock())->onReceiverChannelVideoDeliverPackets();

      mChannelResource->notifyTransportState(mTransportState);

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::cancel()
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
    void RTPReceiverChannelVideo::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPReceiverChannelVideoPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPReceiverChannelVideoStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IRTPReceiverChannelVideoFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPReceiverChannelVideoFactory &IRTPReceiverChannelVideoFactory::singleton()
    {
      return RTPReceiverChannelVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr IRTPReceiverChannelVideoFactory::create(
                                                                       RTPReceiverChannelPtr receiverChannel,
                                                                       MediaStreamTrackPtr track,
                                                                       const Parameters &params
                                                                       )
    {
      if (this) {}
      return internal::RTPReceiverChannelVideo::create(receiverChannel, track, params);
    }

  } // internal namespace
}
