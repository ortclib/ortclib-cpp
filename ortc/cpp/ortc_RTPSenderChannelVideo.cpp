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

#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <webrtc/system_wrappers/include/cpu_info.h>

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
    ZS_DECLARE_CLASS_PTR(RTPSenderChannelVideoSettingsDefaults);

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
    #pragma mark RTPSenderChannelVideoSettingsDefaults
    #pragma mark

    class RTPSenderChannelVideoSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPSenderChannelVideoSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPSenderChannelVideoSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPSenderChannelVideoSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPSenderChannelVideoSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPSenderChannelVideoSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        //      ISettings::setUInt(ORTC_SETTING_RTP_SENDER_CHANNEL_VIDEO_, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPSenderChannelVideoSettingsDefaults()
    {
      RTPSenderChannelVideoSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr IRTPSenderChannelVideoForRTPSenderChannel::create(
                                                                               RTPSenderChannelPtr senderChannel,
                                                                               MediaStreamTrackPtr track,
                                                                               const Parameters &params
                                                                               )
    {
      return internal::IRTPSenderChannelVideoFactory::singleton().create(senderChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo
    #pragma mark
    
    //-------------------------------------------------------------------------
    const char *RTPSenderChannelVideo::toString(States state)
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
    RTPSenderChannelVideo::RTPSenderChannelVideo(
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
    void RTPSenderChannelVideo::init()
    {
      TransportPtr transport = Transport::create(mThisWeak.lock());

      PromiseWithRTPMediaEngineChannelResourcePtr setupChannelPromise = UseMediaEngine::setupChannel(
                                                                                                     mThisWeak.lock(),
                                                                                                     transport,
                                                                                                     MediaStreamTrack::convert(mTrack),
                                                                                                     mParameters,
                                                                                                     IDTMFSenderDelegatePtr()
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
    RTPSenderChannelVideo::~RTPSenderChannelVideo()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForRTPSenderChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForRTPSenderChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForRTPMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onTrackChanged(UseBaseMediaStreamTrackPtr track)
    {
      AutoRecursiveLock lock(*this);
      mTrack = MediaStreamTrack::convert(track);  // NOTE: track might be NULL
#define TODO_HANDLE_CHANGE_IN_TRACK 1
#define TODO_HANDLE_CHANGE_IN_TRACK 2
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::notifyTransportState(ISecureTransportTypes::States state)
    {
      IRTPSenderChannelVideoAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::notifyUpdate(ParametersPtr params)
    {
      IRTPSenderChannelVideoAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(params);
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::handlePacket(RTCPPacketPtr packet)
    {
      UseChannelResourcePtr channelResource;
      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
      }

      if (!channelResource) return true;
      return channelResource->handlePacket(*packet);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
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
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoForRTPSenderChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::create(
                                                           RTPSenderChannelPtr senderChannel,
                                                           MediaStreamTrackPtr track,
                                                           const Parameters &params
                                                           )
    {
      RTPSenderChannelVideoPtr pThis(make_shared<RTPSenderChannelVideo>(make_private {}, IORTCForInternal::queueORTC(), senderChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::sendVideoFrame(VideoFramePtr videoFrame)
    {
      UseChannelResourcePtr channelResource;

      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
      }

      if (!channelResource) return;
      channelResource->sendVideoFrame(videoFrame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onWake()
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
    #pragma mark RTPSenderChannelVideo => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onTimer(ITimerPtr timer)
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
    #pragma mark RTPSenderChannelVideo => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onPromiseSettled(PromisePtr promise)
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
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onSecureTransportState(ISecureTransport::States state)
    {
      ZS_LOG_TRACE(log("notified secure transport state") + ZS_PARAM("state", ISecureTransport::toString(state)))

      AutoRecursiveLock lock(*this);

      mTransportState = state;

      if (mChannelResource)
        mChannelResource->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onUpdate(ParametersPtr params)
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
    #pragma mark RTPSenderChannelVideo => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::SendRtp(
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
    bool RTPSenderChannelVideo::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPSenderChannelVideo::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::Transport::Transport(
                                                const make_private &,
                                                RTPSenderChannelVideoPtr outer
                                                ) :
      mOuter(outer)
    {
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::TransportPtr RTPSenderChannelVideo::Transport::create(RTPSenderChannelVideoPtr outer)
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
    #pragma mark RTPSenderChannelVideo::Transport => webrtc::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::Transport::SendRtp(
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
    bool RTPSenderChannelVideo::Transport::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPSenderChannelVideo => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelVideo::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSenderChannelVideo");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelVideo::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannelVideo::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSenderChannelVideo");

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
    bool RTPSenderChannelVideo::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::step()
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
    bool RTPSenderChannelVideo::stepSetupChannel()
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

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::cancel()
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
    void RTPSenderChannelVideo::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderChannelVideoPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderChannelVideoStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IRTPSenderChannelVideoFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderChannelVideoFactory &IRTPSenderChannelVideoFactory::singleton()
    {
      return RTPSenderChannelVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr IRTPSenderChannelVideoFactory::create(
                                                                   RTPSenderChannelPtr senderChannel,
                                                                   MediaStreamTrackPtr track,
                                                                   const Parameters &params
                                                                   )
    {
      if (this) {}
      return internal::RTPSenderChannelVideo::create(senderChannel, track, params);
    }

  } // internal namespace

}
