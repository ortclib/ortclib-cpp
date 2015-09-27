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
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
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
    #pragma mark RTPListener::RegisteredHeaderExtension
    #pragma mark
    
    //---------------------------------------------------------------------------
    ElementPtr RTPListener::RegisteredHeaderExtension::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPListener::RegisteredHeaderExtension");

      UseServicesHelper::debugAppend(resultEl, "header extension uri", IRTPTypes::toString(mHeaderExtensionURI));
      UseServicesHelper::debugAppend(resultEl, "local id", mLocalID);
      UseServicesHelper::debugAppend(resultEl, "encrypted", mEncrypted);

      ElementPtr referencesEl = Element::create("references");
      for (auto iter = mReferences.begin(); iter != mReferences.end(); ++iter)
      {
        auto objectID = (*iter).first;

        UseServicesHelper::debugAppend(referencesEl, "reference", string(objectID));
      }

      if (referencesEl->hasChildren()) {
        UseServicesHelper::debugAppend(resultEl, referencesEl);
      }

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener::ReceiverInfo
    #pragma mark

    //---------------------------------------------------------------------------
    ElementPtr RTPListener::ReceiverInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPListener::ReceiverInfo");

      UseServicesHelper::debugAppend(resultEl, "object id", mReceiverID);
      UseServicesHelper::debugAppend(resultEl, "receiver", ((bool)mReceiver.lock()));

      UseServicesHelper::debugAppend(resultEl, mParameters.toDebug());

      return resultEl;
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
      mMaxRTCPPacketAge(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS)),
      mReceivers(make_shared<ReceiverObjectMap>()),
      mSenders(make_shared<SenderObjectMap>())
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

      if (headerExtensions.hasValue()) {
        // replace any existing registered header extensions
        listener->setHeaderExtensions(headerExtensions.value());
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
      AutoRecursiveLock lock(*this);

      // unregister previous header extensions
      unregisterAllReference(kAPIReference);

      // register all new header extensions
      for (auto iter = headerExtensions.begin(); iter != headerExtensions.end(); ++iter) {
        auto extension = (*iter);
        auto headerExtension = IRTPTypes::toHeaderExtensionURI(extension.mURI);
        if (HeaderExtensionURI_Unknown == headerExtension) {
          ZS_LOG_WARNING(Debug, log("header extension is not understood") + extension.toDebug())
          continue;
        }

        registerReference(kAPIReference, headerExtension, extension.mID, extension.mEncrypt);
      }
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
      bool result = false;

      ReceiverInfoPtr receiverInfo;

      ReceiverObjectMapPtr receivers;
      SenderObjectMapPtr senders;

      RTPPacketPtr rtpPacket;
      RTCPPacketPtr rtcpPacket;

      // parse packet outside of a lock
      if (IICETypes::Component_RTCP == packetType) {
        rtcpPacket = RTCPPacket::create(buffer, bufferLengthInBytes);
        if (!rtcpPacket) {
          ZS_LOG_WARNING(Trace, log("invalid rtcp packet received (thus dropping)"))
          return false;
        }
      } else {
        rtpPacket = RTPPacket::create(buffer, bufferLengthInBytes);

        if (!rtpPacket) {
          ZS_LOG_WARNING(Trace, log("invalid RTP packet received (thus dropping)"))
          return false;
        }
      }

      {
        AutoRecursiveLock lock(*this);

        if (IICETypes::Component_RTCP == packetType) {
          expireRTCPPackets();

          mBufferedRTCPPackets.push_back(TimePacketPair(zsLib::now(), rtcpPacket));

          receivers = mReceivers;
          senders = mSenders;
          goto process_rtcp;
        }

        if (findMapping(*rtpPacket, receiverInfo)) goto process_rtp;

#define TODO_FIRE_UNHANDLED_EVENT 1
#define TODO_FIRE_UNHANDLED_EVENT 2

        return false;
      }

    process_rtp:
      {
        auto receiver = receiverInfo->mReceiver.lock();

        if (!receiver) {
          ZS_LOG_WARNING(Trace, log("receiver is gone") + receiverInfo->toDebug())
          return false;
        }

        ZS_LOG_TRACE(log("forwarding RTP packet to receiver") + ZS_PARAM("receiver id", receiver->getID()))
        return receiver->handlePacket(viaComponent, rtpPacket);
      }

    process_rtcp:
      {
        for (auto iter = receivers->begin(); iter != receivers->end(); ++iter) {
          ReceiverID receiverID = (*iter).first;
          auto receiverInfo = (*iter).second;

          auto receiver = (*iter).second->mReceiver.lock();

          if (!receiver) {
            ZS_LOG_WARNING(Trace, log("receiver is gone") + ZS_PARAM("receiver ID", receiverID) + receiverInfo->toDebug())
            continue;
          }

          ZS_LOG_TRACE(log("forwarding RTCP packet to receiver") + ZS_PARAM("receiver id", receiverID))
          auto success = receiver->handlePacket(viaComponent, rtcpPacket);
          result = result || success;
        }

        for (auto iter = senders->begin(); iter != senders->end(); ++iter) {
          SenderID senderID = (*iter).first;
          auto sender = (*iter).second.lock();

          if (!sender) {
            ZS_LOG_WARNING(Trace, log("sender is gone") + ZS_PARAM("sender ID", senderID))
            continue;
          }

          ZS_LOG_TRACE(log("forwarding RTCP packet to sender") + ZS_PARAM("sender id", senderID))
          auto success = sender->handlePacket(viaComponent, rtcpPacket);
          result = result || success;
        }
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
                                       RTCPPacketList &outPacketList
                                       )
    {
      ZS_LOG_TRACE(log("registering RTP receiver") + ZS_PARAM("receiver", inReceiver->getID()) + inParams.toDebug())

      outPacketList.clear();

      AutoRecursiveLock lock(*this);

      mReceiverID = inReceiver->getID();
      mReceiver = inReceiver;

      expireRTCPPackets();

      for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter)
      {
        auto buffer = (*iter).second;
        outPacketList.push_back(buffer);
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
                                     RTCPPacketList &outPacketList
                                     )
    {
      ZS_LOG_TRACE(log("registering RTP sender (for RTCP feedback packets)") + ZS_PARAM("sender", inSender->getID()) + inParams.toDebug())

      outPacketList.clear();

      AutoRecursiveLock lock(*this);

      mSenderID = inSender->getID();
      mSender = inSender;

      expireRTCPPackets();

      for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter)
      {
        auto buffer = (*iter).second;
        outPacketList.push_back(buffer);
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
    static bool shouldFilter(IRTPTypes::HeaderExtensionURIs extensionURI)
    {
      switch (extensionURI) {
        case IRTPTypes::HeaderExtensionURI_Unknown:                           return true;
        case IRTPTypes::HeaderExtensionURI_MuxID:                             return false;
      //case IRTPTypes::HeaderExtensionURI_MID:                               return false;
        case IRTPTypes::HeaderExtensionURI_ClienttoMixerAudioLevelIndication: return true;
        case IRTPTypes::HeaderExtensionURI_MixertoClientAudioLevelIndication: return true;
        case IRTPTypes::HeaderExtensionURI_FrameMarking:                      return true;
        case IRTPTypes::HeaderExtensionURI_ExtendedSourceInformation:         return false;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPListener::registerReference(
                                        PUID objectID,
                                        HeaderExtensionURIs extensionURI,
                                        LocalID localID,
                                        bool encrytped
                                        )
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(HeaderExtensionURI_Unknown == extensionURI)

      if (shouldFilter(extensionURI)) {
        ZS_LOG_DEBUG(log("extension header is not relevant  to listener (thus filtering)") + ZS_PARAM("object id", objectID) + ZS_PARAM("extension uri", IRTPTypes::toString(extensionURI)) + ZS_PARAM("local ID", localID) + ZS_PARAM("encrypted", encrytped))
        return;
      }

      auto found = mRegisteredExtensions.find(localID);
      if (found == mRegisteredExtensions.end()) {
        RegisteredHeaderExtension extension;
        extension.mHeaderExtensionURI = extensionURI;
        extension.mLocalID = localID;
        extension.mEncrypted = encrytped;
        extension.mReferences[objectID] = true;
        mRegisteredExtensions[localID] = extension;

        ZS_LOG_DEBUG(log("registered header extension") + ZS_PARAM("object id", objectID) + extension.toDebug())
        return;
      }

      RegisteredHeaderExtension &extension = (*found).second;

      // cannot change meaning of header extension
      ORTC_THROW_INVALID_PARAMETERS_IF((extension.mEncrypted) &&
                                       (!encrytped))
      ORTC_THROW_INVALID_PARAMETERS_IF((!extension.mEncrypted) &&
                                       (encrytped))
      ORTC_THROW_INVALID_PARAMETERS_IF(extensionURI != extension.mHeaderExtensionURI)

      extension.mReferences[objectID] = objectID;

      ZS_LOG_DEBUG(log("referencing existing header extension") + ZS_PARAM("object id", objectID) + extension.toDebug())
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterReference(
                                          PUID objectID,
                                          LocalID localID
                                          )
    {
      auto found = mRegisteredExtensions.find(localID);
      if (found != mRegisteredExtensions.end()) {
        ZS_LOG_DEBUG(log("local id was not registered") + ZS_PARAM("object id", objectID) + ZS_PARAM("local id", localID))
        return;
      }

      RegisteredHeaderExtension &extension = (*found).second;

      auto foundObject = extension.mReferences.find(objectID);
      if (foundObject == extension.mReferences.end()) {
        ZS_LOG_DEBUG(log("local object was not registered") + ZS_PARAM("object id", objectID) + ZS_PARAM("local id", localID))
        return;
      }

      extension.mReferences.erase(foundObject);

      ZS_LOG_DEBUG(log("removing reference to existing header extension") + ZS_PARAM("object id", objectID) + extension.toDebug())
      if (extension.mReferences.size() > 0) return;

      mRegisteredExtensions.erase(found);
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterAllReference(PUID objectID)
    {
      for (auto iter_doNotUse = mRegisteredExtensions.begin(); iter_doNotUse != mRegisteredExtensions.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        RegisteredHeaderExtension &extension = (*current).second;

        auto found = extension.mReferences.find(objectID);
        if (found == extension.mReferences.end()) continue;

        extension.mReferences.erase(found);

        ZS_LOG_DEBUG(log("removing reference to existing header extension") + ZS_PARAM("object id", objectID) + extension.toDebug())

        if (extension.mReferences.size() > 0) continue;

        mRegisteredExtensions.erase(current);
      }
    }

    //-------------------------------------------------------------------------
    bool RTPListener::findMapping(
                                  const RTPPacket &rtpPacket,
                                  ReceiverInfoPtr &outReceiverInfo
                                  )
    {
      String muxID = extractMuxID(rtpPacket);

      {
        if (findMappingUsingSSRC(rtpPacket, outReceiverInfo)) goto fill_mux_id;

        if (findMappingUsingMuxID(muxID, rtpPacket, outReceiverInfo)) return true;

        if (findMappingUsingHeaderExtensions(rtpPacket, outReceiverInfo)) goto fill_mux_id;

        if (findMappingUsingPayloadType(rtpPacket, outReceiverInfo)) return true;
      }

    fill_mux_id:
      {
        ASSERT((bool)outReceiverInfo)
        if (!fillMuxIDParameters(muxID, outReceiverInfo)) {
          outReceiverInfo = ReceiverInfoPtr();
          return false;
        }
      }

      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingSSRC(
                                           const RTPPacket &rtpPacket,
                                           ReceiverInfoPtr &outReceiverInfo
                                           )
    {
      auto found = mSSRCTable.find(rtpPacket.mSSRC);
      if (found == mSSRCTable.end()) return false;

      outReceiverInfo = (*found).second;
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingMuxID(
                                            const String &muxID,
                                            const RTPPacket &rtpPacket,
                                            ReceiverInfoPtr &outReceiverInfo
                                            )
    {
      if (!muxID.hasData()) return false;

      auto found = mMuxIDTable.find(muxID);
      if (found == mMuxIDTable.end()) return false;

      outReceiverInfo = (*found).second;

      ZS_LOG_DEBUG(log("creating new SSRC table entry (based on mux id mapping to existing receiver)") + ZS_PARAM("mux id", muxID) + outReceiverInfo->toDebug())

      mSSRCTable[rtpPacket.ssrc()] = outReceiverInfo;

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingHeaderExtensions(
                                                       const RTPPacket &rtpPacket,
                                                       ReceiverInfoPtr &outReceiverInfo
                                                       )
    {
      for (auto ext = rtpPacket.firstHeaderExtension(); NULL != ext; ext = ext->mNext) {

        LocalID localID = static_cast<LocalID>(ext->mID);
        auto found = mRegisteredExtensions.find(localID);
        if (found == mRegisteredExtensions.end()) continue; // header extension is not understood

        RegisteredHeaderExtension &headerInfo = (*found).second;

        if (IRTPTypes::HeaderExtensionURI_ExtendedSourceInformation != headerInfo.mHeaderExtensionURI) continue;

        RTPPacket::ExtendedSourceInformationHeadExtension extendedInfo(*ext);
        if (!extendedInfo.isAssociatedSSRCValid()) {
          ZS_LOG_TRACE(log("extended source info associated SSRC is not set") + extendedInfo.toDebug())
          continue;
        }

        auto foundSSRC = mSSRCTable.find(extendedInfo.associatedSSRC());

        if (foundSSRC == mSSRCTable.end()) {
          ZS_LOG_WARNING(Trace, log("associated SSRC was not found in SSRC table") + extendedInfo.toDebug())
          continue;
        }

        outReceiverInfo = (*foundSSRC).second;

        ZS_LOG_DEBUG(log("creating a new SSRC entry in SSRC table (based on associated SSRC being found)") + extendedInfo.toDebug() + outReceiverInfo->toDebug())

        // the associated SSRC was found in table thus must route to same receiver
        mSSRCTable[rtpPacket.ssrc()] = outReceiverInfo;
        return true;
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingPayloadType(
                                                  const RTPPacket &rtpPacket,
                                                  ReceiverInfoPtr &outReceiverInfo
                                                  )
    {
#define TODO 1
#define TODO 2
      return false;
    }

    //-------------------------------------------------------------------------
    String RTPListener::extractMuxID(const RTPPacket &rtpPacket)
    {
      for (auto ext = rtpPacket.firstHeaderExtension(); NULL != ext; ext = ext->mNext) {
        LocalID localID = static_cast<LocalID>(ext->mID);
        auto found = mRegisteredExtensions.find(localID);
        if (found == mRegisteredExtensions.end()) continue; // header extension is not understood

        RegisteredHeaderExtension &headerInfo = (*found).second;

        if (IRTPTypes::HeaderExtensionURI_MuxID != headerInfo.mHeaderExtensionURI) continue;

        RTPPacket::MidHeadExtension mid(*ext);

        String muxID(mid.mid());
        if (!muxID.hasData()) continue;

        return muxID;
      }

      return String();
    }

    //-------------------------------------------------------------------------
    bool RTPListener::fillMuxIDParameters(
                                          const String &muxID,
                                          ReceiverInfoPtr &ioReceiverInfo
                                          )
    {
      ASSERT((bool)ioReceiverInfo)

      if (!muxID.hasData()) return true;

      if (ioReceiverInfo->mParameters.mMuxID.hasData()) {
        if (muxID != ioReceiverInfo->mParameters.mMuxID) {
          // already has a MuxID and this isn't it!
          ZS_LOG_WARNING(Debug, log("receiver mux id and packet mux id are mis-matched") + ZS_PARAM("mux id", muxID) + ioReceiverInfo->toDebug())
          return false;
        }
      }

      ReceiverInfoPtr info(make_shared<ReceiverInfo>(*ioReceiverInfo));
      info->mParameters.mMuxID = muxID;
      setReceiverInfo(info);

      ioReceiverInfo = info;
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPListener::setReceiverInfo(ReceiverInfoPtr receiverInfo)
    {
      ReceiverObjectMapPtr receivers(make_shared<ReceiverObjectMap>(*mReceivers));

      // replace or add to replacement list
      (*receivers)[receiverInfo->mReceiverID] = receiverInfo;

      for (auto iter = mSSRCTable.begin(); iter != mSSRCTable.end(); ++iter) {
        ReceiverInfoPtr &existingInfo = (*iter).second;
        if (existingInfo->mReceiverID != receiverInfo->mReceiverID) continue;

        // replace existing entry in receiver table
        existingInfo = receiverInfo;
      }

      if (receiverInfo->mParameters.mMuxID.hasData()) {
        // replace existing receiver info in mux table
        mMuxIDTable[receiverInfo->mParameters.mMuxID] = receiverInfo;
      }

      // point to replacement list
      mReceivers = receivers;
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
