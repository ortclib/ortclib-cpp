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

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

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


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
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
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IRTPSenderChannelVideoForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
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
                                                                                                     mParameters
                                                                                                     );
      {
        AutoRecursiveLock lock(*this);
        mSetupChannelPromise = setupChannelPromise;
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
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
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
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 1
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 2
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

      channelResource->getStream()->DeliverRtcp(packet->buffer()->data(), packet->buffer()->size());
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::handleUpdate(ParametersPtr params)
    {
#define TODO_UPDATE_PARAMETERS 1
#define TODO_UPDATE_PARAMETERS 2
      {
        AutoRecursiveLock lock(*this);
        mParameters = make_shared<Parameters>(*params);
      }
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
    void RTPSenderChannelVideo::sendVideoFrame(const webrtc::VideoFrame& videoFrame)
    {
      UseChannelResourcePtr channelResource;

      {
        AutoRecursiveLock lock(*this);
        channelResource = mChannelResource;
      }

      if (!channelResource) return;
      channelResource->getStream()->Input()->IncomingCapturedFrame(videoFrame);
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
    void RTPSenderChannelVideo::onTimer(TimerPtr timer)
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
    void RTPSenderChannelVideo::onPromiseSettled(PromisePtr promise)
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
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoAsyncDelegate
    #pragma mark

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
      UseServicesHelper::debugAppend(objectEl, "id", mID);
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

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto senderChannel = mSenderChannel.lock();
      UseServicesHelper::debugAppend(resultEl, "sender channel", senderChannel ? senderChannel->getID() : 0);

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
      if (!stepPromiseEngine()) goto not_ready;
      if (!stepPromiseExampleDeviceResource()) goto not_ready;
      if (!stepSetupChannel()) goto not_ready;
      if (!stepCloseChannel()) goto not_ready;
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
    bool RTPSenderChannelVideo::stepPromiseEngine()
    {
      if (mMediaEngine) {
        ZS_LOG_TRACE(log("already setup engine"))
          return true;
      }

      if (!mMediaEnginePromise) {
        mMediaEnginePromise = UseMediaEngine::create();
      }

      if (!mMediaEnginePromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for media engine promise to resolve"))
          return false;
      }

      mMediaEngine = mMediaEnginePromise->value();

      if (!mMediaEngine) {
        ZS_LOG_WARNING(Detail, log("failed to initialize media"))
          cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media engine is setup") + ZS_PARAM("engine", mMediaEngine->getID()))
        return true;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::stepPromiseExampleDeviceResource()
    {
      if (mDeviceResource) {
        ZS_LOG_TRACE(log("already setup device resource"))
          return true;
      }

      if (!mDeviceResourcePromise) {
        mDeviceResourcePromise = UseMediaEngine::getDeviceResource("camera");
      }

      if (!mDeviceResourcePromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for media device resource promise to resolve"))
          return false;
      }

      if (mSetupChannelPromise->isRejected()) {
        ZS_LOG_WARNING(Debug, log("media engine rejected device setup"))
        cancel();
        return false;
      }

      mDeviceResource = mDeviceResourcePromise->value();

      if (!mDeviceResource) {
        ZS_LOG_WARNING(Detail, log("failed to initialize device resource"))
          cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media device is setup") + ZS_PARAM("device", mDeviceResource->getDeviceID()))
        return true;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::stepSetupChannel()
    {
      if (mChannelResource) {
        ZS_LOG_TRACE(log("already setup channel resource"))
        return true;
      }

      if (!mSetupChannelPromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for setup channel promise to be set up"))
        return false;
      }

      if (mSetupChannelPromise->isRejected()) {
        ZS_LOG_WARNING(Debug, log("media engine rejected channel setup"))
        cancel();
        return false;
      }

      mChannelResource = ZS_DYNAMIC_PTR_CAST(UseChannelResource, mSetupChannelPromise->value());

      if (!mChannelResource) {
        ZS_LOG_WARNING(Detail, log("failed to initialize channel resource"))
        cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media channel is setup") + ZS_PARAM("channel", mChannelResource->getChannelID()))

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::stepCloseChannel()
    {
      if (!mCloseChannelPromise) {
        ZS_LOG_TRACE(log("waiting for close channel promise"))
        return true;
      }

      if (!mSetupChannelPromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for close channel promise to be set up"))
        return false;
      }

      if (mSetupChannelPromise->isRejected()) {
        ZS_LOG_WARNING(Debug, log("media engine rejected channel close"))
        cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media channel is closed") + ZS_PARAM("channel", mChannelResource->getChannelID()))

      cancel();

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
        mCloseChannelPromise = UseMediaEngine::closeChannel(*this);
        mCloseChannelPromise->thenWeak(mGracefulShutdownReference);
      }

      if (mGracefulShutdownReference) {
        if (!mCloseChannelPromise->isSettled())
          return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

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
