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

#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_SRTPSDESTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/SafeInt.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


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

  ZS_DECLARE_INTERACTION_TEAR_AWAY(IRTPListener, internal::RTPListener::TearAwayData)

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
    void IRTPListenerForSettings::applyDefaults()
    {
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTCP_PACKETS_IN_BUFFER, 100);
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS, 30);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPListenerForSecureTransport::toDebug(ForSecureTransportPtr listener)
    {
      if (!listener) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPListener, listener)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr IRTPListenerForSecureTransport::create(IRTPTransportPtr transport)
    {
      return IRTPListenerFactory::singleton().create(transport);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPListenerForRTPReceiver::toDebug(ForRTPReceiverPtr listener)
    {
      if (!listener) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPListener, listener)->toDebug();
    }

    //-------------------------------------------------------------------------
    IRTPListenerForRTPReceiver::ForRTPReceiverPtr IRTPListenerForRTPReceiver::getListener(IRTPTransportPtr rtpTransport)
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPListener, UseSecureTransport)

      ORTC_THROW_INVALID_PARAMETERS_IF(!rtpTransport)

      UseSecureTransportPtr secureTransport;

      {
        auto result = IDTLSTransport::convert(rtpTransport);
        if (result) {
          secureTransport = DTLSTransport::convert(result);
          if (secureTransport) return secureTransport->getListener();
        }
      }

      {
        auto result = ISRTPSDESTransport::convert(rtpTransport);
        if (result) {
          secureTransport = SRTPSDESTransport::convert(result);
          if (secureTransport) return secureTransport->getListener();
        }
      }

      return ForRTPReceiverPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPListenerForRTPSender::toDebug(ForRTPSenderPtr listener)
    {
      if (!listener) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPListener, listener)->toDebug();
    }

    //-------------------------------------------------------------------------
    IRTPListenerForRTPSender::ForRTPSenderPtr IRTPListenerForRTPSender::getListener(IRTPTransportPtr rtpTransport)
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPListener, UseSecureTransport)

      ORTC_THROW_INVALID_PARAMETERS_IF(!rtpTransport)

      UseSecureTransportPtr secureTransport;

      {
        auto result = IDTLSTransport::convert(rtpTransport);
        if (result) {
          secureTransport = DTLSTransport::convert(result);
          if (secureTransport) return secureTransport->getListener();
        }
      }

      {
        auto result = ISRTPSDESTransport::convert(rtpTransport);
        if (result) {
          secureTransport = SRTPSDESTransport::convert(result);
          if (secureTransport) return secureTransport->getListener();
        }
      }

      return ForRTPSenderPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPListener::toString(States state)
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
    RTPListener::RTPListener(
                             const make_private &,
                             IMessageQueuePtr queue,
                             IRTPListenerDelegatePtr originalDelegate,
                             IRTPTransportPtr transport
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mRTPTransport(transport),
      mMaxBufferedRTCPPackets(SafeInt<decltype(mMaxBufferedRTCPPackets)>(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTCP_PACKETS_IN_BUFFER))),
      mMaxRTCPPacketAge(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS))
    {
      ZS_LOG_DETAIL(debug("created"))

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPListener::~RTPListener()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr RTPListener::convert(IRTPListenerPtr object)
    {
      IRTPListenerPtr original = IRTPListenerTearAway::original(object);
      return ZS_DYNAMIC_PTR_CAST(RTPListener, original);
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr RTPListener::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPListener, object);
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr RTPListener::convert(ForSecureTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPListener, object);
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr RTPListener::convert(ForRTPReceiverPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPListener, object);
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr RTPListener::convert(ForRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPListener, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => IRTPListener
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPListener::toDebug(RTPListenerPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    IRTPListenerPtr RTPListener::create(
                                        IRTPListenerDelegatePtr delegate,
                                        IRTPTransportPtr transport,
                                        Optional<HeaderExtensionParametersList> headerExtensions
                                        )
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport)

      auto useSecureTransport = UseSecureTransport::convert(transport);
      ASSERT(((bool)useSecureTransport))

      auto listener = useSecureTransport->getListener();
      ORTC_THROW_INVALID_STATE_IF(!listener)

      auto tearAway = IRTPListenerTearAway::create(listener, make_shared<TearAwayData>());
      ORTC_THROW_INVALID_STATE_IF(!tearAway)

      auto tearAwayData = IRTPListenerTearAway::data(tearAway);
      ORTC_THROW_INVALID_STATE_IF(!tearAwayData)

      tearAwayData->mRTPTransport = transport;

      if (delegate) {
        tearAwayData->mDefaultSubscription = listener->subscribe(delegate);
      }

      return tearAway;
    }

    //-------------------------------------------------------------------------
    IRTPListenerSubscriptionPtr RTPListener::subscribe(IRTPListenerDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IRTPListenerSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IRTPListenerDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        RTPListenerPtr pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IRTPTransportPtr RTPListener::transport() const
    {
      return mRTPTransport.lock();
    }

    //-------------------------------------------------------------------------
    void RTPListener::setHeaderExtensions(const HeaderExtensionParametersList &headerExtensions)
    {
      ZS_THROW_NOT_IMPLEMENTED("todo")
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => IRTPListenerForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPListenerPtr RTPListener::create(IRTPTransportPtr transport)
    {
      RTPListenerPtr pThis(make_shared<RTPListener>(make_private {}, IORTCForInternal::queueORTC(), IRTPListenerDelegatePtr(), transport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::handleRTPPacket(
                                      IICETypes::Components viaComponent,
                                      IICETypes::Components packetType,
                                      const BYTE *buffer,
                                      size_t bufferLengthInBytes
                                      )
    {
      UseRTPReceiverPtr receiver;
      UseRTPSenderPtr sender;

      {
        AutoRecursiveLock lock(*this);

        if (IICETypes::Component_RTCP == packetType) {
          SecureByteBlockPtr rtcpBuffer = UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes);

          expireRTCPPackets();

          mBufferedRTCPPackets.push_back(TimeBufferPair(zsLib::now(), rtcpBuffer));
        }

        receiver = mReceiver.lock();
        if (IICETypes::Component_RTCP == packetType) {
          sender = mSender.lock();
        }
      }

#define TODO_THIS_IS_NOT_PROPER_HANDLING_OF_PACKET 1
#define TODO_THIS_IS_NOT_PROPER_HANDLING_OF_PACKET 2

      bool result = false;

      if (receiver) {
        auto success = receiver->handlePacket(viaComponent, packetType, buffer, bufferLengthInBytes);
        result = result || success;
      }

      if (sender) {
        auto success = sender->handlePacket(viaComponent, packetType, buffer, bufferLengthInBytes);
        result = result || success;
      }

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => IRTPListenerForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPListener::registerReceiver(
                                       UseReceiverPtr inReceiver,
                                       const Parameters &inParams,
                                       BufferList &outBufferList
                                       )
    {
      ZS_LOG_TRACE(log("registering RTP receiver") + ZS_PARAM("receiver", inReceiver->getID()) + inParams.toDebug())

      outBufferList.clear();

      AutoRecursiveLock lock(*this);

      mReceiverID = inReceiver->getID();
      mReceiver = inReceiver;

      expireRTCPPackets();

      for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter)
      {
        auto buffer = (*iter).second;
        outBufferList.push_back(buffer);
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterReceiver(UseReceiver &inReceiver)
    {
      AutoRecursiveLock lock(*this);

      if (inReceiver.getID() != mReceiverID) return;
      mReceiverID = 0;
      mReceiver.reset();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => IRTPListenerForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPListener::registerSender(
                                     UseSenderPtr inSender,
                                     const Parameters &inParams,
                                     BufferList &outBufferList
                                     )
    {
      ZS_LOG_TRACE(log("registering RTP sender (for RTCP feedback packets)") + ZS_PARAM("sender", inSender->getID()) + inParams.toDebug())

      outBufferList.clear();

      AutoRecursiveLock lock(*this);

      mSenderID = inSender->getID();
      mSender = inSender;

      expireRTCPPackets();

      for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter)
      {
        auto buffer = (*iter).second;
        outBufferList.push_back(buffer);
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterSender(UseSender &inSender)
    {
      AutoRecursiveLock lock(*this);

      if (inSender.getID() != mSenderID) return;
      mSenderID = 0;
      mSender.reset();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPListener::onWake()
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
    #pragma mark RTPListener => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPListener::onTimer(TimerPtr timer)
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
    #pragma mark RTPListener => IRTPListenerAsyncDelegate
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPListener::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::RTPListener");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPListener::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPListener");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPListener::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPListener::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPListener");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto rtpTransport = mRTPTransport.lock();
      UseServicesHelper::debugAppend(resultEl, "rtp transport", rtpTransport ? rtpTransport->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPListener::step()
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
    bool RTPListener::stepBogusDoSomething()
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
    void RTPListener::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 2

        // grace shutdown process done here

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
    void RTPListener::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPListenerPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPListenerStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::setError(WORD errorCode, const char *inReason)
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
    void RTPListener::expireRTCPPackets()
    {
      auto tick = zsLib::now();

      while (mBufferedRTCPPackets.size() > 0) {
        auto packetTime = mBufferedRTCPPackets.front().first;

        {
          if (mBufferedRTCPPackets.size() > mMaxBufferedRTCPPackets) goto expire_packet;
          if (packetTime + mMaxRTCPPacketAge < tick) goto expire_packet;
        }

      expire_packet:
        {
          ZS_LOG_TRACE(log("expiring buffered rtcp packet") + ZS_PARAM("tick", tick) + ZS_PARAM("packet time (s)", packetTime) + ZS_PARAM("total", mBufferedRTCPPackets.size()))
          mBufferedRTCPPackets.pop_front();
        }
      }
    }
    

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPListenerFactory &IRTPListenerFactory::singleton()
    {
      return RTPListenerFactory::singleton();
    }

    //-------------------------------------------------------------------------
    IRTPListenerPtr IRTPListenerFactory::create(
                                                IRTPListenerDelegatePtr delegate,
                                                IRTPTransportPtr transport,
                                                Optional<HeaderExtensionParametersList> headerExtensions
                                                )
    {
      if (this) {}
      return internal::RTPListener::create(delegate, transport, headerExtensions);
    }

    //-------------------------------------------------------------------------
    RTPListenerPtr IRTPListenerFactory::create(IRTPTransportPtr transport)
    {
      if (this) {}
      return internal::RTPListener::create(transport);
    }
    
  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPListener
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPListener::toDebug(IRTPListenerPtr transport)
  {
    return internal::RTPListener::toDebug(internal::RTPListener::convert(transport));
  }

  //---------------------------------------------------------------------------
  IRTPListenerPtr IRTPListener::create(
                                       IRTPListenerDelegatePtr delegate,
                                       IRTPTransportPtr transport,
                                       Optional<HeaderExtensionParametersList> headerExtensions
                                       )
  {
    return internal::IRTPListenerFactory::singleton().create(delegate, transport, headerExtensions);
  }

}
