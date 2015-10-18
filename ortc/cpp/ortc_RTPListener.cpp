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
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTP_PACKETS_IN_BUFFER, 100);
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTP_PACKETS_IN_SECONDS, 30);

      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTCP_PACKETS_IN_BUFFER, 100);
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS, 30);

      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_SSRC_TO_MUX_ID_TIMEOUT_IN_SECONDS, 60);
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_UNHANDLED_EVENTS_TIMEOUT_IN_SECONDS, 60);
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

      UseServicesHelper::debugAppend(resultEl, "order id", mOrderID);
      UseServicesHelper::debugAppend(resultEl, "receiver id", mReceiverID);
      UseServicesHelper::debugAppend(resultEl, "receiver", ((bool)mReceiver.lock()));

      UseServicesHelper::debugAppend(resultEl, mParameters.toDebug());
      UseServicesHelper::debugAppend(resultEl, mOriginalParameters.toDebug());

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
    bool RTPListener::UnhandledEventInfo::operator<(const UnhandledEventInfo &op2) const
    {
      if (mSSRC < op2.mSSRC) return true;
      if (mSSRC > op2.mSSRC) return false;
      if (mCodecPayloadType < op2.mCodecPayloadType) return true;
      if (mCodecPayloadType > op2.mCodecPayloadType) return false;
      if (mMuxID < op2.mMuxID) return true;
      //if (mMuxID > op2.mMuxID) return false; // test not needed (will be false either way)
      return false;
    }
    
    //---------------------------------------------------------------------------
    ElementPtr RTPListener::UnhandledEventInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPListener::UnhandledEventInfo");

      UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
      UseServicesHelper::debugAppend(resultEl, "codec payload type", mCodecPayloadType);
      UseServicesHelper::debugAppend(resultEl, "mux id", mMuxID);

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
      mMaxBufferedRTPPackets(SafeInt<decltype(mMaxBufferedRTCPPackets)>(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTP_PACKETS_IN_BUFFER))),
      mMaxRTPPacketAge(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTP_PACKETS_IN_SECONDS)),
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

      mSSRCToMuxTableExpires = Seconds(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_SSRC_TO_MUX_ID_TIMEOUT_IN_SECONDS));
      if (mSSRCToMuxTableExpires < Seconds(1)) {
        mSSRCToMuxTableExpires = Seconds(1);
      }

      mSSRCToMuxTableTimer = Timer::create(mThisWeak.lock(), (zsLib::toMilliseconds(mSSRCToMuxTableExpires) / 2));

      mUnhanldedEventsExpires = Seconds(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_UNHANDLED_EVENTS_TIMEOUT_IN_SECONDS));
      if (mUnhanldedEventsExpires < Seconds(1)) {
        mUnhanldedEventsExpires = Seconds(1);
      }
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

        for (auto iter = mUnhandledEvents.begin(); iter != mUnhandledEvents.end(); ++iter) {
          auto &unhandledData = (*iter).first;

          delegate->onRTPListenerUnhandledRTP(pThis, unhandledData.mSSRC, unhandledData.mCodecPayloadType, unhandledData.mMuxID.c_str());
        }
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
      unregisterAllHeaderExtensionReferences(kAPIReference);

      // register all new header extensions
      for (auto iter = headerExtensions.begin(); iter != headerExtensions.end(); ++iter) {
        auto extension = (*iter);
        auto headerExtension = IRTPTypes::toHeaderExtensionURI(extension.mURI);
        if (HeaderExtensionURI_Unknown == headerExtension) {
          ZS_LOG_WARNING(Debug, log("header extension is not understood") + extension.toDebug())
          continue;
        }

        registerHeaderExtensionReference(kAPIReference, headerExtension, extension.mID, extension.mEncrypt);
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

        if (isShutdown()) {
          ZS_LOG_WARNING(Trace, log("ingoring incomign packet (already shutdown)"))
          return false;
        }

        if (IICETypes::Component_RTCP == packetType) {
          expireRTCPPackets();

          processByes(*rtcpPacket);
          processSDESMid(*rtcpPacket);
          processSenderReports(*rtcpPacket);

          mBufferedRTCPPackets.push_back(TimeRTCPPacketPair(zsLib::now(), rtcpPacket));

          receivers = mReceivers;
          senders = mSenders;
          goto process_rtcp;
        }

        String muxID;
        if (findMapping(*rtpPacket, receiverInfo, muxID)) goto process_rtp;

        if (isShuttingDown()) {
          ZS_LOG_WARNING(Debug, log("ignoring unhandled packet (during shutdown process)"))
          return false;
        }

        expireRTPPackets();

        Time tick = zsLib::now();

        ASSERT(IICETypes::Component_RTP == viaComponent)

        // provide some modest buffering
        mBufferedRTPPackets.push_back(TimeRTPPacketPair(tick, rtpPacket));

        UnhandledEventInfo unhandled;
        unhandled.mSSRC = rtpPacket->ssrc();
        unhandled.mCodecPayloadType = rtpPacket->pt();
        unhandled.mMuxID = muxID;

        auto found = mUnhandledEvents.find(unhandled);
        if (found == mUnhandledEvents.end()) {
          if (mUnhandledEvents.size() < 1) {
            mUnhanldedEventsTimer = Timer::create(mThisWeak.lock(), mUnhanldedEventsExpires);
          }

          mUnhandledEvents[unhandled] = tick;

          mSubscriptions.delegate()->onRTPListenerUnhandledRTP(mThisWeak.lock(), unhandled.mSSRC, unhandled.mCodecPayloadType, unhandled.mMuxID.c_str());
        }

        return true;
      }

    process_rtp:
      {
        auto receiver = receiverInfo->mReceiver.lock();

        if (!receiver) {
          ZS_LOG_WARNING(Trace, log("receiver is gone") + receiverInfo->toDebug())
          return false;
        }

        ZS_LOG_TRACE(log("forwarding RTP packet to receiver") + ZS_PARAM("receiver id", receiver->getID()) + ZS_PARAM("ssrc", rtpPacket->ssrc()))
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

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Detail, log("cannot register receiver while shutdown / shutting down"))
        return;
      }

      ReceiverID receiverID = inReceiver->getID();

      ReceiverInfoPtr replacementInfo(make_shared<ReceiverInfo>());
      replacementInfo->mOrderID = AutoPUID();
      replacementInfo->mReceiverID = receiverID;
      replacementInfo->mReceiver = inReceiver;
      replacementInfo->mParameters = inParams;
      replacementInfo->mOriginalParameters = inParams;

      auto found = mReceivers->find(receiverID);
      if (found == mReceivers->end()) {
        setReceiverInfo(replacementInfo);
        goto finalize_registration;
      }

      // scope: check to see if anything important changed in the encoding parameters
      {
        auto &existingInfo = (*found).second;

        size_t indexExisting = 0;
        for (auto iterExistingEncoding = existingInfo->mParameters.mEncodingParameters.begin(); iterExistingEncoding != existingInfo->mParameters.mEncodingParameters.begin(); ++iterExistingEncoding, ++indexExisting) {
          auto &existinEncodingInfo = (*iterExistingEncoding);

          if (existinEncodingInfo.mEncodingID.hasData()) {
            // scope: search replacement for same encoding ID
            {
              for (auto iterReplacementEncoding = replacementInfo->mParameters.mEncodingParameters.begin(); iterReplacementEncoding != replacementInfo->mParameters.mEncodingParameters.end(); ++iterReplacementEncoding) {
                auto &replacementEncodingInfo = (*iterReplacementEncoding);
                if (replacementEncodingInfo.mEncodingID == existinEncodingInfo.mEncodingID) {
                  // these encoding are identical
                  handleDeltaChanges(existinEncodingInfo, replacementEncodingInfo);
                  goto done_search_by_encoding_id;
                }
              }

              unregisterEncoding(existinEncodingInfo);
            }

          done_search_by_encoding_id: {}
            continue;
          }

          // scope: lookup by index
          {
            size_t indexReplacement = 0;
            for (auto iterReplacementEncoding = replacementInfo->mParameters.mEncodingParameters.begin(); iterReplacementEncoding != replacementInfo->mParameters.mEncodingParameters.end(); ++iterReplacementEncoding, ++indexReplacement) {
              if (indexExisting != indexReplacement) continue;

              auto &replacementEncodingInfo = (*iterReplacementEncoding);
              if (replacementEncodingInfo.mEncodingID.hasData()) {
                unregisterEncoding(existinEncodingInfo);
                goto done_search_by_index;
              }

              handleDeltaChanges(existinEncodingInfo, replacementEncodingInfo);
              goto done_search_by_index;
            }

            unregisterEncoding(existinEncodingInfo);
            goto done_search_by_index;
          }

        done_search_by_index: {}
          continue;
        }

        setReceiverInfo(replacementInfo);
        goto finalize_registration;
      }

    finalize_registration:
      {
        unregisterAllHeaderExtensionReferences(receiverID);

        // register all new header extensions
        for (auto iter = inParams.mHeaderExtensions.begin(); iter != inParams.mHeaderExtensions.end(); ++iter) {
          auto extension = (*iter);

          auto headerExtension = IRTPTypes::toHeaderExtensionURI(extension.mURI);
          if (HeaderExtensionURI_Unknown == headerExtension) {
            ZS_LOG_WARNING(Debug, log("header extension is not understood") + extension.toDebug())
            continue;
          }

          registerHeaderExtensionReference(receiverID, headerExtension, extension.mID, extension.mEncrypt);
        }

        expireRTCPPackets();

        for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter)
        {
          auto buffer = (*iter).second;
          outPacketList.push_back(buffer);
        }

        reattemptDelivery();
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::handleDeltaChanges(
                                         const EncodingParameters &existing,
                                         EncodingParameters &ioReplacement
                                         )
    {
      // scope: check for codec changes
      {
        bool codecChanged = false;

        if (existing.mCodecPayloadType.hasValue()) {
          if (ioReplacement.mCodecPayloadType.hasValue()) {
            if (existing.mCodecPayloadType.value() != ioReplacement.mCodecPayloadType.value()) codecChanged = true;
          } else {
            codecChanged = true;
          }
        } else if (ioReplacement.mCodecPayloadType.hasValue()){
          codecChanged = true;
        }

        if (codecChanged) {
          unregisterEncoding(existing);
        }
      }

      if (existing.mSSRC.hasValue()) {
        if (ioReplacement.mSSRC.hasValue()) {
          if (existing.mSSRC.value() != ioReplacement.mSSRC.value()) {
            unregisterSSRCUsage(existing.mSSRC.value());
          }
        } else {
          ioReplacement.mSSRC = existing.mSSRC;
        }
      }

      if (existing.mRTX.hasValue()) {
        if (ioReplacement.mRTX.hasValue()) {
          if (existing.mRTX.value().mSSRC != ioReplacement.mRTX.value().mSSRC) {
            unregisterSSRCUsage(existing.mRTX.value().mSSRC);
          }
        } else {
          ioReplacement.mRTX = existing.mRTX;
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterEncoding(const EncodingParameters &existing)
    {
      if (existing.mSSRC.hasValue()) {
        unregisterSSRCUsage(existing.mSSRC.value());
      }
      if (existing.mRTX.hasValue()) {
        unregisterSSRCUsage(existing.mRTX.value().mSSRC);
      }
      if (existing.mRTX.hasValue()) {
        unregisterSSRCUsage(existing.mRTX.value().mSSRC);
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterSSRCUsage(SSRCType ssrc)
    {
      auto found = mSSRCTable.find(ssrc);
      if (found == mSSRCTable.end()) return;

      ZS_LOG_TRACE(log("removing entry from SSRC table") + ZS_PARAM("ssrc", ssrc) + (*found).second->toDebug())

      mSSRCTable.erase(found);
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterReceiver(UseReceiver &inReceiver)
    {
      AutoRecursiveLock lock(*this);

      ReceiverID receiverID = inReceiver.getID();

      {
        // scope: search existing receiver list
        {
          auto found = mReceivers->find(receiverID);
          if (found == mReceivers->end()) {
            ZS_LOG_WARNING(Debug, log("receiver was not registered (thus ignoring request to unregister)") + ZS_PARAM("receiver id", receiverID))
            return;
          }
        }

        // scope: create a replacement receiver list (without this receiver)
        {
          ReceiverObjectMapPtr receivers(make_shared<ReceiverObjectMap>(*mReceivers));

          auto found = receivers->find(receiverID);
          ASSERT(found != receivers->end())

          auto receiverInfo = (*found).second;

          ZS_LOG_DEBUG(log("unregistering receiver") + ZS_PARAM("receiver id", receiverID) + receiverInfo->toDebug())

          receivers->erase(found);

          mReceivers = receivers;
        }
      }

      // purge receiver from ssrc table
      for (auto iter_doNotUse = mSSRCTable.begin(); iter_doNotUse != mSSRCTable.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        SSRCType ssrc = (*current).first;
        ReceiverInfoPtr &receiverInfo = (*current).second;
        if (receiverInfo->mReceiverID != receiverID) continue;

        ZS_LOG_TRACE(log("removing SSRC mapping to receiver") + ZS_PARAM("ssrc", ssrc) + receiverInfo->toDebug())

        mSSRCTable.erase(current);
      }

      // purge from mux id table
      for (auto iter_doNotUse = mMuxIDTable.begin(); iter_doNotUse != mMuxIDTable.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        const String &muxID = (*current).first;
        ReceiverInfoPtr &receiverInfo = (*current).second;

        if (receiverInfo->mReceiverID != receiverID) continue;

        ZS_LOG_TRACE(log("removing mux id mapping to receiver") + ZS_PARAM("mux id", muxID) + receiverInfo->toDebug())

        mMuxIDTable.erase(current);
      }

      unregisterAllHeaderExtensionReferences(receiverID);
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

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Detail, log("cannot register sender while shutdown / shutting down"))
        return;
      }

      SenderObjectMapPtr senders(make_shared<SenderObjectMap>(*mSenders));
      (*senders)[inSender->getID()] = inSender;
      mSenders = senders;

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

      SenderID senderID = inSender.getID();

      {
        // scope: search existing sender list
        {
          auto found = mSenders->find(senderID);
          if (found == mSenders->end()) {
            ZS_LOG_WARNING(Debug, log("sender was not registered (thus ignoring request to unregister)") + ZS_PARAM("sender id", senderID))
            return;
          }
        }

        // scope: create a replacement sender list (without this sender)
        {
          SenderObjectMapPtr senders(make_shared<SenderObjectMap>(*mSenders));

          auto found = senders->find(senderID);
          ASSERT(found != senders->end())

          ZS_LOG_DEBUG(log("unregistering sender") + ZS_PARAM("sender id", senderID))

          senders->erase(found);

          mSenders = senders;
        }
      }
      
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

      if (timer == mSSRCToMuxTableTimer) {

        expireRTPPackets();   // might as well expire these too now
        expireRTCPPackets();  // might as well expire these too now

        auto adjustedTick = zsLib::now() - mSSRCToMuxTableExpires;

        // now =  N; then = T; expire = E; adjusted = A;    N-E = A; if A > T then expired
        // now = 10; then = 5; expiry = 3;                 10-3 = 7;    7 > 5 = expired (true)
        // now =  6; then = 5; expiry = 3;                  6-3 = 3;    3 > 5 = not expired (false)

        for (auto iter_doNotUse = mSSRCToMuxTable.begin(); iter_doNotUse != mSSRCToMuxTable.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          const Time &lastReceived = (*current).second.first;

          if (!(adjustedTick > lastReceived)) continue;

          SSRCType ssrc = (*current).first;

          ZS_LOG_TRACE(log("expiring SSRC to mux ID mapping") + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("last received", lastReceived) + ZS_PARAM("adjusted tick", adjustedTick))
          mSSRCToMuxTable.erase(current);
        }

        return;
      }

      if (timer == mUnhanldedEventsTimer) {

        auto tick = zsLib::now();

        for (auto iter_doNotUse = mUnhandledEvents.begin(); iter_doNotUse != mUnhandledEvents.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &eventFiredTime = (*current).second;

          if (eventFiredTime + mUnhanldedEventsExpires > tick) continue;

          mUnhandledEvents.erase(current);
        }

        if (mUnhandledEvents.size() < 1) {
          mUnhanldedEventsTimer->cancel();
          mUnhanldedEventsTimer.reset();
        }
        return;
      }

      ZS_LOG_WARNING(Debug, log("notified about obsolete timer (thus ignoring)") + ZS_PARAM("timer id", timer->getID()))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener => IRTPListenerAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPListener::onDeliverPacket(
                                      IICETypes::Components viaComponent,
                                      UseRTPReceiverPtr receiver,
                                      RTPPacketPtr packet
                                      )
    {
      ZS_LOG_TRACE(log("forwarding previously buffered RTP packet to receiver") + ZS_PARAM("receiver id", receiver->getID()) + ZS_PARAM("via", IICETypes::toString(viaComponent)) + ZS_PARAM("ssrc", packet->ssrc()))

      receiver->handlePacket(viaComponent, packet);
    }


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
      if (!stepAttemptDelivery()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("dtls is not ready"))
        return;
      }

    ready:
      {
        setState(State_Ready);
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool RTPListener::stepAttemptDelivery()
    {
      if (!mReattemptRTPDelivery) {
        ZS_LOG_TRACE(log("no need to reattempt deliver at this time"))
        return true;
      }

      ZS_LOG_DEBUG(log("will attempt to deliver any buffered RTP packets"))

      mReattemptRTPDelivery = false;

      expireRTPPackets();

      for (auto iter_doNotUse = mBufferedRTPPackets.begin(); iter_doNotUse != mBufferedRTPPackets.end();) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        RTPPacketPtr packet = (*current).second;

        ReceiverInfoPtr receiverInfo;
        String muxID;
        if (!findMapping(*packet, receiverInfo, muxID)) continue;

        auto receiver = receiverInfo->mReceiver.lock();

        if (receiver) {
          ZS_LOG_TRACE(log("will attempt to deliver buffered RTP packet") + ZS_PARAM("receiver", receiver->getID()) + ZS_PARAM("ssrc", packet->ssrc()))
          IRTPListenerAsyncDelegateProxy::create(mThisWeak.lock())->onDeliverPacket(IICETypes::Component_RTP, receiver, packet);
        }

        mBufferedRTPPackets.erase(current);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPListener::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {}

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      mBufferedRTPPackets.clear();
      mBufferedRTCPPackets.clear();

      mRegisteredExtensions.clear();

      mReceivers = make_shared<ReceiverObjectMap>();
      mSenders = make_shared<SenderObjectMap>();

      mSSRCTable.clear();
      mMuxIDTable.clear();
      mSSRCToMuxTable.clear();
      mUnhandledEvents.clear();

      if (mSSRCToMuxTableTimer) {
        mSSRCToMuxTableTimer->cancel();
        mSSRCToMuxTableTimer.reset();
      }

      if (mUnhanldedEventsTimer) {
        mUnhanldedEventsTimer->cancel();
        mUnhanldedEventsTimer.reset();
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
    void RTPListener::expireRTPPackets()
    {
      auto tick = zsLib::now();

      while (mBufferedRTPPackets.size() > 0) {
        auto packetTime = mBufferedRTPPackets.front().first;

        {
          if (mBufferedRTPPackets.size() > mMaxBufferedRTPPackets) goto expire_packet;
          if (packetTime + mMaxRTPPacketAge < tick) goto expire_packet;
          break;
        }

      expire_packet:
        {
          ZS_LOG_TRACE(log("expiring buffered rtp packet") + ZS_PARAM("tick", tick) + ZS_PARAM("packet time (s)", packetTime) + ZS_PARAM("total", mBufferedRTPPackets.size()))
          mBufferedRTPPackets.pop_front();
        }
      }
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
          break;
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
        case IRTPTypes::HeaderExtensionURI_RID:                               return true;
        case IRTPTypes::HeaderExtensionURI_3gpp_VideoOrientation:             return true;
        case IRTPTypes::HeaderExtensionURI_3gpp_VideoOrientation6:            return true;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPListener::registerHeaderExtensionReference(
                                                       PUID objectID,
                                                       HeaderExtensionURIs extensionURI,
                                                       LocalID localID,
                                                       bool encrytped
                                                       )
    {
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
    void RTPListener::unregisterAllHeaderExtensionReferences(PUID objectID)
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
                                  ReceiverInfoPtr &outReceiverInfo,
                                  String &outMuxID
                                  )
    {
      outMuxID = extractMuxID(rtpPacket);

      {
        if (findMappingUsingSSRCTable(rtpPacket, outReceiverInfo)) goto fill_mux_id;

        if (findMappingUsingMuxID(outMuxID, rtpPacket, outReceiverInfo)) return true;

        if (findMappingUsingSSRCInEncodingParams(outMuxID, rtpPacket, outReceiverInfo)) goto fill_mux_id;

        if (findMappingUsingPayloadType(outMuxID, rtpPacket, outReceiverInfo)) goto fill_mux_id;

        return false;
      }

    fill_mux_id:
      {
        ASSERT((bool)outReceiverInfo)
        if (!fillMuxIDParameters(outMuxID, outReceiverInfo)) {
          outReceiverInfo = ReceiverInfoPtr();
          return false;
        }
      }

      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingSSRCTable(
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
      reattemptDelivery();

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingSSRCInEncodingParams(
                                                           const String &muxID,
                                                           const RTPPacket &rtpPacket,
                                                           ReceiverInfoPtr &outReceiverInfo
                                                           )
    {
      ReceiverObjectMapPtr receivers = mReceivers;

      for (auto iter = receivers->begin(); iter != receivers->end(); ++iter)
      {
        ReceiverInfoPtr &info = (*iter).second;

        if ((info->mParameters.mMuxID.hasData()) &&
            (muxID.hasData())) {
          if (muxID != info->mParameters.mMuxID) {
            // cannot consider any receiver which has a mux id but does not
            // match this receiver's mux id
            continue;
          }
        }

        // first check to see if this SSRC is inside this receiver's
        // encoding parameters but if this value was auto-filled in
        // those encoding paramters and not set by the application
        // developer.
        {
          auto iterParm = info->mParameters.mEncodingParameters.begin();

          for (; iterParm != info->mParameters.mEncodingParameters.end(); ++iterParm)
          {
            EncodingParameters &encParams = (*iterParm);

            if (encParams.mSSRC.hasValue()) {
              if (rtpPacket.ssrc() == encParams.mSSRC.value()) goto map_ssrc;
            }

            if (encParams.mRTX.hasValue())  {
              if (rtpPacket.ssrc() == encParams.mRTX.value().mSSRC) goto map_ssrc;
            }

            if (encParams.mFEC.hasValue()) {
              if (rtpPacket.ssrc() == encParams.mFEC.value().mSSRC) goto map_ssrc;
            }
          }

          // no SSRC match was found
          continue;
        }

      map_ssrc:
        {
          outReceiverInfo = info;

          ZS_LOG_DEBUG(log("creating a new SSRC entry in SSRC table (based on associated SSRC being found)") + outReceiverInfo->toDebug())

          // the associated SSRC was found in table thus must route to same receiver
          mSSRCTable[rtpPacket.ssrc()] = outReceiverInfo;
          reattemptDelivery();
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPListener::findMappingUsingPayloadType(
                                                  const String &muxID,
                                                  const RTPPacket &rtpPacket,
                                                  ReceiverInfoPtr &outReceiverInfo
                                                  )
    {
      auto payloadType = rtpPacket.pt();

      CodecKinds foundKind = CodecKind_Unknown;

      for (auto iter = mReceivers->begin(); iter != mReceivers->end(); ++iter) {
        auto &receiverInfo = (*iter).second;

        if ((receiverInfo->mParameters.mMuxID.hasData()) &&
            (muxID.hasData())) {
          if (muxID != receiverInfo->mParameters.mMuxID) {
            // cannot consider any receiver which has a mux id but does not
            // match this receiver's mux id
            continue;
          }
        }

        CodecKinds kind = CodecKind_Unknown;

        for (auto iterCodec = receiverInfo->mParameters.mCodecs.begin(); iterCodec != receiverInfo->mParameters.mCodecs.end(); ++iterCodec) {
          auto &codecInfo = (*iterCodec);
          if (payloadType != codecInfo.mPayloadType) continue;

          auto supportedType = toSupportedCodec(codecInfo.mName);
          if (SupportedCodec_Unknown == supportedType) continue;

          kind = getCodecKind(supportedType);
          if (CodecKind_Unknown == kind) continue;

          break;
        }

        if (CodecKind_Unknown == kind) continue; // make sure this codec type is understood

        if (receiverInfo->mParameters.mEncodingParameters.size() < 1) {
          // special case where this is a "match all" for the codec
          outReceiverInfo = receiverInfo;
          goto found_receiver;
        }

        for (auto encodingIter = receiverInfo->mParameters.mEncodingParameters.begin(); encodingIter != receiverInfo->mParameters.mEncodingParameters.end(); ++encodingIter) {

          auto &encodingInfo = (*encodingIter);

          if (encodingInfo.mCodecPayloadType.hasValue()) {
            if (encodingInfo.mCodecPayloadType.value() != payloadType) continue;  // do not allow non-matching codec types to match
          }

          switch (kind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_Video:
            case CodecKind_AV:     {
              if (encodingInfo.mSSRC.hasValue()) break; // cannot match if there was an SSRC already attached
              goto found_receiver;
            }
            case CodecKind_RTX:    {
              if (encodingInfo.mRTX.hasValue()) break; // cannot match if there was an SSRC already attached
              goto found_receiver;
            }
            case CodecKind_FEC:    {
              if (encodingInfo.mFEC.hasValue()) break; // cannot match if there was an SSRC already attached
              goto found_receiver;
            }
          }
        }

        continue;

      found_receiver:
        {
          if (!outReceiverInfo) outReceiverInfo = receiverInfo;
          if (outReceiverInfo->mOrderID < receiverInfo->mOrderID) continue; // smaller = older (and better match)

          // this is a better match
          outReceiverInfo = receiverInfo;
          foundKind = kind;
        }
      }

      if (!outReceiverInfo) return false;

      ReceiverInfoPtr replacementInfo;

      // scope: fill in SSRC in encoding parameters
      {
        if (outReceiverInfo->mParameters.mEncodingParameters.size() < 1) goto insert_ssrc_into_table;

        replacementInfo = make_shared<ReceiverInfo>(*outReceiverInfo);

        auto encodingIter = outReceiverInfo->mParameters.mEncodingParameters.begin();
        auto replacementIter = replacementInfo->mParameters.mEncodingParameters.begin();

        for (; encodingIter != outReceiverInfo->mParameters.mEncodingParameters.end(); ++encodingIter, ++replacementIter) {

          ASSERT(replacementIter != replacementInfo->mParameters.mEncodingParameters.end())

          auto &encodingInfo = (*encodingIter);
          auto &replaceEncodingInfo = (*encodingIter);

          switch (foundKind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_Video:
            case CodecKind_AV:     {
              if (encodingInfo.mSSRC.hasValue()) break; // cannot match if there was an SSRC already attached
              replaceEncodingInfo.mSSRC = rtpPacket.ssrc();
              goto replace_receiver;
            }
            case CodecKind_RTX:    {
              if (encodingInfo.mRTX.hasValue()) break; // cannot match if there was an SSRC already attached
              RTXParameters rtx;
              rtx.mSSRC = rtpPacket.ssrc();
              replaceEncodingInfo.mRTX = rtx;
              goto replace_receiver;
            }
            case CodecKind_FEC:    {
              if (encodingInfo.mFEC.hasValue()) break; // cannot match if there was an SSRC already attached
              goto insert_ssrc_into_table;
              FECParameters fec;
              fec.mSSRC = rtpPacket.ssrc();
              replaceEncodingInfo.mFEC = fec;
              goto replace_receiver;
            }
          }
        }

        ASSERT(false)
        return false;
      }

    replace_receiver:
      {
        ZS_LOG_DEBUG(log("filled in SSSRC value in receiver (thus replacing existing receiver)") + outReceiverInfo->toDebug())

        setReceiverInfo(replacementInfo);
        outReceiverInfo = replacementInfo;
      }

    insert_ssrc_into_table:
      {
        ZS_LOG_DEBUG(log("creating a new SSRC entry in SSRC table (based on payload type matching)") + outReceiverInfo->toDebug())

        setReceiverInfo(replacementInfo);
        outReceiverInfo = replacementInfo;

        mSSRCTable[rtpPacket.ssrc()] = replacementInfo;
        reattemptDelivery();
      }

      return true;
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

        RTPPacket::MidHeaderExtension mid(*ext);

        String muxID(mid.mid());
        if (!muxID.hasData()) continue;

        auto foundMuxEntry = mSSRCToMuxTable.find(rtpPacket.ssrc());
        if (foundMuxEntry != mSSRCToMuxTable.end()) {
          auto &timeMuxPair = (*foundMuxEntry).second;
          timeMuxPair = TimeMuxPair(zsLib::now(), muxID);
        } else {
          mSSRCToMuxTable[rtpPacket.ssrc()] = TimeMuxPair(zsLib::now(), muxID);
          reattemptDelivery();
        }

        return muxID;
      }

      auto found = mSSRCToMuxTable.find(rtpPacket.ssrc());
      if (found == mSSRCToMuxTable.end()) return String();

      TimeMuxPair &info = (*found).second;

      info.first = zsLib::now();  // reflect last usage of this SSRC

      return info.second;
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

        return true;
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
        auto &existingInfo = (*iter).second;
        if (existingInfo->mReceiverID != receiverInfo->mReceiverID) continue;

        // replace existing entry in receiver table
        existingInfo = receiverInfo;
      }

      for (auto iter_doNotUse = mMuxIDTable.begin(); iter_doNotUse != mMuxIDTable.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &existingInfo = (*current).second;
        if (existingInfo->mReceiverID != receiverInfo->mReceiverID) continue;

        const MuxID &oldMuxID = existingInfo->mParameters.mMuxID;
        if (oldMuxID == receiverInfo->mParameters.mMuxID) continue;

        mMuxIDTable.erase(current);
      }

      if (receiverInfo->mParameters.mMuxID.hasData()) {
        // replace existing receiver info in mux table
        mMuxIDTable[receiverInfo->mParameters.mMuxID] = receiverInfo;
        reattemptDelivery();
      }

      for (auto iter = receiverInfo->mParameters.mEncodingParameters.begin(); iter != receiverInfo->mParameters.mEncodingParameters.end(); ++iter) {
        auto &encodingInfo = (*iter);

        if (encodingInfo.mSSRC.hasValue()) {
          mSSRCTable[encodingInfo.mSSRC.value()] = receiverInfo;
          reattemptDelivery();

          if (receiverInfo->mParameters.mMuxID.hasData()) {
            mSSRCToMuxTable[encodingInfo.mSSRC.value()] = TimeMuxPair(zsLib::now(), receiverInfo->mParameters.mMuxID);
          }
        }
        if (encodingInfo.mRTX.hasValue()) {
          mSSRCTable[encodingInfo.mRTX.value().mSSRC] = receiverInfo;
          reattemptDelivery();

          if (receiverInfo->mParameters.mMuxID.hasData()) {
            mSSRCToMuxTable[encodingInfo.mRTX.value().mSSRC] = TimeMuxPair(zsLib::now(), receiverInfo->mParameters.mMuxID);
          }
        }
        if (encodingInfo.mFEC.hasValue()) {
          mSSRCTable[encodingInfo.mFEC.value().mSSRC] = receiverInfo;
          reattemptDelivery();

          if (receiverInfo->mParameters.mMuxID.hasData()) {
            mSSRCToMuxTable[encodingInfo.mFEC.value().mSSRC] = TimeMuxPair(zsLib::now(), receiverInfo->mParameters.mMuxID);
          }
        }
      }

      // point to replacement list
      mReceivers = receivers;
    }

    //-------------------------------------------------------------------------
    void RTPListener::processByes(const RTCPPacket &rtcpPacket)
    {
      for (auto bye = rtcpPacket.firstBye(); NULL != bye; bye = bye->nextBye()) {
        for (size_t index = 0; index < bye->sc(); ++index) {
          auto byeSSRC = bye->ssrc(index);

          // scope: clean SSRC to mux id table
          {
            auto found = mSSRCToMuxTable.find(byeSSRC);
            if (found != mSSRCToMuxTable.end()) {
              auto muxID = (*found).second.second;

              ZS_LOG_TRACE(log("removing ssrc to mux table entry due to BYE") + ZS_PARAM("ssrc", byeSSRC) + ZS_PARAM("mux id", muxID))

              mSSRCToMuxTable.erase(found);
            }
          }

          // scope: clean normal SSRC table
          {
            auto found = mSSRCTable.find(byeSSRC);
            if (found != mSSRCTable.end()) {
              auto receiverInfo = (*found).second;
              ZS_LOG_TRACE(log("removing ssrc table entry due to BYE") + ZS_PARAM("ssrc", byeSSRC) + receiverInfo->toDebug())
              mSSRCTable.erase(found);
            }
          }

          // scope: clean out any receiver infos that have this SSRCs
          {
            ReceiverObjectMapPtr receivers = mReceivers;

            for (auto iter = receivers->begin(); iter != receivers->end(); ++iter)
            {
              ReceiverInfoPtr &info = (*iter).second;

              // first check to see if this SSRC is inside this receiver's
              // encoding parameters but if this value was auto-filled in
              // those encoding paramters and not set by the application
              // developer.
              {
                auto iterParm = info->mParameters.mEncodingParameters.begin();
                auto iterOriginalParams = info->mParameters.mEncodingParameters.begin();

                for (; iterParm != info->mParameters.mEncodingParameters.end(); ++iterParm, ++iterOriginalParams)
                {
                  ASSERT(iterOriginalParams != info->mOriginalParameters.mEncodingParameters.end())
                  EncodingParameters &encParams = (*iterParm);
                  EncodingParameters &originalEncParams = (*iterParm);

                  if ((encParams.mSSRC.hasValue()) &&
                      (!originalEncParams.mSSRC.hasValue())) {
                    if (byeSSRC == encParams.mSSRC.value()) goto strip_ssrc;
                  }

                  if ((encParams.mRTX.hasValue()) &&
                      (!originalEncParams.mRTX.hasValue())) {
                    if (byeSSRC == encParams.mRTX.value().mSSRC) goto strip_ssrc;
                  }

                  if ((encParams.mFEC.hasValue()) &&
                      (!originalEncParams.mFEC.hasValue())) {
                    if (byeSSRC == encParams.mFEC.value().mSSRC) goto strip_ssrc;
                  }
                }

                // no stripping of SSRC is needed
                continue;
              }

            strip_ssrc:
              {
                ReceiverInfoPtr replacementInfo(make_shared<ReceiverInfo>(*info));

                auto iterParm = replacementInfo->mParameters.mEncodingParameters.begin();
                auto iterOriginalParams = replacementInfo->mParameters.mEncodingParameters.begin();

                for (; iterParm != info->mParameters.mEncodingParameters.end(); ++iterParm, ++iterOriginalParams)
                {
                  ASSERT(iterOriginalParams != info->mOriginalParameters.mEncodingParameters.end())
                  EncodingParameters &encParams = (*iterParm);
                  EncodingParameters &originalEncParams = (*iterParm);

                  // reset SSRC matches back to the original value specified
                  // by the application developer

                  if (encParams.mSSRC.hasValue()) {
                    if (byeSSRC == encParams.mSSRC.value()) {
                      encParams.mSSRC = originalEncParams.mSSRC;
                    }
                  }

                  if (encParams.mRTX.hasValue()) {
                    if (byeSSRC == encParams.mRTX.value().mSSRC) {
                      encParams.mRTX = originalEncParams.mRTX;
                    }
                  }

                  if (encParams.mFEC.hasValue()) {
                    if (byeSSRC == encParams.mFEC.value().mSSRC) {
                      encParams.mFEC = originalEncParams.mFEC;
                    }
                  }
                }
                
                setReceiverInfo(replacementInfo);
              }
            }
          }
          
        }
      }
      
    }

    //-------------------------------------------------------------------------
    void RTPListener::processSDESMid(const RTCPPacket &rtcpPacket)
    {
      for (auto sdes = rtcpPacket.firstSDES(); NULL != sdes; sdes = sdes->nextSDES()) {

        for (auto chunk = sdes->firstChunk(); NULL != chunk; chunk = chunk->next()) {
          for (auto mid = chunk->firstMid(); NULL != mid; mid = mid->next()) {
            auto foundMuxEntry = mSSRCToMuxTable.find(chunk->ssrc());
            if (foundMuxEntry != mSSRCToMuxTable.end()) {
              auto &timeMuxPair = (*foundMuxEntry).second;
              timeMuxPair = TimeMuxPair(zsLib::now(), String(mid->mid()));
            } else {
              mSSRCToMuxTable[chunk->ssrc()] = TimeMuxPair(zsLib::now(), String(mid->mid()));
              reattemptDelivery();
            }
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::processSenderReports(const RTCPPacket &rtcpPacket)
    {
      for (auto sr = rtcpPacket.firstSenderReport(); NULL != sr; sr = sr->nextSenderReport()) {
        SSRCType ssrc = sr->ssrcOfSender();

        auto found = mSSRCToMuxTable.find(ssrc);
        if (found == mSSRCToMuxTable.end()) continue;

        auto &timeMuxPair = (*found).second;

        // this SSRC is still active (keep the SSRC to mux ID binding alive)
        timeMuxPair.first = zsLib::now();
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::reattemptDelivery()
    {
      if (mReattemptRTPDelivery) return;
      mReattemptRTPDelivery = true;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
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
