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

#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_RTPListener.h>
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

#include <webrtc/modules/rtp_rtcp/interface/rtp_header_parser.h>
#include <webrtc/modules/rtp_rtcp/source/byte_io.h>
#include <webrtc/video/video_receive_stream.h>
#include <webrtc/video_renderer.h>


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
    void IRTPReceiverForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverForRTPListener
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverForRTPListener::toDebug(ForRTPListenerPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverForRTPReceiverChannel::toDebug(ForRTPReceiverChannelPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiver::toString(States state)
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
    RTPReceiver::RTPReceiver(
                             const make_private &,
                             IMessageQueuePtr queue,
                             IRTPReceiverDelegatePtr delegate,
                             IRTPTransportPtr transport,
                             IRTCPTransportPtr rtcpTransport
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_LOG_DETAIL(debug("created"))

      mListener = UseListener::getListener(transport);
      ORTC_THROW_INVALID_PARAMETERS_IF(!mListener)

      UseSecureTransport::getReceivingTransport(transport, rtcpTransport, mReceiveRTPOverTransport, mReceiveRTCPOverTransport, mRTPTransport, mRTCPTransport);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPReceiver::~RTPReceiver()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(IRTPReceiverPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(ForRTPListenerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr RTPReceiver::getStats() const throw(InvalidStateError)
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
    #pragma mark RTPReceiver => IRTPReceiver
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::toDebug(RTPReceiverPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::create(
                                       IRTPReceiverDelegatePtr delegate,
                                       IRTPTransportPtr transport,
                                       IRTCPTransportPtr rtcpTransport
                                       )
    {
      RTPReceiverPtr pThis(make_shared<RTPReceiver>(make_private {}, IORTCForInternal::queueORTC(), delegate, transport, rtcpTransport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IRTPReceiverSubscriptionPtr RTPReceiver::subscribe(IRTPReceiverDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to receiver"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IRTPReceiverSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IRTPReceiverDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        RTPReceiverPtr pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RTPReceiver::track() const
    {
      return IMediaStreamTrackPtr(MediaStreamTrack::convert(mVideoTrack));
    }

    //-------------------------------------------------------------------------
    IRTPTransportPtr RTPReceiver::transport() const
    {
#define TODO 1
#define TODO 2
      return IRTPTransportPtr();
    }

    //-------------------------------------------------------------------------
    IRTCPTransportPtr RTPReceiver::rtcpTransport() const
    {
#define TODO 1
#define TODO 2
      return IRTCPTransportPtr();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::setTransport(
                                   IRTPTransportPtr transport,
                                   IRTCPTransportPtr rtcpTransport
                                   )
    {
      typedef UseListener::RTCPPacketList RTCPPacketList;

      AutoRecursiveLock lock(*this);

      UseListenerPtr listener = UseListener::getListener(transport);
      ORTC_THROW_INVALID_PARAMETERS_IF(!listener)

      if (listener->getID() == mListener->getID()) {
        ZS_LOG_TRACE(log("transport has not changed (noop)"))
        return;
      }

      if (mParameters) {
        // unregister from old listener
        mListener->unregisterReceiver(*this);

        // register to new listener
        RTCPPacketList historicalRTCPPackets;
        mListener->registerReceiver(mThisWeak.lock(), *mParameters, historicalRTCPPackets);

#define TODO_PROCESS_HISTORICAL_RTCP_PACKETS_FROM_NEW_TRANSPORT 1
#define TODO_PROCESS_HISTORICAL_RTCP_PACKETS_FROM_NEW_TRANSPORT 2
      }

      UseSecureTransport::getReceivingTransport(transport, rtcpTransport, mReceiveRTPOverTransport, mReceiveRTCPOverTransport, mRTPTransport, mRTCPTransport);
    }

    //-------------------------------------------------------------------------
    IRTPReceiverTypes::CapabilitiesPtr RTPReceiver::getCapabilities(Optional<Kinds> kind)
    {
      CapabilitiesPtr result(make_shared<Capabilities>());
#define TODO 1
#define TODO 2
      return result;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::receive(const Parameters &parameters)
    {
      typedef UseListener::RTCPPacketList RTCPPacketList;

      AutoRecursiveLock lock(*this);

      if (mParameters) {
        auto hash = parameters.hash();
        auto previousHash = mParameters->hash();
        if (hash == previousHash) {
          ZS_LOG_TRACE(log("receive has not changed (noop)"))
          return;
        }
      }

      mParameters = make_shared<Parameters>(parameters);

      RTCPPacketList historicalRTCPPackets;
      mListener->registerReceiver(mThisWeak.lock(), *mParameters, historicalRTCPPackets);

#define TODO_PROCESS_HISTORICAL_RTCP_PACKETS_FROM_NEW_TRANSPORT 1
#define TODO_PROCESS_HISTORICAL_RTCP_PACKETS_FROM_NEW_TRANSPORT 2
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::stop()
    {
      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);

      cancel();
    }

    //-------------------------------------------------------------------------
    IRTPReceiverTypes::ContributingSourceList RTPReceiver::getContributingSources() const
    {
#define TODO 1
#define TODO 2
      return ContributingSourceList();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::requestSendCSRC(SSRCType csrc)
    {
      ZS_THROW_NOT_IMPLEMENTED("solely used by the H.264/UC codec; for a receiver to request an SSRC from a sender (not implemented by this client)")
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IRTPReceiverForRTPListener
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiver::handlePacket(
                                   IICETypes::Components viaTransport,
                                   RTPPacketPtr packet
                                   )
    {
      ZS_LOG_TRACE(log("received packet") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + packet->toDebug())

      {
        AutoRecursiveLock lock(*this);
        // process packet here
#define TOOD_PROCESS_PACKET_HERE 1
#define TOOD_PROCESS_PACKET_HERE 2
      }
      return false; // return true if packet was handled
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::handlePacket(
                                   IICETypes::Components viaTransport,
                                   RTCPPacketPtr packet
                                   )
    {
      ZS_LOG_TRACE(log("received packet") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + packet->toDebug())

      {
        AutoRecursiveLock lock(*this);
        // process packet here
#define TOOD_PROCESS_PACKET_HERE 1
#define TOOD_PROCESS_PACKET_HERE 2
      }

      return true; // return true if handled
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IRTPReceiverForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiver::sendPacket(RTCPPacketPtr packet)
    {
#define TODO 1
#define TODO 2
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiver::onWake()
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
    #pragma mark RTPReceiver => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiver::onTimer(TimerPtr timer)
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
    #pragma mark RTPReceiver => IRTPReceiverAsyncDelegate
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiver::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiver");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiver::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPReceiver");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "listener", mListener ? mListener->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::step()
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
    bool RTPReceiver::stepBogusDoSomething()
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
    void RTPReceiver::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
//        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      if (mParameters) {
        mListener->unregisterReceiver(*this);
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPReceiverPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPReceiverStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::setError(WORD errorCode, const char *inReason)
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
    bool RTPReceiver::sendPacket(
                                 IICETypes::Components packetType,
                                 const BYTE *buffer,
                                 size_t bufferSizeInBytes
                                 )
    {
      IICETypes::Components sendOver{ packetType };
      UseSecureTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);

        switch (packetType) {
        case IICETypes::Component_RTP:  {
          transport = mRTPTransport;
          sendOver = mReceiveRTPOverTransport;
          break;
        }
        case IICETypes::Component_RTCP: {
          transport = mRTCPTransport;
          sendOver = mReceiveRTCPOverTransport;
          break;
        }
        }
      }

      if (!transport) {
        ZS_LOG_WARNING(Debug, log("no transport available"))
          return false;
      }

      return transport->sendPacket(sendOver, packetType, buffer, bufferSizeInBytes);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPReceiverFactory &IRTPReceiverFactory::singleton()
    {
      return RTPReceiverFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr IRTPReceiverFactory::create(
                                               IRTPReceiverDelegatePtr delegate,
                                               IRTPTransportPtr transport,
                                               IRTCPTransportPtr rtcpTransport
                                               )
    {
      if (this) {}
      return internal::RTPReceiver::create(delegate, transport, rtcpTransport);
    }

    //-------------------------------------------------------------------------
    IRTPReceiverFactory::CapabilitiesPtr IRTPReceiverFactory::getCapabilities(Optional<Kinds> kind)
    {
      if (this) {}
      return RTPReceiver::getCapabilities(kind);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiverTypes::ContributingSource
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPReceiverTypes::ContributingSource::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPReceiverTypes::Capabilities");

    UseServicesHelper::debugAppend(resultEl, "timestamp", mTimestamp);
    UseServicesHelper::debugAppend(resultEl, "csrc", mCSRC);
    UseServicesHelper::debugAppend(resultEl, "audio level", mAudioLevel);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPReceiverTypes::ContributingSource::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IRTPReceiverTypes:ContributingSource:");
    hasher.update(mTimestamp);
    hasher.update(":");
    hasher.update(mCSRC);
    hasher.update(":");
    hasher.update(mAudioLevel);
    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiver
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPReceiver::toDebug(IRTPReceiverPtr transport)
  {
    return internal::RTPReceiver::toDebug(internal::RTPReceiver::convert(transport));
  }

  //---------------------------------------------------------------------------
  IRTPReceiverPtr IRTPReceiver::create(
                                       IRTPReceiverDelegatePtr delegate,
                                       IRTPTransportPtr transport,
                                       IRTCPTransportPtr rtcpTransport
                                       )
  {
    return internal::IRTPReceiverFactory::singleton().create(delegate, transport, rtcpTransport);
  }

  //---------------------------------------------------------------------------
  IRTPReceiverTypes::CapabilitiesPtr IRTPReceiver::getCapabilities(Optional<Kinds> kind)
  {
    return internal::IRTPReceiverFactory::singleton().getCapabilities(kind);
  }

}
