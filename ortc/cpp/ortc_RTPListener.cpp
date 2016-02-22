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
#include <ortc/internal/ortc_RTPTypes.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_Tracing.h>
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
    static bool shouldFilter(IRTPTypes::HeaderExtensionURIs extensionURI)
    {
      switch (extensionURI) {
        case IRTPTypes::HeaderExtensionURI_Unknown:                           return true;
        case IRTPTypes::HeaderExtensionURI_MuxID:                             return false;
      //case IRTPTypes::HeaderExtensionURI_MID:                               return false;
        case IRTPTypes::HeaderExtensionURI_ClienttoMixerAudioLevelIndication: return true;
        case IRTPTypes::HeaderExtensionURI_MixertoClientAudioLevelIndication: return true;
        case IRTPTypes::HeaderExtensionURI_FrameMarking:                      return true;
        case IRTPTypes::HeaderExtensionURI_RID:                               return false;
        case IRTPTypes::HeaderExtensionURI_3gpp_VideoOrientation:             return true;
        case IRTPTypes::HeaderExtensionURI_3gpp_VideoOrientation6:            return true;
      }
      return true;
    }
    

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

      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_SSRC_TIMEOUT_IN_SECONDS, 60);
      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_UNHANDLED_EVENTS_TIMEOUT_IN_SECONDS, 60);

      UseSettings::setUInt(ORTC_SETTING_RTP_LISTENER_ONLY_RESOLVE_AMBIGUOUS_PAYLOAD_MAPPING_IF_ACTIVITY_DIFFERS_IN_MILLISECONDS, 5*1000);
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

    //-------------------------------------------------------------------------
    RTPListener::SSRCInfoPtr RTPListener::ReceiverInfo::registerSSRCUsage(SSRCInfoPtr ssrcInfo)
    {
      mRegisteredSSRCs[ssrcInfo->mSSRC] = ssrcInfo;
      return ssrcInfo;
    }

    //-------------------------------------------------------------------------
    void RTPListener::ReceiverInfo::unregisterSSRCUsage(SSRCType ssrc)
    {
      auto found = mRegisteredSSRCs.find(ssrc);
      if (found == mRegisteredSSRCs.end()) return;
      mRegisteredSSRCs.erase(found);
    }

    //---------------------------------------------------------------------------
    ElementPtr RTPListener::ReceiverInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPListener::ReceiverInfo");

      UseServicesHelper::debugAppend(resultEl, "order id", mOrderID);
      UseServicesHelper::debugAppend(resultEl, "receiver id", mReceiverID);
      UseServicesHelper::debugAppend(resultEl, "receiver", ((bool)mReceiver.lock()));

      UseServicesHelper::debugAppend(resultEl, "kind", mKind.hasValue() ? IMediaStreamTrackTypes::toString(mKind) : (const char *)NULL);
      UseServicesHelper::debugAppend(resultEl, mFilledParameters.toDebug());
      UseServicesHelper::debugAppend(resultEl, mOriginalParameters.toDebug());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener::SSRCInfo
    #pragma mark

    //---------------------------------------------------------------------------
    RTPListener::SSRCInfo::SSRCInfo() :
      mLastUsage(zsLib::now())
    {
    }

    //---------------------------------------------------------------------------
    ElementPtr RTPListener::SSRCInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPListener::SSRCInfo");

      UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
      UseServicesHelper::debugAppend(resultEl, "last usage", mLastUsage);
      UseServicesHelper::debugAppend(resultEl, "mux id", mMuxID);
      UseServicesHelper::debugAppend(resultEl, mReceiverInfo ? mReceiverInfo->toDebug() : ElementPtr());

      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener::UnhandledEventInfo
    #pragma mark

    //---------------------------------------------------------------------------
    bool RTPListener::UnhandledEventInfo::operator<(const UnhandledEventInfo &op2) const
    {
      if (mSSRC < op2.mSSRC) return true;
      if (mSSRC > op2.mSSRC) return false;
      if (mCodecPayloadType < op2.mCodecPayloadType) return true;
      if (mCodecPayloadType > op2.mCodecPayloadType) return false;

      if (mMuxID < op2.mMuxID) return true;
      if (mMuxID > op2.mMuxID) return false;

      if (mRID < op2.mRID) return true;
      //if (mRID > op2.mRID) return false;  // test not needed (will be false either way)
      return false;
    }
    
    //---------------------------------------------------------------------------
    ElementPtr RTPListener::UnhandledEventInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPListener::UnhandledEventInfo");

      UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
      UseServicesHelper::debugAppend(resultEl, "codec payload type", mCodecPayloadType);
      UseServicesHelper::debugAppend(resultEl, "mux id", mMuxID);
      UseServicesHelper::debugAppend(resultEl, "rid", mRID);

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
                             UseRTPTransportPtr transport
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mRTPTransport(transport),
      mMaxBufferedRTPPackets(SafeInt<decltype(mMaxBufferedRTPPackets)>(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTP_PACKETS_IN_BUFFER))),
      mMaxRTPPacketAge(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTP_PACKETS_IN_SECONDS)),
      mMaxBufferedRTCPPackets(SafeInt<decltype(mMaxBufferedRTCPPackets)>(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_RTCP_PACKETS_IN_BUFFER))),
      mMaxRTCPPacketAge(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS)),
      mReceivers(make_shared<ReceiverObjectMap>()),
      mSenders(make_shared<SenderObjectMap>()),
      mAmbigousPayloadMappingMinDifference(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_ONLY_RESOLVE_AMBIGUOUS_PAYLOAD_MAPPING_IF_ACTIVITY_DIFFERS_IN_MILLISECONDS)),
      mSSRCTableExpires(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_SSRC_TIMEOUT_IN_SECONDS)),
      mUnhanldedEventsExpires(UseSettings::getUInt(ORTC_SETTING_RTP_LISTENER_UNHANDLED_EVENTS_TIMEOUT_IN_SECONDS))
    {
      EventWriteOrtcRtpListenerCreate(
                                      __func__,
                                      mID,
                                      ((bool)transport) ? transport->getID() : 0,
                                      mMaxBufferedRTPPackets,
                                      mMaxRTPPacketAge.count(),
                                      mMaxBufferedRTCPPackets,
                                      mAmbigousPayloadMappingMinDifference.count(),
                                      mSSRCTableExpires.count(),
                                      mUnhanldedEventsExpires.count()
                                      );

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

      if (mSSRCTableExpires < Seconds(1)) {
        mSSRCTableExpires = Seconds(1);
      }

      mSSRCTableTimer = Timer::create(mThisWeak.lock(), (zsLib::toMilliseconds(mSSRCTableExpires) / 2));

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
      EventWriteOrtcRtpListenerDestroy(__func__, mID);
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

          delegate->onRTPListenerUnhandledRTP(pThis, unhandledData.mSSRC, unhandledData.mCodecPayloadType, unhandledData.mMuxID.c_str(), unhandledData.mRID.c_str());
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
      EventWriteOrtcRtpListenerReceivedIncomingPacket(__func__, mID, zsLib::to_underlying(viaComponent), zsLib::to_underlying(packetType), bufferLengthInBytes, buffer);

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

          EventWriteOrtcRtpListenerBufferIncomingPacket(__func__, mID, zsLib::to_underlying(viaComponent), zsLib::to_underlying(packetType), bufferLengthInBytes, buffer);

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

        EventWriteOrtcRtpListenerBufferIncomingPacket(__func__, mID, zsLib::to_underlying(viaComponent), zsLib::to_underlying(packetType), bufferLengthInBytes, buffer);

        // provide some modest buffering
        mBufferedRTPPackets.push_back(TimeRTPPacketPair(tick, rtpPacket));

        String rid = extractRID(*rtpPacket);

        processUnhandled(muxID, rid, rtpPacket->ssrc(), rtpPacket->pt(), tick);
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
        EventWriteOrtcRtpListenerForwardIncomingPacket(__func__, mID, receiver->getID(), zsLib::to_underlying(viaComponent), zsLib::to_underlying(packetType), rtpPacket->buffer()->SizeInBytes(), rtpPacket->buffer()->BytePtr());
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
          EventWriteOrtcRtpListenerForwardIncomingPacket(__func__, mID, receiver->getID(), zsLib::to_underlying(viaComponent), zsLib::to_underlying(packetType), rtcpPacket->buffer()->SizeInBytes(), rtcpPacket->buffer()->BytePtr());
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
          EventWriteOrtcRtpListenerForwardIncomingPacket(__func__, mID, sender->getID(), zsLib::to_underlying(viaComponent), zsLib::to_underlying(packetType), rtcpPacket->buffer()->SizeInBytes(), rtcpPacket->buffer()->BytePtr());
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
                                       Optional<IMediaStreamTrack::Kinds> kind,
                                       UseReceiverPtr inReceiver,
                                       const Parameters &inParams,
                                       RTCPPacketList *outPacketList
                                       )
    {
      ZS_LOG_TRACE(log("registering RTP receiver") + ZS_PARAM("receiver", inReceiver->getID()) + inParams.toDebug())

      if (outPacketList) {
        outPacketList->clear();
      }

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
      replacementInfo->mKind = kind;
      replacementInfo->mFilledParameters = inParams;
      replacementInfo->mOriginalParameters = inParams;

      EventWriteOrtcRtpListenerRegisterReceiver(__func__, mID, receiverID, replacementInfo->mOrderID, kind.hasValue() ? IMediaStreamTrackTypes::toString(kind.value()) : NULL);

      auto found = mReceivers->find(receiverID);
      if (found == mReceivers->end()) {
        setReceiverInfo(replacementInfo);
        goto finalize_registration;
      }

      // scope: check to see if anything important changed in the encoding parameters
      {
        auto &existingInfo = (*found).second;

        size_t indexExisting = 0;
        for (auto iterExistingEncoding = existingInfo->mFilledParameters.mEncodings.begin(); iterExistingEncoding != existingInfo->mFilledParameters.mEncodings.begin(); ++iterExistingEncoding, ++indexExisting) {
          auto &existinEncodingInfo = (*iterExistingEncoding);

          if (existinEncodingInfo.mEncodingID.hasData()) {
            // scope: search replacement for same encoding ID
            {
              for (auto iterReplacementEncoding = replacementInfo->mFilledParameters.mEncodings.begin(); iterReplacementEncoding != replacementInfo->mFilledParameters.mEncodings.end(); ++iterReplacementEncoding) {
                auto &replacementEncodingInfo = (*iterReplacementEncoding);
                if (replacementEncodingInfo.mEncodingID == existinEncodingInfo.mEncodingID) {
                  // these encoding are identical
                  handleDeltaChanges(replacementInfo, existinEncodingInfo, replacementEncodingInfo);
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
            for (auto iterReplacementEncoding = replacementInfo->mFilledParameters.mEncodings.begin(); iterReplacementEncoding != replacementInfo->mFilledParameters.mEncodings.end(); ++iterReplacementEncoding, ++indexReplacement) {
              if (indexExisting != indexReplacement) continue;

              auto &replacementEncodingInfo = (*iterReplacementEncoding);
              if (replacementEncodingInfo.mEncodingID.hasData()) {
                unregisterEncoding(existinEncodingInfo);
                goto done_search_by_index;
              }

              handleDeltaChanges(replacementInfo, existinEncodingInfo, replacementEncodingInfo);
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

        if (outPacketList) {
          for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter) {
            auto buffer = (*iter).second;
            outPacketList->push_back(buffer);
          }
        }

        reattemptDelivery();
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterReceiver(UseReceiver &inReceiver)
    {
      AutoRecursiveLock lock(*this);

      ReceiverID receiverID = inReceiver.getID();

      EventWriteOrtcRtpListenerUnregisterReceiver(__func__, mID, receiverID);

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

        auto ssrc = (*current).first;
        auto &ssrcInfo = (*current).second;

        ReceiverInfoPtr &receiverInfo = ssrcInfo->mReceiverInfo;
        if (!receiverInfo) continue;

        if (receiverInfo->mReceiverID != receiverID) continue;

        ZS_LOG_TRACE(log("removing SSRC mapping to receiver") + ZS_PARAM("ssrc", ssrc) + receiverInfo->toDebug())

        EventWriteOrtcRtpListenerSsrcTableEntryRemoved(__func__, mID, ((bool)ssrcInfo->mReceiverInfo) ? ssrcInfo->mReceiverInfo->mReceiverID : 0, ssrcInfo->mSSRC, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(), ssrcInfo->mMuxID, "receiver removed");
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
    void RTPListener::getPackets(RTCPPacketList &outPacketList)
    {
      AutoRecursiveLock lock(*this);

      expireRTCPPackets();

      for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter)
      {
        auto buffer = (*iter).second;
        outPacketList.push_back(buffer);
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::notifyUnhandled(
                                      const String &muxID,
                                      const String &rid,
                                      IRTPTypes::SSRCType ssrc,
                                      IRTPTypes::PayloadType payloadType
                                      )
    {
      ZS_LOG_TRACE(log("notified that a receiver did not handle a delivered packet") + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload", payloadType))

      AutoRecursiveLock lock(*this);
      processUnhandled(muxID, rid, ssrc, payloadType, zsLib::now());
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
      EventWriteOrtcRtpListenerRegisterSender(__func__, mID, inSender->getID());

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
      EventWriteOrtcRtpListenerUnregisterSender(__func__, mID, inSender.getID());

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
      EventWriteOrtcRtpListenerInternalWakeEventFired(__func__, mID);
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
      EventWriteOrtcRtpListenerInternalTimerEventFired(__func__, mID, timer->getID(), NULL);
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);

      if (timer == mSSRCTableTimer) {
        EventWriteOrtcRtpListenerInternalTimerEventFired(__func__, mID, timer->getID(), "sssrc table timer");

        expireRTPPackets();   // might as well expire these too now
        expireRTCPPackets();  // might as well expire these too now

        auto adjustedTick = zsLib::now() - mSSRCTableExpires;

        // now =  N; then = T; expire = E; adjusted = A;    N-E = A; if A > T then expired
        // now = 10; then = 5; expiry = 3;                 10-3 = 7;    7 > 5 = expired (true)
        // now =  6; then = 5; expiry = 3;                  6-3 = 3;    3 > 5 = not expired (false)

        for (auto iter_doNotUse = mSSRCTable.begin(); iter_doNotUse != mSSRCTable.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &ssrcInfo = (*current).second;

          const Time &lastReceived = ssrcInfo->mLastUsage;

          if (!(adjustedTick > lastReceived)) continue;

          ZS_LOG_TRACE(log("expiring SSRC mapping") + ssrcInfo->toDebug() + ZS_PARAM("adjusted tick", adjustedTick))
          EventWriteOrtcRtpListenerSsrcTableEntryRemoved(__func__, mID, ((bool)ssrcInfo->mReceiverInfo) ? ssrcInfo->mReceiverInfo->mReceiverID : 0, ssrcInfo->mSSRC, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(), ssrcInfo->mMuxID, "expired");
          mSSRCTable.erase(current);
        }

        return;
      }

      if (timer == mUnhanldedEventsTimer) {
        EventWriteOrtcRtpListenerInternalTimerEventFired(__func__, mID, timer->getID(), "unhandled events timer");

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

      EventWriteOrtcRtpListenerInternalTimerEventFired(__func__, mID, timer->getID(), "obsolete");

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

      EventWriteOrtcRtpListenerForwardIncomingPacket(__func__, mID, receiver->getID(), zsLib::to_underlying(viaComponent), zsLib::to_underlying(IICETypes::Component_RTP), packet->buffer()->SizeInBytes(), packet->buffer()->BytePtr());
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

      EventWriteOrtcRtpListenerStep(__func__, mID);

      // ... other steps here ...
      if (!stepAttemptDelivery()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("listener is not ready"))
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
      EventWriteOrtcRtpListenerStep(__func__, mID);

      if (!mReattemptRTPDelivery) {
        ZS_LOG_TRACE(log("no need to reattempt deliver at this time"))
        return true;
      }

      ZS_LOG_DEBUG(log("will attempt to deliver any buffered RTP packets"))

      mReattemptRTPDelivery = false;

      expireRTPPackets();

      size_t previousSize = 0;

      do
      {
        previousSize = mBufferedRTPPackets.size();
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

      // NOTE: need to repetitively attempt to deliver packets as it's possible
      //       processinging some packets will then allow delivery of other
      //       packets
      } while ((mBufferedRTPPackets.size() != previousSize) &&
               (0 != mBufferedRTPPackets.size()));

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPListener::cancel()
    {
      EventWriteOrtcRtpListenerCancel(__func__, mID);

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
      mUnhandledEvents.clear();

      if (mSSRCTableTimer) {
        mSSRCTableTimer->cancel();
        mSSRCTableTimer.reset();
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
      EventWriteOrtcRtpListenerStateChangedEventFired(__func__, mID, toString(mCurrentState));

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
      EventWriteOrtcRtpListenerErrorEventFired(__func__, mID, errorCode, reason);

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    void RTPListener::expireRTPPackets()
    {
      auto tick = zsLib::now();

      while (mBufferedRTPPackets.size() > 0) {
        auto &info = mBufferedRTPPackets.front();
        auto &packetTime = info.first;
        auto &packet = info.second;
        (void)packet;

        {
          if (mBufferedRTPPackets.size() > mMaxBufferedRTPPackets) goto expire_packet;
          if (packetTime + mMaxRTPPacketAge < tick) goto expire_packet;
          break;
        }

      expire_packet:
        {
          EventWriteOrtcRtpListenerDisposeBufferedIncomingPacket(__func__, mID, zsLib::to_underlying(IICETypes::Component_RTP), packet->buffer()->SizeInBytes(), packet->buffer()->BytePtr());
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
        auto &info = mBufferedRTCPPackets.front();
        auto &packetTime = info.first;
        auto &packet = info.second;
        (void)packet;

        {
          if (mBufferedRTCPPackets.size() > mMaxBufferedRTCPPackets) goto expire_packet;
          if (packetTime + mMaxRTCPPacketAge < tick) goto expire_packet;
          break;
        }

      expire_packet:
        {
          EventWriteOrtcRtpListenerDisposeBufferedIncomingPacket(__func__, mID, zsLib::to_underlying(IICETypes::Component_RTCP), packet->buffer()->SizeInBytes(), packet->buffer()->BytePtr());
          ZS_LOG_TRACE(log("expiring buffered rtcp packet") + ZS_PARAM("tick", tick) + ZS_PARAM("packet time (s)", packetTime) + ZS_PARAM("total", mBufferedRTCPPackets.size()))
          mBufferedRTCPPackets.pop_front();
        }
      }
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

        EventWriteOrtcRtpListenerRegisterHeaderExtension(__func__, mID, objectID, IRTPTypes::toString(extension.mHeaderExtensionURI), extension.mLocalID, extension.mEncrypted, extension.mReferences.size());

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

      extension.mReferences[objectID] = true;
      EventWriteOrtcRtpListenerRegisterHeaderExtension(__func__, mID, objectID, IRTPTypes::toString(extension.mHeaderExtensionURI), extension.mLocalID, extension.mEncrypted, extension.mReferences.size());

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

        EventWriteOrtcRtpListenerUnregisterHeaderExtension(__func__, mID, objectID, IRTPTypes::toString(extension.mHeaderExtensionURI), extension.mLocalID, extension.mEncrypted, extension.mReferences.size());

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
      outMuxID = extractMuxID(rtpPacket, outReceiverInfo);

      EventWriteOrtcRtpListenerFindMapping(__func__, mID, outMuxID, rtpPacket.buffer()->SizeInBytes(), rtpPacket.buffer()->BytePtr());

      {
        if (outReceiverInfo) goto fill_mux_id;

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

      String inMuxID = muxID;
      setSSRCUsage(rtpPacket.ssrc(), inMuxID, outReceiverInfo);

      EventWriteOrtcRtpListenerFoundMappingByMuxID(__func__, mID, ((bool)outReceiverInfo) ? outReceiverInfo->mReceiverID : 0, muxID);
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

        if ((info->mFilledParameters.mMuxID.hasData()) &&
            (muxID.hasData())) {
          if (muxID != info->mFilledParameters.mMuxID) {
            // cannot consider any receiver which has a mux id but does not
            // match this receiver's mux id
            continue;
          }
        }

        // first check to see if this SSRC is inside this receiver's
        // encoding parameters if this value was auto-filled in those encoding
        // paramters or set by the application developer.
        {
          auto iterParm = info->mFilledParameters.mEncodings.begin();

          for (; iterParm != info->mFilledParameters.mEncodings.end(); ++iterParm)
          {
            EncodingParameters &encParams = (*iterParm);

            if (encParams.mSSRC.hasValue()) {
              if (rtpPacket.ssrc() == encParams.mSSRC.value()) goto map_ssrc;
            }

            if ((encParams.mRTX.hasValue()) &&
                (encParams.mRTX.value().mSSRC.hasValue())) {
              if (rtpPacket.ssrc() == encParams.mRTX.value().mSSRC.value()) goto map_ssrc;
            }

            if ((encParams.mFEC.hasValue()) &&
                (encParams.mFEC.value().mSSRC.hasValue())) {
              if (rtpPacket.ssrc() == encParams.mFEC.value().mSSRC.value()) goto map_ssrc;
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
          String inMuxID = muxID;
          setSSRCUsage(rtpPacket.ssrc(), inMuxID, outReceiverInfo);
          EventWriteOrtcRtpListenerFoundMappingBySsrc(__func__, mID, ((bool)outReceiverInfo) ? outReceiverInfo->mReceiverID : 0, rtpPacket.ssrc());
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
      EncodingParameters *foundEncoding = NULL;
      CodecKinds foundCodecKind = CodecKind_Unknown;

      Time lastMatchUsageTime {};

      for (auto iter = mReceivers->begin(); iter != mReceivers->end(); ++iter) {
        auto &receiverInfo = (*iter).second;

        if ((receiverInfo->mFilledParameters.mMuxID.hasData()) &&
            (muxID.hasData())) {
          if (muxID != receiverInfo->mFilledParameters.mMuxID) {
            // cannot consider any receiver which has a mux id but does not
            // match this receiver's mux id
            continue;
          }
        }

        const CodecParameters *codecParams = NULL;
        IRTPTypes::SupportedCodecs supportedCodec {};
        IRTPTypes::CodecKinds codecKind {};

        EncodingParameters *baseEncoding = NULL;
        auto matchEncoding = RTPTypesHelper::pickEncodingToFill(receiverInfo->mKind, rtpPacket.pt(), receiverInfo->mFilledParameters, codecParams, supportedCodec, codecKind, baseEncoding);

        if (receiverInfo->mFilledParameters.mEncodings.size() < 1) {
          if (!codecParams) {
            ZS_LOG_WARNING(Debug, log("unable to find a codec for packet") + ZS_PARAM("packet ssrc", rtpPacket.ssrc()) + ZS_PARAM("payload type", rtpPacket.pt()) + receiverInfo->mFilledParameters.toDebug())
            return false;
          }

          // special case where this is a "latch all" for the codec
          outReceiverInfo = receiverInfo;
          goto found_receiver;
        }

        if (NULL == matchEncoding) continue; // did not find an appropriate encoding
        ASSERT(NULL != baseEncoding)  // has to always have a base

        {
          switch (codecKind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_AudioSupplemental:
            case CodecKind_Video:
            case CodecKind_AV:
            case CodecKind_Data:    break;

            case CodecKind_RTX:
            case CodecKind_FEC:     {

              auto ssrc = baseEncoding->mSSRC.value();

              auto foundSSRC = mSSRCTable.find(ssrc);
              if (foundSSRC == mSSRCTable.end()) {
                ZS_LOG_WARNING(Trace, log("catch not match encoding as master SSRC was not active recently") + receiverInfo->toDebug())
                continue;
              }

              auto &ssrcInfo = (*foundSSRC).second;

              if (outReceiverInfo) {
                // look at the latest time the master SSRC was used

                auto tick = zsLib::now();

                auto diffLast = tick - lastMatchUsageTime;
                auto diffCurrent = tick - ssrcInfo->mLastUsage;

                if ((diffLast < mAmbigousPayloadMappingMinDifference) &&
                    (diffCurrent < mAmbigousPayloadMappingMinDifference)) {
                  ZS_LOG_WARNING(Debug, log("ambiguity exists to which receiver the packet should match because both channels have been recendly active (thus cannot pick any encoding)") + ZS_PARAM("tick", tick) + ZS_PARAM("match time", lastMatchUsageTime) + ZS_PARAM("ambiguity window", mAmbigousPayloadMappingMinDifference) + ZS_PARAM("diff last", diffLast) + ZS_PARAM("diff current", diffCurrent) + ssrcInfo->toDebug() + ZS_PARAM("previous find", outReceiverInfo->toDebug()) + ZS_PARAM("found", receiverInfo->toDebug()))
                  return false;
                }

                if (ssrcInfo->mLastUsage < lastMatchUsageTime) {
                  ZS_LOG_WARNING(Trace, log("possible ambiguity in match (but going with previous more recent usage)") + ZS_PARAM("match time", lastMatchUsageTime) + ssrcInfo->toDebug())
                  continue;
                }

                ZS_LOG_WARNING(Trace, log("possible ambiguity in match (going with this as more recent in usage)") + ZS_PARAM("match time", lastMatchUsageTime) + ssrcInfo->toDebug() + ZS_PARAM("using", receiverInfo->toDebug()) + ZS_PARAM("previous found", outReceiverInfo->toDebug()))

                lastMatchUsageTime = ssrcInfo->mLastUsage;
                outReceiverInfo = receiverInfo;
                foundEncoding = matchEncoding;
                foundCodecKind = codecKind;
              } else {
                ZS_LOG_TRACE(log("found likely match") + receiverInfo->toDebug() + ssrcInfo->toDebug())

                lastMatchUsageTime = ssrcInfo->mLastUsage;
                outReceiverInfo = receiverInfo;
                foundEncoding = matchEncoding;
                foundCodecKind = codecKind;
              }
              continue;
            }
          }

          if (!outReceiverInfo) outReceiverInfo = receiverInfo;
          if (outReceiverInfo->mOrderID < receiverInfo->mOrderID) continue; // smaller = older (and thus better match)

          // this is a better match
          outReceiverInfo = receiverInfo;
          foundEncoding = matchEncoding;
        }
      }

      if (!outReceiverInfo) return false;

    found_receiver: {}

      ReceiverInfoPtr replacementInfo;

      // scope: fill in SSRC in encoding parameters
      {
        replacementInfo = make_shared<ReceiverInfo>(*outReceiverInfo);

        if (outReceiverInfo->mFilledParameters.mEncodings.size() < 1) goto insert_ssrc_into_table;

        auto encodingIter = outReceiverInfo->mFilledParameters.mEncodings.begin();
        auto replacementIter = replacementInfo->mFilledParameters.mEncodings.begin();

        for (; encodingIter != outReceiverInfo->mFilledParameters.mEncodings.end(); ++encodingIter, ++replacementIter) {

          ASSERT(replacementIter != replacementInfo->mFilledParameters.mEncodings.end())

          auto &encoding = (*encodingIter);
          auto &replaceEncoding = (*encodingIter);

          if ((&encoding) != foundEncoding) continue; // this is not the encoding you are searching for...

          switch (foundCodecKind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_Video:
            case CodecKind_AV:
            case CodecKind_Data:
            {
              replaceEncoding.mSSRC = rtpPacket.ssrc();
              replaceEncoding.mCodecPayloadType = rtpPacket.pt();
              goto replace_receiver;
            }
            case CodecKind_AudioSupplemental:
            {
              goto replace_receiver;
            }
            case CodecKind_RTX:
            {
              replaceEncoding.mRTX.value().mSSRC = rtpPacket.ssrc();
              replaceEncoding.mRTX.value().mPayloadType = rtpPacket.pt();
              goto replace_receiver;
            }
            case CodecKind_FEC:
            {
              replaceEncoding.mFEC.value().mSSRC = rtpPacket.ssrc();
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

        String inMuxID = muxID;
        setSSRCUsage(rtpPacket.ssrc(), inMuxID, replacementInfo);
        EventWriteOrtcRtpListenerFoundMappingByPayloadType(__func__, mID, ((bool)outReceiverInfo) ? outReceiverInfo->mReceiverID : 0, rtpPacket.pt());
      }

      return true;
    }

    //-------------------------------------------------------------------------
    String RTPListener::extractMuxID(
                                     const RTPPacket &rtpPacket,
                                     ReceiverInfoPtr &ioReceiverInfo
                                     )
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

        setSSRCUsage(rtpPacket.ssrc(), muxID, ioReceiverInfo);
        return muxID;
      }

      String muxID;
      setSSRCUsage(rtpPacket.ssrc(), muxID, ioReceiverInfo);

      return muxID;
    }

    //-------------------------------------------------------------------------
    String RTPListener::extractRID(const RTPPacket &rtpPacket)
    {
      for (auto ext = rtpPacket.firstHeaderExtension(); NULL != ext; ext = ext->mNext) {
        LocalID localID = static_cast<LocalID>(ext->mID);
        auto found = mRegisteredExtensions.find(localID);
        if (found == mRegisteredExtensions.end()) continue; // header extension is not understood

        RegisteredHeaderExtension &headerInfo = (*found).second;

        if (IRTPTypes::HeaderExtensionURI_RID != headerInfo.mHeaderExtensionURI) continue;

        RTPPacket::RidHeaderExtension rid(*ext);

        String ridStr(rid.rid());
        if (!ridStr.hasData()) continue;

        return ridStr;
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

      if (ioReceiverInfo->mFilledParameters.mMuxID.hasData()) {
        if (muxID != ioReceiverInfo->mFilledParameters.mMuxID) {
          // already has a MuxID and this isn't it!
          ZS_LOG_WARNING(Debug, log("receiver mux id and packet mux id are mis-matched") + ZS_PARAM("mux id", muxID) + ioReceiverInfo->toDebug())
          return false;
        }

        return true;
      }

      ReceiverInfoPtr info(make_shared<ReceiverInfo>(*ioReceiverInfo));
      info->mFilledParameters.mMuxID = muxID;
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
        SSRCInfoPtr &existingInfo = (*iter).second;
        if (!existingInfo->mReceiverInfo) continue;
        if (existingInfo->mReceiverInfo->mReceiverID != receiverInfo->mReceiverID) continue;

        // replace existing entry in receiver table
        existingInfo->mReceiverInfo = receiverInfo;
      }

      for (auto iter_doNotUse = mRegisteredSSRCs.begin(); iter_doNotUse != mRegisteredSSRCs.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        SSRCInfoPtr existingInfo = (*current).second.lock();
        if (!existingInfo) {
          mRegisteredSSRCs.erase(current);
          continue;
        }

        if (!existingInfo->mReceiverInfo) continue;
        if (existingInfo->mReceiverInfo->mReceiverID != receiverInfo->mReceiverID) continue;

        // replace existing entry in receiver table
        existingInfo->mReceiverInfo = receiverInfo;
      }

      for (auto iter_doNotUse = mMuxIDTable.begin(); iter_doNotUse != mMuxIDTable.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &existingInfo = (*current).second;
        if (existingInfo->mReceiverID != receiverInfo->mReceiverID) continue;

        const MuxID &oldMuxID = existingInfo->mFilledParameters.mMuxID;
        if (oldMuxID == receiverInfo->mFilledParameters.mMuxID) continue;

        mMuxIDTable.erase(current);
      }

      if (receiverInfo->mFilledParameters.mMuxID.hasData()) {
        // replace existing receiver info in mux table
        mMuxIDTable[receiverInfo->mFilledParameters.mMuxID] = receiverInfo;
        reattemptDelivery();
      }

      for (auto iter = receiverInfo->mOriginalParameters.mEncodings.begin(); iter != receiverInfo->mOriginalParameters.mEncodings.end(); ++iter) {
        auto &encodingInfo = (*iter);

        String muxID = receiverInfo->mOriginalParameters.mMuxID;

        if (encodingInfo.mSSRC.hasValue()) {
          registerSSRCUsage(receiverInfo->registerSSRCUsage(setSSRCUsage(encodingInfo.mSSRC.value(), muxID, receiverInfo)));
        }
        if ((encodingInfo.mRTX.hasValue()) &&
            (encodingInfo.mRTX.value().mSSRC.hasValue())) {
          registerSSRCUsage(receiverInfo->registerSSRCUsage(setSSRCUsage(encodingInfo.mRTX.value().mSSRC.value(), muxID, receiverInfo)));
        }
        if ((encodingInfo.mFEC.hasValue()) &&
            (encodingInfo.mFEC.value().mSSRC.hasValue())) {
          registerSSRCUsage(receiverInfo->registerSSRCUsage(setSSRCUsage(encodingInfo.mFEC.value().mSSRC.value(), muxID, receiverInfo)));
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

          // scope: clean normal SSRC table
          {
            auto found = mSSRCTable.find(byeSSRC);
            if (found != mSSRCTable.end()) {
              auto &ssrcInfo = (*found).second;
              ZS_LOG_TRACE(log("removing ssrc table entry due to BYE") + ZS_PARAM("ssrc", byeSSRC) + ssrcInfo->toDebug())
              EventWriteOrtcRtpListenerSsrcTableEntryRemoved(__func__, mID, ((bool)ssrcInfo->mReceiverInfo) ? ssrcInfo->mReceiverInfo->mReceiverID : 0, ssrcInfo->mSSRC, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(), ssrcInfo->mMuxID, "bye");
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
                auto iterParm = info->mFilledParameters.mEncodings.begin();
                auto iterOriginalParams = info->mOriginalParameters.mEncodings.begin();

                for (; iterParm != info->mFilledParameters.mEncodings.end(); ++iterParm, ++iterOriginalParams)
                {
                  ASSERT(iterOriginalParams != info->mOriginalParameters.mEncodings.end())
                  EncodingParameters &encParams = (*iterParm);
                  EncodingParameters &originalEncParams = (*iterOriginalParams);

                  if ((encParams.mSSRC.hasValue()) &&
                      (!originalEncParams.mSSRC.hasValue())) {
                    if (byeSSRC == encParams.mSSRC.value()) goto strip_ssrc;
                  }

                  if (((encParams.mRTX.hasValue()) &&
                       (encParams.mRTX.value().mSSRC.hasValue())) &&
                      (!  ((originalEncParams.mRTX.hasValue()) &&
                           (originalEncParams.mRTX.value().mSSRC.hasValue()))
                       )) {
                    if (byeSSRC == encParams.mRTX.value().mSSRC.value()) goto strip_ssrc;
                  }

                  if (((encParams.mFEC.hasValue()) &&
                       (encParams.mFEC.value().mSSRC.hasValue())) &&
                      (! ((originalEncParams.mFEC.hasValue()) &&
                          (originalEncParams.mFEC.value().mSSRC.hasValue()))
                       )) {
                    if (byeSSRC == encParams.mFEC.value().mSSRC.value()) goto strip_ssrc;
                  }
                }

                // no stripping of SSRC is needed
                continue;
              }

            strip_ssrc:
              {
                ReceiverInfoPtr replacementInfo(make_shared<ReceiverInfo>(*info));

                auto iterParm = replacementInfo->mFilledParameters.mEncodings.begin();
                auto iterOriginalParams = replacementInfo->mOriginalParameters.mEncodings.begin();

                for (; iterParm != info->mFilledParameters.mEncodings.end(); ++iterParm, ++iterOriginalParams)
                {
                  ASSERT(iterOriginalParams != info->mOriginalParameters.mEncodings.end())
                  EncodingParameters &encParams = (*iterParm);
                  EncodingParameters &originalEncParams = (*iterOriginalParams);

                  // reset SSRC matches back to the original value specified
                  // by the application developer

                  if (encParams.mSSRC.hasValue()) {
                    if (byeSSRC == encParams.mSSRC.value()) {
                      encParams.mSSRC = originalEncParams.mSSRC;
                    }
                  }

                  if ((encParams.mRTX.hasValue()) &&
                      (encParams.mRTX.value().mSSRC.hasValue())) {
                    if (byeSSRC == encParams.mRTX.value().mSSRC.value()) {
                      encParams.mRTX = originalEncParams.mRTX;
                    }
                  }

                  if ((encParams.mFEC.hasValue()) &&
                      (encParams.mFEC.value().mSSRC.hasValue())) {
                    if (byeSSRC == encParams.mFEC.value().mSSRC.value()) {
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
            String inMuxID = mid->mid();
            ReceiverInfoPtr ignored;
            setSSRCUsage(chunk->ssrc(), inMuxID, ignored);
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::processSenderReports(const RTCPPacket &rtcpPacket)
    {
      for (auto sr = rtcpPacket.firstSenderReport(); NULL != sr; sr = sr->nextSenderReport()) {
        String ignoredStr;
        ReceiverInfoPtr ignored;
        setSSRCUsage(sr->ssrcOfSender(), ignoredStr, ignored);
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::handleDeltaChanges(
                                         ReceiverInfoPtr replacementInfo,
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
            registerSSRCUsage(replacementInfo->registerSSRCUsage(setSSRCUsage(ioReplacement.mSSRC.value(), replacementInfo->mOriginalParameters.mMuxID, replacementInfo)));
          }
        } else {
          ioReplacement.mSSRC = existing.mSSRC;
        }
      }

      if ((existing.mRTX.hasValue()) &&
          (existing.mRTX.value().mSSRC.hasValue())) {
        if ((ioReplacement.mRTX.hasValue()) &&
            (ioReplacement.mRTX.value().mSSRC.hasValue())) {
          if (existing.mRTX.value().mSSRC.value() != ioReplacement.mRTX.value().mSSRC.value()) {
            registerSSRCUsage(replacementInfo->registerSSRCUsage(setSSRCUsage(ioReplacement.mRTX.value().mSSRC.value(), replacementInfo->mOriginalParameters.mMuxID, replacementInfo)));
          }
        } else {
          ioReplacement.mRTX = existing.mRTX;
        }
      }

      if ((existing.mFEC.hasValue()) &&
          (existing.mFEC.value().mSSRC.hasValue())) {
        if ((ioReplacement.mFEC.hasValue()) &&
            (ioReplacement.mFEC.value().mSSRC.hasValue())) {
          if (existing.mFEC.value().mSSRC.value() != ioReplacement.mFEC.value().mSSRC.value()) {
            registerSSRCUsage(replacementInfo->registerSSRCUsage(setSSRCUsage(ioReplacement.mFEC.value().mSSRC.value(), replacementInfo->mOriginalParameters.mMuxID, replacementInfo)));
          }
        } else {
          ioReplacement.mFEC = existing.mFEC;
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPListener::unregisterEncoding(const EncodingParameters &existing)
    {
    }

    //-------------------------------------------------------------------------
    RTPListener::SSRCInfoPtr RTPListener::setSSRCUsage(
                                                       SSRCType ssrc,
                                                       String &ioMuxID,
                                                       ReceiverInfoPtr &ioReceiverInfo
                                                       )
    {
      SSRCInfoPtr ssrcInfo;

      auto found = mSSRCTable.find(ssrc);

      if (found == mSSRCTable.end()) {
        auto foundWeak = mRegisteredSSRCs.find(ssrc);
        if (foundWeak != mRegisteredSSRCs.end()) {
          ssrcInfo = (*foundWeak).second.lock();
          if (!ssrcInfo) {
            mRegisteredSSRCs.erase(foundWeak);
          }
        }
      } else {
        ssrcInfo = (*found).second;
      }

      if (!ssrcInfo) {
        ssrcInfo = make_shared<SSRCInfo>();
        ssrcInfo->mSSRC = ssrc;
        if (ioMuxID.hasData()) {
          ssrcInfo->mMuxID = ioMuxID;
        } else if (ioReceiverInfo) {
          ioMuxID = ssrcInfo->mMuxID = ioReceiverInfo->mFilledParameters.mMuxID;
        }
        ssrcInfo->mReceiverInfo = ioReceiverInfo;
        EventWriteOrtcRtpListenerSsrcTableEntryAdded(__func__, mID, ((bool)ioReceiverInfo) ? ioReceiverInfo->mReceiverID : 0, ssrcInfo->mSSRC, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(), ssrcInfo->mMuxID);
        mSSRCTable[ssrc] = ssrcInfo;
        reattemptDelivery();
        return ssrcInfo;
      }

      ssrcInfo->mLastUsage = zsLib::now();

      if (ioReceiverInfo) {
        ssrcInfo->mReceiverInfo = ioReceiverInfo;
      } else {
        ioReceiverInfo = ssrcInfo->mReceiverInfo;
      }

      if (ioMuxID.hasData()) {
        if (ioMuxID != ssrcInfo->mMuxID) ssrcInfo->mMuxID = ioMuxID;
      } else if (ssrcInfo->mReceiverInfo) {
        if (ssrcInfo->mReceiverInfo->mFilledParameters.mMuxID.hasData()) {
          if (ssrcInfo->mMuxID != ssrcInfo->mReceiverInfo->mFilledParameters.mMuxID) {
            ioMuxID = ssrcInfo->mMuxID = ssrcInfo->mReceiverInfo->mFilledParameters.mMuxID;
          } else {
            ioMuxID = ssrcInfo->mMuxID;
          }
        }
      }

      EventWriteOrtcRtpListenerSsrcTableEntryUpdated(__func__, mID, ((bool)ssrcInfo->mReceiverInfo) ? ssrcInfo->mReceiverInfo->mReceiverID : 0, ssrcInfo->mSSRC, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(), ssrcInfo->mMuxID);

      return ssrcInfo;
    }
    
    //-------------------------------------------------------------------------
    void RTPListener::registerSSRCUsage(SSRCInfoPtr ssrcInfo)
    {
      mRegisteredSSRCs[ssrcInfo->mSSRC] = ssrcInfo;
    }

    //-------------------------------------------------------------------------
    void RTPListener::reattemptDelivery()
    {
      if (mReattemptRTPDelivery) return;
      mReattemptRTPDelivery = true;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void RTPListener::processUnhandled(
                                       const String &muxID,
                                       const String &rid,
                                       IRTPTypes::SSRCType ssrc,
                                       IRTPTypes::PayloadType payloadType,
                                       const Time &tick
                                       )
    {
      UnhandledEventInfo unhandled;
      unhandled.mSSRC = ssrc;
      unhandled.mCodecPayloadType = payloadType;
      unhandled.mMuxID = muxID;
      unhandled.mRID = rid;

      auto found = mUnhandledEvents.find(unhandled);
      if (found != mUnhandledEvents.end()) return;

      if (mUnhandledEvents.size() < 1) {
        mUnhanldedEventsTimer = Timer::create(mThisWeak.lock(), mUnhanldedEventsExpires);
      }

      mUnhandledEvents[unhandled] = tick;

      ZS_LOG_TRACE(log("notifying subscribers of unhandled SSRC") + ZS_PARAM("mux id", muxID) + ZS_PARAM("rid", rid) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType) + ZS_PARAM("timer", mUnhanldedEventsTimer ? mUnhanldedEventsTimer->getID() : 0) + ZS_PARAM("tick", tick))

      EventWriteOrtcRtpListenerUnhandledEventFired(__func__, mID, muxID, rid, ssrc, payloadType);

      mSubscriptions.delegate()->onRTPListenerUnhandledRTP(mThisWeak.lock(), unhandled.mSSRC, unhandled.mCodecPayloadType, unhandled.mMuxID.c_str(), unhandled.mRID.c_str());
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
