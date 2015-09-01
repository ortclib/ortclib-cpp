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

#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <webrtc/modules/rtp_rtcp/interface/rtp_header_parser.h>
#include <webrtc/video/video_send_stream.h>


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
    void IRTPSenderForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForRTPListener
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderForRTPListener::toDebug(ForRTPListenerPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSender, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForRTPListener
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderForDTMFSender::toDebug(ForDTMFSenderPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSender, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPSender::toString(States state)
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
    RTPSender::RTPSender(
                         const make_private &,
                         IMessageQueuePtr queue,
                         IRTPSenderDelegatePtr delegate,
                         IMediaStreamTrackPtr track,
                         IRTPTransportPtr transport,
                         IRTCPTransportPtr rtcpTransport
                         ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mModuleProcessThread(webrtc::ProcessThread::Create()),
      mChannelGroup(new webrtc::ChannelGroup(mModuleProcessThread.get()))
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void RTPSender::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      mModuleProcessThread->Start();

      webrtc::newapi::Transport* transport = NULL;
      webrtc::CpuOveruseObserver* overuseObserver = NULL;
      int numCpuCores = 2;
      int channelID = 0;
      const webrtc::VideoSendStream::Config config;
      const webrtc::VideoEncoderConfig encoderConfig;
      const std::map<uint32_t, webrtc::RtpState> suspendedSSRCs;

      mVideoStream = rtc::scoped_ptr<webrtc::VideoSendStream>(new webrtc::internal::VideoSendStream(
        transport, overuseObserver, numCpuCores,
        mModuleProcessThread.get(), mChannelGroup.get(),
        channelID, config, encoderConfig,
        suspendedSSRCs));
    }

    //-------------------------------------------------------------------------
    RTPSender::~RTPSender()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(IRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(ForRTPListenerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(ForDTMFSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr RTPSender::getStats() const throw(InvalidStateError)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return PromiseWithStatsReportPtr();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPSender::toDebug(RTPSenderPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::create(
                                   IRTPSenderDelegatePtr delegate,
                                   IMediaStreamTrackPtr track,
                                   IRTPTransportPtr transport,
                                   IRTCPTransportPtr rtcpTransport
                                   )
    {
      RTPSenderPtr pThis(make_shared<RTPSender>(make_private {}, IORTCForInternal::queueORTC(), delegate, track, transport, rtcpTransport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IRTPSenderSubscriptionPtr RTPSender::subscribe(IRTPSenderDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to receiver"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IRTPSenderSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IRTPSenderDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        RTPSenderPtr pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RTPSender::track() const
    {
#define TODO 1
#define TODO 2
      return IMediaStreamTrackPtr();
    }

    //-------------------------------------------------------------------------
    IRTPTransportPtr RTPSender::transport() const
    {
#define TODO 1
#define TODO 2
      return IRTPTransportPtr();
    }

    //-------------------------------------------------------------------------
    IRTCPTransportPtr RTPSender::rtcpTransport() const
    {
#define TODO 1
#define TODO 2
      return IRTCPTransportPtr();
    }

    //-------------------------------------------------------------------------
    void RTPSender::setTransport(
                                 IRTPTransportPtr transport,
                                 IRTCPTransportPtr rtcpTransport
                                 )
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPSender::setTrack(IMediaStreamTrackPtr track)
    {
#define TODO 1
#define TODO 2
      return PromisePtr();
    }

    //-------------------------------------------------------------------------
    IRTPSenderTypes::CapabilitiesPtr RTPSender::getCapabilities(Optional<Kinds> kind)
    {
      CapabilitiesPtr result(make_shared<Capabilities>());
#define TODO 1
#define TODO 2
      return result;
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPSender::send(const Parameters &parameters)
    {
#define TODO 1
#define TODO 2
      return PromisePtr();
    }

    //-------------------------------------------------------------------------
    void RTPSender::stop()
    {
      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderForRTPListener
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderForDTMFSender
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onWake()
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
    #pragma mark RTPSender => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onTimer(TimerPtr timer)
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
    #pragma mark RTPSender => IRTPSenderAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPTypes::PacketReceiver
    #pragma mark

    IRTPTypes::PacketReceiver::DeliveryStatuses RTPSender::DeliverPacket(
                                                                         MediaTypes mediaType,
                                                                         const uint8_t* packet,
                                                                         size_t length
                                                                         )
    {
      if (webrtc::RtpHeaderParser::IsRtcp(packet, length))
        return DeliverRtcp(mediaType, packet, length);

      return DeliveryStatus_PacketError;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSender::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSender");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSender::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSender::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSender");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPSender::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSender::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSender::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepBogusDoSomething()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("dtls is not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool RTPSender::stepBogusDoSomething()
    {
      if ( /* step already done */ false ) {
        ZS_LOG_TRACE(log("already completed do something"))
        return true;
      }

      if ( /* cannot do step yet */ false) {
        ZS_LOG_DEBUG(log("waiting for XYZ to complete before continuing"))
        return false;
      }

      ZS_LOG_DEBUG(log("doing step XYZ"))

      // ....
#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSender::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
        webrtc::internal::VideoSendStream* sendStreamImpl =
          static_cast<webrtc::internal::VideoSendStream*>(mVideoStream.get());
        sendStreamImpl->Stop();
        mModuleProcessThread->Stop();
        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPSender::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSender::setError(WORD errorCode, const char *inReason)
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
    IRTPTypes::PacketReceiver::DeliveryStatuses RTPSender::DeliverRtcp(
                                                                       MediaTypes mediaType,
                                                                       const uint8_t* packet,
                                                                       size_t length
                                                                       )
    {
      bool rtcpDelivered = false;
      if (mediaType == MediaTypes::MediaType_Any || mediaType == MediaTypes::MediaType_Video) {
        webrtc::internal::VideoSendStream* sendStreamImpl =
          static_cast<webrtc::internal::VideoSendStream*>(mVideoStream.get());
        if (sendStreamImpl->DeliverRtcp(packet, length))
          rtcpDelivered = true;
      }
      return rtcpDelivered ? DeliveryStatus_OK : DeliveryStatus_PacketError;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderFactory &IRTPSenderFactory::singleton()
    {
      return RTPSenderFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr IRTPSenderFactory::create(
                                           IRTPSenderDelegatePtr delegate,
                                           IMediaStreamTrackPtr track,
                                           IRTPTransportPtr transport,
                                           IRTCPTransportPtr rtcpTransport
                                           )
    {
      if (this) {}
      return internal::RTPSender::create(delegate, track, transport, rtcpTransport);
    }

    //-------------------------------------------------------------------------
    IRTPSenderFactory::CapabilitiesPtr IRTPSenderFactory::getCapabilities(Optional<Kinds> kind)
    {
      if (this) {}
      return RTPSender::getCapabilities(kind);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSender
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPSender::toDebug(IRTPSenderPtr transport)
  {
    return internal::RTPSender::toDebug(internal::RTPSender::convert(transport));
  }

  //---------------------------------------------------------------------------
  IRTPSenderPtr IRTPSender::create(
                                   IRTPSenderDelegatePtr delegate,
                                   IMediaStreamTrackPtr track,
                                   IRTPTransportPtr transport,
                                   IRTCPTransportPtr rtcpTransport
                                   )
  {
    return internal::IRTPSenderFactory::singleton().create(delegate, track, transport, rtcpTransport);
  }

  //---------------------------------------------------------------------------
  IRTPSenderTypes::CapabilitiesPtr IRTPSender::getCapabilities(Optional<Kinds> kind)
  {
    return internal::IRTPSenderFactory::singleton().getCapabilities(kind);
  }

}
