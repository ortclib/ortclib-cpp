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
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPUtils.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_RTPTypes.h>
#include <ortc/internal/ortc_SRTPSDESTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
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


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpreceiver) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP)

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPReceiverSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

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
      //case IRTPTypes::HeaderExtensionURI_MID:                               return true;
        case IRTPTypes::HeaderExtensionURI_ClienttoMixerAudioLevelIndication: return false;
        case IRTPTypes::HeaderExtensionURI_MixertoClientAudioLevelIndication: return false;
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
    #pragma mark RTPReceiverSettingsDefaults
    #pragma mark

    class RTPReceiverSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPReceiverSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPReceiverSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPReceiverSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPReceiverSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPReceiverSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_SSRC_TIMEOUT_IN_SECONDS, 60);

        ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_MAX_RTP_PACKETS_IN_BUFFER, 100);
        ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_MAX_AGE_RTP_PACKETS_IN_SECONDS, 30);

        ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_CSRC_EXPIRY_TIME_IN_SECONDS, 10);

        ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_ONLY_RESOLVE_AMBIGUOUS_PAYLOAD_MAPPING_IF_ACTIVITY_DIFFERS_IN_MILLISECONDS, 5 * 1000);

        ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_LOCK_TO_RECEIVER_CHANNEL_AFTER_SWITCH_EXCLUSIVELY_FOR_IN_MILLISECONDS, 3 * 1000);
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPReceiverSettingsDefaults()
    {
      RTPReceiverSettingsDefaults::singleton();
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
    #pragma mark IRTPReceiverForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::RegisteredHeaderExtension
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::CodecTypes
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiver::toString(CodecTypes type)
    {
      switch (type) {
        case CodecType_Normal:    return "normal";
        case CodecType_RED:       return "red";
        case CodecType_ULPFEC:    return "ulpfec";
        case CodecType_FlexFEC:   return "flexfec";
        case CodecType_RTX:       return "rtx";
      }
      return "unknown";
    }

    //---------------------------------------------------------------------------
    RTPReceiver::CodecTypes RTPReceiver::toCodecType(const char *type)
    {
      String str(type);

      for (CodecTypes index = CodecType_First; index <= CodecType_Last; index = static_cast<CodecTypes>(static_cast<std::underlying_type<CodecTypes>::type>(index) + 1)) {
        if (str == toString(index)) return index;
      }

      ORTC_THROW_INVALID_PARAMETERS(type)
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::ChannelHolder
    #pragma mark

    //-------------------------------------------------------------------------
    RTPReceiver::ChannelHolder::ChannelHolder()
    {
    }

    //-------------------------------------------------------------------------
    RTPReceiver::ChannelHolder::~ChannelHolder()
    {
      notify(ISecureTransport::State_Closed);

      auto outer = mHolder.lock();
      if (outer) {
        outer->notifyChannelGone();
      }
    }

    //-------------------------------------------------------------------------
    PUID RTPReceiver::ChannelHolder::getID() const
    {
      return mChannel->getID();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelHolder::notify(ISecureTransport::States state)
    {
      if (state == mLastReportedState) return;

      mLastReportedState = state;
      mChannel->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelHolder::notify(RTPPacketPtr packet)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return;
      mChannel->notifyPacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelHolder::notify(RTCPPacketListPtr packets)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return;
      mChannel->notifyPackets(packets);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelHolder::update(const Parameters &params)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return;
      mChannel->notifyUpdate(params);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::ChannelHolder::handle(RTPPacketPtr packet)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return false;
      return mChannel->handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::ChannelHolder::handle(RTCPPacketPtr packet)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return false;
      return mChannel->handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelHolder::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      return mChannel->requestStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::ChannelHolder::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::ChannelHolder");

      auto outer = mHolder.lock();
      UseServicesHelper::debugAppend(resultEl, "outer", outer ? outer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "channel", mChannel ? mChannel->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "channel info", mChannelInfo ? mChannelInfo->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "last reported state", ISecureTransport::toString(mLastReportedState));
      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::ChannelInfo
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiver::ChannelInfo::shouldLatchAll() const
    {
      if (mOriginalParameters->mEncodings.size() < 1) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    String RTPReceiver::ChannelInfo::rid() const
    {
      if (shouldLatchAll()) return String();
      return mFilledParameters->mEncodings.front().mEncodingID;
    }

    //-------------------------------------------------------------------------
    RTPReceiver::SSRCInfoPtr RTPReceiver::ChannelInfo::registerSSRCUsage(SSRCInfoPtr ssrcInfo)
    {
      mRegisteredSSRCs[SSRCRoutingPair(ssrcInfo->mSSRC, ssrcInfo->mRoutingPayload)] = ssrcInfo;
      return ssrcInfo;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelInfo::unregisterSSRCUsage(
                                                       SSRCType ssrc,
                                                       RoutingPayloadType routingPayload
                                                       )
    {
      auto found = mRegisteredSSRCs.find(SSRCRoutingPair(ssrc, routingPayload));
      if (found == mRegisteredSSRCs.end()) return;
      mRegisteredSSRCs.erase(found);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelInfo::registerHolder(ChannelHolderPtr channelHolder)
    {
      if (!channelHolder) return;
      mChannelHolder = channelHolder;

      for (auto iter = mRegisteredSSRCs.begin(); iter != mRegisteredSSRCs.end(); ++iter) {
        auto &ssrcInfo = (*iter).second;
        ssrcInfo->mChannelHolder = channelHolder;
      }
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::ChannelInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::ChannelInfo");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "channel params", mOriginalParameters ? mOriginalParameters->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "filled params", mFilledParameters ? mFilledParameters->toDebug() : ElementPtr());
      auto channelHolder = mChannelHolder.lock();
      UseServicesHelper::debugAppend(resultEl, "channel", channelHolder ? channelHolder->getID() : 0);

      if (mRegisteredSSRCs.size() > 0) {
        ElementPtr ssrcsEl = Element::create("ssrcs");
        for (auto iter = mRegisteredSSRCs.begin(); iter != mRegisteredSSRCs.end(); ++iter) {
          auto &ssrcInfo = (*iter).second;
          UseServicesHelper::debugAppend(ssrcsEl, "ssrc", ssrcInfo->mSSRC);
        }
        UseServicesHelper::debugAppend(resultEl, ssrcsEl);
      }

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::RIDInfo
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::RIDInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::RIDInfo");

      UseServicesHelper::debugAppend(resultEl, "rid", mRID);
      UseServicesHelper::debugAppend(resultEl, "channel info", mChannelInfo ? mChannelInfo->toDebug() : ElementPtr());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::SSRCInfo
    #pragma mark

    //---------------------------------------------------------------------------
    RTPReceiver::SSRCInfo::SSRCInfo() :
      mLastUsage(zsLib::now())
    {
    }

    //---------------------------------------------------------------------------
    ElementPtr RTPReceiver::SSRCInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::SSRCInfo");

      UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
      UseServicesHelper::debugAppend(resultEl, "routing payload", mRoutingPayload);
      UseServicesHelper::debugAppend(resultEl, "rid", mRID);
      UseServicesHelper::debugAppend(resultEl, "last usage", mLastUsage);
      UseServicesHelper::debugAppend(resultEl, mChannelHolder ? mChannelHolder->toDebug() : ElementPtr());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver::CodecInfo
    #pragma mark

    //---------------------------------------------------------------------------
    ElementPtr RTPReceiver::CodecInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::CodecInfo");

      UseServicesHelper::debugAppend(resultEl, "payload type", mPayloadType);
      UseServicesHelper::debugAppend(resultEl, "codec type", toString(mCodecType));
      if (mOriginalCodec) {
        UseServicesHelper::debugAppend(resultEl, "original codec payload type", mOriginalCodec->mPayloadType);
      }
      if (mOriginalCodecParams) {
        UseServicesHelper::debugAppend(resultEl, "original codec parameters", mOriginalCodecParams->toDebug());
      }

      return resultEl;
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
                             Kinds kind,
                             IRTPTransportPtr transport,
                             IRTCPTransportPtr rtcpTransport
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mKind(kind),
      mChannels(make_shared<ChannelWeakMap>()),
      mMaxBufferedRTPPackets(SafeInt<decltype(mMaxBufferedRTPPackets)>(ISettings::getUInt(ORTC_SETTING_RTP_RECEIVER_MAX_RTP_PACKETS_IN_BUFFER))),
      mMaxRTPPacketAge(ISettings::getUInt(ORTC_SETTING_RTP_RECEIVER_MAX_AGE_RTP_PACKETS_IN_SECONDS)),
      mLockAfterSwitchTime(ISettings::getUInt(ORTC_SETTING_RTP_RECEIVER_LOCK_TO_RECEIVER_CHANNEL_AFTER_SWITCH_EXCLUSIVELY_FOR_IN_MILLISECONDS)),
      mAmbiguousPayloadMappingMinDifference(ISettings::getUInt(ORTC_SETTING_RTP_RECEIVER_ONLY_RESOLVE_AMBIGUOUS_PAYLOAD_MAPPING_IF_ACTIVITY_DIFFERS_IN_MILLISECONDS)),
      mSSRCTableExpires(Seconds(ISettings::getUInt(ORTC_SETTING_RTP_RECEIVER_SSRC_TIMEOUT_IN_SECONDS))),
      mContributingSourcesExpiry(Seconds(ISettings::getUInt(ORTC_SETTING_RTP_RECEIVER_CSRC_EXPIRY_TIME_IN_SECONDS)))
    {
      ZS_LOG_DETAIL(debug("created"))

      mListener = UseListener::getListener(transport);
      ORTC_THROW_INVALID_PARAMETERS_IF(!mListener)

      UseSecureTransport::getReceivingTransport(transport, rtcpTransport, mReceiveRTPOverTransport, mReceiveRTCPOverTransport, mRTPTransport, mRTCPTransport);
      mSendRTCPOverTransport = mReceiveRTCPOverTransport;

      ZS_LOG_DEBUG(log("creating media stream track") + ZS_PARAM("kind", IMediaStreamTrack::toString(mKind)))
      
      mTrack = UseMediaStreamTrack::create(mKind);

      ZS_LOG_DEBUG(log("created media stream track") + ZS_PARAM("kind", IMediaStreamTrack::toString(mKind)) + ZS_PARAM("track", mTrack ? mTrack->getID() : 0));
      
      ZS_EVENTING_12(
                     x, i, Detail, RtpReceiverCreate, ol, RtpReceiver, Start,
                     puid, id, mID,
                     string, kind, IMediaStreamTrackTypes::toString(kind),
                     puid, listenerId, ((bool)mListener) ? mListener->getID() : 0,
                     puid, rtpTransportId, ((bool)mRTPTransport) ? mRTPTransport->getID() : 0,
                     puid, rtcpTransportId, ((bool)mRTCPTransport) ? mRTCPTransport->getID() : 0,
                     puid, trackId, ((bool)mTrack) ? mTrack->getID() : 0,
                     size_t, maxBufferedRtpPackets, mMaxBufferedRTPPackets,
                     duration, maxRtpPacketAge, mMaxRTPPacketAge.count(),
                     duration, lockAfterSwitchTime, mLockAfterSwitchTime.count(),
                     duration, ambiguousPayloadMappingMinDifference, mAmbiguousPayloadMappingMinDifference.count(),
                     duration, ssrcTableExpires, mSSRCTableExpires.count(),
                     duration, contributingSourcesExpiry, mContributingSourcesExpiry.count()
                     );

    }

    //-------------------------------------------------------------------------
    void RTPReceiver::init()
    {
      AutoRecursiveLock lock(*this);

      if (mSSRCTableExpires < Seconds(1)) {
        mSSRCTableExpires = Seconds(1);
      }

      mSSRCTableTimer = ITimer::create(mThisWeak.lock(), (zsLib::toMilliseconds(mSSRCTableExpires) / 2));

      if (mContributingSourcesExpiry < Seconds(1)) {
        mContributingSourcesExpiry = Seconds(1);
      }
      mContributingSourcesTimer = ITimer::create(mThisWeak.lock(), (zsLib::toMilliseconds(mContributingSourcesExpiry) / 2));

      mRTCPTransportSubscription = mRTCPTransport->subscribe(mThisWeak.lock());

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPReceiver::~RTPReceiver()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RtpReceiverDestroy, ol, RtpReceiver, Start, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(IRTPReceiverPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(ForRTPListenerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(ForRTPReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiver, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(ForMediaStreamTrackPtr object)
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
    IStatsProvider::PromiseWithStatsReportPtr RTPReceiver::getStats(const StatsTypeSet &stats) const
    {
      if (!stats.hasStatType(IStatsReportTypes::StatsType_InboundRTP)) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      ChannelWeakMapPtr channels;

      UseStatsReport::PromiseWithStatsReportList promises;

      {
        AutoRecursiveLock lock(*this);
        channels = mChannels; // obtain pointer to COW list while inside a lock
      }

      bool result = false;
      for (auto iter = channels->begin(); iter != channels->end(); ++iter)
      {
        auto channel = (*iter).second.lock();
        if (!channel) continue;

        auto channelPromise = PromiseWithStatsReport::create(IORTCForInternal::queueORTC());
        channel->requestStats(channelPromise, stats);
        promises.push_back(channelPromise);
      }

      return UseStatsReport::collectReports(promises);
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
                                       Kinds kind,
                                       IRTPTransportPtr transport,
                                       IRTCPTransportPtr rtcpTransport
                                       )
    {
      RTPReceiverPtr pThis(make_shared<RTPReceiver>(make_private {}, IORTCForInternal::queueORTC(), delegate, kind, transport, rtcpTransport));
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

      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RTPReceiver::track() const
    {
      return IMediaStreamTrackPtr(MediaStreamTrack::convert(mTrack));
    }

    //-------------------------------------------------------------------------
    IRTPTransportPtr RTPReceiver::transport() const
    {
      AutoRecursiveLock lock(*this);
      if (!mRTPTransport) return IRTPTransportPtr();

      {
        auto result = DTLSTransport::convert(mRTPTransport);
        if (result) return result;
      }
      {
        auto result = SRTPSDESTransport::convert(mRTPTransport);
        if (result) return result;
      }

      return IRTPTransportPtr();
    }

    //-------------------------------------------------------------------------
    IRTCPTransportPtr RTPReceiver::rtcpTransport() const
    {
      AutoRecursiveLock lock(*this);
      if (!mRTCPTransport) return IRTCPTransportPtr();

      {
        auto result = DTLSTransport::convert(mRTCPTransport);
        if (result) return result;
      }
      {
        auto result = SRTPSDESTransport::convert(mRTCPTransport);
        if (result) {
          auto iceTransport = mRTCPTransport->getICETransport();
          if (iceTransport) return iceTransport;
        }
      }

      return IRTCPTransportPtr();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::setTransport(
                                   IRTPTransportPtr transport,
                                   IRTCPTransportPtr rtcpTransport
                                   )
    {
      typedef std::set<PUID> PUIDSet;

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
        mListener->registerReceiver(mKind, mThisWeak.lock(), *mParameters, &historicalRTCPPackets);

        if (historicalRTCPPackets.size() > 0) {
          RTCPPacketListPtr notifyPackets;

          for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter)
          {
            auto channelHolder = (*iter).second.lock();
            if (shouldCleanChannel((bool)channelHolder)) continue;

            if (!notifyPackets) {
              notifyPackets = make_shared<RTCPPacketList>(historicalRTCPPackets);
            }
            channelHolder->notify(notifyPackets);
          }
        }
      }

      UseSecureTransport::getReceivingTransport(transport, rtcpTransport, mReceiveRTPOverTransport, mReceiveRTCPOverTransport, mRTPTransport, mRTCPTransport);
      mSendRTCPOverTransport = mReceiveRTCPOverTransport;

      if (mRTCPTransportSubscription) {
        mRTCPTransportSubscription->cancel();
        mRTCPTransportSubscription.reset();
      }

      mRTCPTransportSubscription = mRTCPTransport->subscribe(mThisWeak.lock());

      ZS_EVENTING_4(
                    x, i, Detail, RtpReceiverSetTransport, ol, RtpReceiver, SetTransport,
                    puid, id, mID,
                    puid, listenerId, ((bool)mListener) ? mListener->getID() : 0,
                    puid, rtpTransportId, ((bool)mRTPTransport) ? mRTPTransport->getID() : 0,
                    puid, rtcpTransportId, ((bool)mRTCPTransport) ? mRTCPTransport->getID() : 0
                    );

      notifyChannelsOfTransportState();
    }

    //-------------------------------------------------------------------------
    IRTPReceiverTypes::CapabilitiesPtr RTPReceiver::getCapabilities(Optional<Kinds> kind)
    {
      //kRtcpReport - always on
      //kRtcpSr - always on
      //kRtcpRr - always on
      //kRtcpSdes - always on
      //kRtcpBye - always on
      //kRtcpPli - always on, video only
      //kRtcpNack - set on/off by user
      //kRtcpFir - never used
      //kRtcpTmmbr - never used, video only
      //kRtcpTmmbn - intenally controlled, video only
      //kRtcpSrReq - never used
      //kRtcpXrVoipMetric - never used, RTCP XR
      //kRtcpApp - never used
      //kRtcpSli - always on, video only
      //kRtcpRpsi - always on, video only
      //kRtcpRemb - set on/off by user, video only
      //kRtcpTransmissionTimeOffset - never used
      //kRtcpXrReceiverReferenceTime - set on/off by user, video only, RTCP XR
      //kRtcpXrDlrrReportBlock - internally controlled, video only, RTCP XR
      //kRtcpTransportFeedback - never used

      CapabilitiesPtr result(make_shared<Capabilities>());
      String kindStr;
      if (kind.hasValue()) kindStr = IMediaStreamTrackTypes::toString(kind.value());

      for (IRTPTypes::SupportedCodecs index = IRTPTypes::SupportedCodec_First; index <= IRTPTypes::SupportedCodec_Last; index = static_cast<IRTPTypes::SupportedCodecs>(static_cast<std::underlying_type<IRTPTypes::SupportedCodecs>::type>(index) + 1)) {

        CodecCapability codec;

        codec.mName = IRTPTypes::toString(index);

        switch (IRTPTypes::getCodecKind(index)) {

          case IRTPTypes::CodecKind_Unknown:    break;
          case IRTPTypes::CodecKind_Audio:
          case IRTPTypes::CodecKind_AudioSupplemental:
          {
            codec.mNumChannels = 1;
            codec.mKind = IMediaStreamTrack::toString(IMediaStreamTrackTypes::Kind_Audio);
            codec.mMaxPTime = Milliseconds(60);

            // transport-cc
            {
              IRTPTypes::RTCPFeedback feedback;
              feedback.mType = IRTPTypes::toString(KnownFeedbackType_transport_cc);
              feedback.mParameter = IRTPTypes::toString(KnownFeedbackParameter_Unknown);
              ZS_EVENTING_2(
                            x, i, Detail, RtpReceiverReportRtcpFeedback, ol, RtpReceiver, Info,
                            string, feedbackType, feedback.mType,
                            string, feedbackParameter, feedback.mParameter
                            );

              codec.mRTCPFeedback.push_back(feedback);
            }

            break;
          }
          case IRTPTypes::CodecKind_Video:
          {
            codec.mKind = IMediaStreamTrack::toString(IMediaStreamTrackTypes::Kind_Video);

            // generic NACK
            {
              IRTPTypes::RTCPFeedback feedback;
              feedback.mType = IRTPTypes::toString(KnownFeedbackType_NACK);
              feedback.mParameter = IRTPTypes::toString(KnownFeedbackParameter_Unknown);
              ZS_EVENTING_2(
                            x, i, Detail, RtpReceiverReportRtcpFeedback, ol, RtpReceiver, Info,
                            string, feedbackType, feedback.mType,
                            string, feedbackParameter, feedback.mParameter
                            );

              codec.mRTCPFeedback.push_back(feedback);
            }
            // NACK + PLI - always enabled in webrtc engine
            {
              IRTPTypes::RTCPFeedback feedback;
              feedback.mType = IRTPTypes::toString(KnownFeedbackType_NACK);
              feedback.mParameter = IRTPTypes::toString(KnownFeedbackParameter_PLI);
              ZS_EVENTING_2(
                            x, i, Detail, RtpReceiverReportRtcpFeedback, ol, RtpReceiver, Info,
                            string, feedbackType, feedback.mType,
                            string, feedbackParameter, feedback.mParameter
                            );

              codec.mRTCPFeedback.push_back(feedback);
            }
            // CCM + FIR - cannot be set by webrtc API
            {
              IRTPTypes::RTCPFeedback feedback;
              feedback.mType = IRTPTypes::toString(KnownFeedbackType_CCM);
              feedback.mParameter = IRTPTypes::toString(KnownFeedbackParameter_FIR);
              ZS_EVENTING_2(
                            x, i, Detail, RtpReceiverReportRtcpFeedback, ol, RtpReceiver, Info,
                            string, feedbackType, feedback.mType,
                            string, feedbackParameter, feedback.mParameter
                            );

              codec.mRTCPFeedback.push_back(feedback);
            }
            // REMB
            {
              IRTPTypes::RTCPFeedback feedback;
              feedback.mType = IRTPTypes::toString(KnownFeedbackType_REMB);
              feedback.mParameter = IRTPTypes::toString(KnownFeedbackParameter_Unknown);
              ZS_EVENTING_2(
                            x, i, Detail, RtpReceiverReportRtcpFeedback, ol, RtpReceiver, Info,
                            string, feedbackType, feedback.mType,
                            string, feedbackParameter, feedback.mParameter
                            );

              codec.mRTCPFeedback.push_back(feedback);
            }
            // transport-cc
            {
              IRTPTypes::RTCPFeedback feedback;
              feedback.mType = IRTPTypes::toString(KnownFeedbackType_transport_cc);
              feedback.mParameter = IRTPTypes::toString(KnownFeedbackParameter_Unknown);
              ZS_EVENTING_2(
                            x, i, Detail, RtpReceiverReportRtcpFeedback, ol, RtpReceiver, Info,
                            string, feedbackType, feedback.mType,
                            string, feedbackParameter, feedback.mParameter
                            );

              codec.mRTCPFeedback.push_back(feedback);
            }

            codec.mClockRate = 90000;

            if (IRTPTypes::isMRSTCodec(index)) {
              codec.mSVCMultiStreamSupport = true;
            }
            break;
          }

          case IRTPTypes::CodecKind_AV:         break;
          case IRTPTypes::CodecKind_RTX:        {
            codec.mKind = IMediaStreamTrack::toString(IMediaStreamTrackTypes::Kind_Video);
            codec.mClockRate = 90000;
            break;
          }
          case IRTPTypes::CodecKind_FEC:        {
            codec.mKind = IMediaStreamTrack::toString(IMediaStreamTrackTypes::Kind_Video);
            codec.mClockRate = 90000;
            break;
          }
          case IRTPTypes::CodecKind_Data:
          {
            break;
          }
        }

        bool add = true;

        switch (index) {
          case IRTPTypes::SupportedCodec_Unknown: {
            add = false;
            break;
          }
          case IRTPTypes::SupportedCodec_Opus:    {
            codec.mPreferredPayloadType = 120;
            codec.mNumChannels = 2;
            codec.mClockRate = 48000;
            auto params = make_shared<OpusCodecCapabilityParameters>();
            params->mUseInbandFEC = true;
            codec.mParameters = params;
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:    {
            codec.mPreferredPayloadType = 104;
            codec.mClockRate = 32000;
            break;
          }
          case IRTPTypes::SupportedCodec_G722:    {
            codec.mPreferredPayloadType = 107;
            codec.mClockRate = 16000;
            break;
          }
          case IRTPTypes::SupportedCodec_ILBC:    {
            codec.mPreferredPayloadType = 102;
            codec.mClockRate = 16000;
            break;
          }
          case IRTPTypes::SupportedCodec_PCMU:    {
            codec.mPreferredPayloadType = 0;
            codec.mClockRate = 8000;
            break;
          }
          case IRTPTypes::SupportedCodec_PCMA:    {
            codec.mPreferredPayloadType = 8;
            codec.mClockRate = 8000;
            break;
          }

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:     {
            codec.mPreferredPayloadType = 100;
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:     {
            codec.mPreferredPayloadType = 99;
            break;
          }
          case IRTPTypes::SupportedCodec_H264:    {
            codec.mPreferredPayloadType = 98;
            break;
          }

            // RTX
          case IRTPTypes::SupportedCodec_RTX:     {
            codec.mPreferredPayloadType = 117;
            break;
          }

            // FEC
          case IRTPTypes::SupportedCodec_RED:     {
            codec.mPreferredPayloadType = 115;
            break;
          }
          case IRTPTypes::SupportedCodec_ULPFEC:  {
            codec.mPreferredPayloadType = 116;
            break;
          }
          case IRTPTypes::SupportedCodec_FlexFEC: {
            add = false;
            break;
          }

          case IRTPTypes::SupportedCodec_CN:              {
            codec.mClockRate = 32000;
            codec.mPreferredPayloadType = 106;
            break;
          }
            
          case IRTPTypes::SupportedCodec_TelephoneEvent:  {
            codec.mClockRate = 8000;
            codec.mPreferredPayloadType = 126;
            break;
          }
        }

        if (codec.mKind.hasData()) {
            if (codec.mKind != kindStr) {
              add = false;
            }
          }

        switch (index) {
          case IRTPTypes::SupportedCodec_Unknown:         break;
          case IRTPTypes::SupportedCodec_Opus:            break;
          case IRTPTypes::SupportedCodec_Isac:            {
            if (add) {
              ZS_EVENTING_7(
                            x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                            string, name, codec.mName,
                            string, kind, codec.mKind, 
                            ulong, clockRate, codec.mClockRate.value(), 
                            byte, preferredPayloadType, codec.mPreferredPayloadType, 
                            duration, ptime, codec.mPTime.count(), 
                            duration, maxPTime, codec.mMaxPTime.count(), 
                            ulong, numChannels, codec.mNumChannels.value()
                            );

              result->mCodecs.push_back(codec);
            }
            codec.mClockRate = 16000;
            codec.mPreferredPayloadType = 103;
            break;
          }
          case IRTPTypes::SupportedCodec_G722:            break;
          case IRTPTypes::SupportedCodec_ILBC:            {
            if (add) {
              ZS_EVENTING_7(
                            x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                            string, name, codec.mName,
                            string, kind, codec.mKind,
                            ulong, clockRate, codec.mClockRate.value(),
                            byte, preferredPayloadType, codec.mPreferredPayloadType,
                            duration, ptime, codec.mPTime.count(),
                            duration, maxPTime, codec.mMaxPTime.count(),
                            ulong, numChannels, codec.mNumChannels.value()
                            );

              result->mCodecs.push_back(codec);
            }
            codec.mPreferredPayloadType = 101;
            codec.mClockRate = 8000;
            break;
          }
          case IRTPTypes::SupportedCodec_PCMU:            break;
          case IRTPTypes::SupportedCodec_PCMA:            break;

            // video codecs
          case IRTPTypes::SupportedCodec_VP8:             break;
          case IRTPTypes::SupportedCodec_VP9:             break;
          case IRTPTypes::SupportedCodec_H264:            break;

            // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            RTXCodecCapabilityParameters rtxParams;
            rtxParams.mRTXTime = Milliseconds(3000);
            rtxParams.mApt = 100;
            codec.mParameters = RTXCodecCapabilityParameters::create(rtxParams);

            if (add) {
              ZS_EVENTING_7(
                            x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                            string, name, codec.mName,
                            string, kind, codec.mKind,
                            ulong, clockRate, codec.mClockRate.value(),
                            byte, preferredPayloadType, codec.mPreferredPayloadType,
                            duration, ptime, codec.mPTime.count(),
                            duration, maxPTime, codec.mMaxPTime.count(),
                            ulong, numChannels, codec.mNumChannels.value()
                            );

              result->mCodecs.push_back(codec);
            }

            ++codec.mPreferredPayloadType;          // 118
            rtxParams.mApt = 99;
            codec.mParameters = RTXCodecCapabilityParameters::create(rtxParams);
            if (add) {
              ZS_EVENTING_7(
                            x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                            string, name, codec.mName,
                            string, kind, codec.mKind,
                            ulong, clockRate, codec.mClockRate.value(),
                            byte, preferredPayloadType, codec.mPreferredPayloadType,
                            duration, ptime, codec.mPTime.count(),
                            duration, maxPTime, codec.mMaxPTime.count(),
                            ulong, numChannels, codec.mNumChannels.value()
                            );

              result->mCodecs.push_back(codec);
            }

            ++codec.mPreferredPayloadType;          // 119
            rtxParams.mApt = 98;
            codec.mParameters = RTXCodecCapabilityParameters::create(rtxParams);

            break;
          }

            // FEC
          case IRTPTypes::SupportedCodec_RED:             break;
          case IRTPTypes::SupportedCodec_ULPFEC:          break;
          case IRTPTypes::SupportedCodec_FlexFEC:         break;

          case IRTPTypes::SupportedCodec_CN:              {
            if (add) {
              ZS_EVENTING_7(
                            x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                            string, name, codec.mName,
                            string, kind, codec.mKind,
                            ulong, clockRate, codec.mClockRate.value(),
                            byte, preferredPayloadType, codec.mPreferredPayloadType,
                            duration, ptime, codec.mPTime.count(),
                            duration, maxPTime, codec.mMaxPTime.count(),
                            ulong, numChannels, codec.mNumChannels.value()
                            );

              result->mCodecs.push_back(codec);
            }
            codec.mClockRate = 16000;
            codec.mPreferredPayloadType = 105;
            if (add) {
              ZS_EVENTING_7(
                            x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                            string, name, codec.mName,
                            string, kind, codec.mKind,
                            ulong, clockRate, codec.mClockRate.value(),
                            byte, preferredPayloadType, codec.mPreferredPayloadType,
                            duration, ptime, codec.mPTime.count(),
                            duration, maxPTime, codec.mMaxPTime.count(),
                            ulong, numChannels, codec.mNumChannels.value()
                            );

              result->mCodecs.push_back(codec);
            }
            codec.mPreferredPayloadType = 13;
            codec.mClockRate = 8000;
            break;
          }

          case IRTPTypes::SupportedCodec_TelephoneEvent:  break;
        }

        if (add) {
          ZS_EVENTING_7(
                        x, i, Detail, RtpReceiverReportCodec, ol, RtpReceiver, Info,
                        string, name, codec.mName,
                        string, kind, codec.mKind,
                        ulong, clockRate, codec.mClockRate.value(),
                        byte, preferredPayloadType, codec.mPreferredPayloadType,
                        duration, ptime, codec.mPTime.count(),
                        duration, maxPTime, codec.mMaxPTime.count(),
                        ulong, numChannels, codec.mNumChannels.value()
                        );

          result->mCodecs.push_back(codec);
        }
      }

      USHORT preference = 0;

      for (IRTPTypes::HeaderExtensionURIs index = IRTPTypes::HeaderExtensionURI_First; index <= IRTPTypes::HeaderExtensionURI_Last; index = static_cast<IRTPTypes::HeaderExtensionURIs>(static_cast<std::underlying_type<IRTPTypes::HeaderExtensionURIs>::type>(index) + 1), ++preference) {
        IRTPTypes::HeaderExtension ext;

        ext.mPreferredID = preference;
        ext.mURI = IRTPTypes::toString(index);

        bool add = true;

        switch (index) {
          case HeaderExtensionURI_Unknown:                                {
            add = false;
            break;
          }

          case HeaderExtensionURI_MuxID:                                  {
            break;
          }
          case HeaderExtensionURI_ClienttoMixerAudioLevelIndication:
          case HeaderExtensionURI_MixertoClientAudioLevelIndication:      {
            ext.mKind = "audio";
            break;
          }

          case HeaderExtensionURI_FrameMarking:                           {
            ext.mKind = "video";
            break;
          }
          case HeaderExtensionURI_RID:                                    {
            break;
          }

          case HeaderExtensionURI_3gpp_VideoOrientation:
          case HeaderExtensionURI_3gpp_VideoOrientation6:                 {
            ext.mKind = "video";
            break;
          }
          case HeaderExtensionURI_TransmissionTimeOffsets: {
            ext.mKind = "video";
            break;
          }
          case HeaderExtensionURI_AbsoluteSendTime: {
            break;
          }
          case HeaderExtensionURI_TransportSequenceNumber: {
            ext.mKind = "video";
            break;
          }
        }

        if (ext.mKind.hasData()) {
          if (ext.mKind != kindStr) {
            add = false;
          }
        }

        if (add) {
          ZS_EVENTING_4(
                        x, i, Detail, RtpReceiverReportHeaderExtension, ol, RtpReceiver, Info,
                        string, kind, ext.mKind, 
                        string, uri, ext.mURI, 
                        ushort, preferredId, ext.mPreferredID, 
                        bool, preferredEncrypt, ext.mPreferredEncrypt
                        );

          result->mHeaderExtensions.push_back(ext);
        }
      }

      bool addFecMechanisms = true;
      if ((kind.hasValue()) &&
          (IMediaStreamTrackTypes::Kind_Video != kind.value())) {
        addFecMechanisms = false;
      }

      if (addFecMechanisms) {
        result->mFECMechanisms.push_back(String(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC)));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPReceiver::receive(const Parameters &inParameters)
    {
      typedef RTPTypesHelper::ParametersPtrPairList ParametersPtrPairList;

      PromisePtr promise = Promise::create(IORTCForInternal::queueDelegate());

      auto parameters = make_shared<Parameters>(inParameters);

      AutoRecursiveLock lock(*this);

      Optional<IMediaStreamTrack::Kinds> foundKind;

      RTPTypesHelper::validateCodecParameters(inParameters, foundKind);

      ORTC_THROW_INVALID_PARAMETERS_IF(!foundKind.hasValue());

      ORTC_THROW_INVALID_PARAMETERS_IF(foundKind.value() != mKind);

      ZS_EVENTING_2(
                    x, i, Detail, RtpReceiverReceive, ol, RtpReceiver, Info,
                    puid, id, mID,
                    puid, trackId, mTrack->getID()
                    );

      // scope: calculate codec infos from parameters passed
      {
        mCodecInfos.clear();
        for (auto iter = parameters->mCodecs.begin(); iter != parameters->mCodecs.end(); ++iter)
        {
          auto &codec = (*iter);

          CodecInfo info;
          info.mPayloadType = codec.mPayloadType;
          info.mOriginalCodecParams = &codec;

          switch (IRTPTypes::toSupportedCodec(codec.mName))
          {
            case SupportedCodec_Unknown:

            // audio codecs
            case SupportedCodec_Opus:
            case SupportedCodec_Isac:
            case SupportedCodec_G722:
            case SupportedCodec_ILBC:
            case SupportedCodec_PCMU:
            case SupportedCodec_PCMA:

            // video codecs
            case SupportedCodec_VP8:
            case SupportedCodec_VP9:
            case SupportedCodec_H264:

            case SupportedCodec_CN:

            case SupportedCodec_TelephoneEvent:
            {
              break;
            }

            // RTX
            case SupportedCodec_RTX:
            {
              info.mCodecType = CodecType_RTX;
              break;
            }

            // FEC
            case SupportedCodec_RED:
            {
              info.mCodecType = CodecType_RED;
              break;
            }
            case SupportedCodec_ULPFEC:
            {
              info.mCodecType = CodecType_ULPFEC;
              break;
            }
            case SupportedCodec_FlexFEC:
            {
              info.mCodecType = CodecType_FlexFEC;
              break;
            }
          }

          mCodecInfos[info.mPayloadType] = info;
        }

        // make RTX codecs point to original codec being RTX'ed (if found)
        for (auto iter = mCodecInfos.begin(); iter != mCodecInfos.end(); ++iter)
        {
          auto &info = (*iter).second;

          if (CodecType_RTX != info.mCodecType) continue;

          for (auto iter = parameters->mCodecs.begin(); iter != parameters->mCodecs.end(); ++iter)
          {
            auto &codec = (*iter);
            if (codec.mPayloadType != info.mPayloadType) continue;

            RTXCodecParametersPtr rtxParam = RTXCodecParameters::convert(codec.mParameters);
            if (!rtxParam) continue;

            auto found = mCodecInfos.find(rtxParam->mApt);
            if (found == mCodecInfos.end()) continue;

            CodecInfo &aptCodec = (*found).second;

            info.mOriginalCodec = &aptCodec;
          }

          ORTC_THROW_INVALID_PARAMETERS_IF(NULL == info.mOriginalCodec) // must point to an original codec
        }
      }

      // if H264 is first listed video codec, mark track for H264 rendering
      for (auto iter = parameters->mCodecs.begin(); iter != parameters->mCodecs.end(); ++iter)
      {
        auto &codec = (*iter);

        switch (IRTPTypes::toSupportedCodec(codec.mName))
        {
          case SupportedCodec_VP8:
          case SupportedCodec_VP9:
            mTrack->setH264Rendering(false);
            goto end_codec_loop;
          case SupportedCodec_H264:
            mTrack->setH264Rendering(true);
            goto end_codec_loop;
        }
        continue;

      end_codec_loop:
        break;
      }

      if (mParameters) {
        auto hash = parameters->hash();
        auto previousHash = mParameters->hash();
        if (hash == previousHash) {
          ZS_LOG_TRACE(log("receive has not changed (noop)"))
          promise->resolve();
          return promise;
        }

        bool oldShouldLatchAll = shouldLatchAll();
        ParametersPtrList oldGroupedParams = mParametersGroupedIntoChannels;

        mParameters = parameters;

        mParametersGroupedIntoChannels.clear();
        RTPTypesHelper::splitParamsIntoChannels(*parameters, mParametersGroupedIntoChannels);

        ParametersPtrPairList unchangedChannels;
        ParametersPtrList newChannels;
        ParametersPtrPairList updateChannels;
        ParametersPtrList removeChannels;

        RTPTypesHelper::calculateDeltaChangesInChannels(mKind, oldGroupedParams, mParametersGroupedIntoChannels, unchangedChannels, newChannels, updateChannels, removeChannels);

        // scope: remove dead channels
        {
          for (auto iter = removeChannels.begin(); iter != removeChannels.end(); ++iter) {
            auto &params = (*iter);
            auto found = mChannelInfos.find(params);
            ASSERT(found != mChannelInfos.end())

            if (found == mChannelInfos.end()) continue;

            auto &channelInfo = (*found).second;

            removeChannel(*channelInfo);
            mChannelInfos.erase(found);
          }
        }

        // scope: swap out new / old parameters
        {
          for (auto iter = unchangedChannels.begin(); iter != unchangedChannels.end(); ++iter) {
            auto &pairInfo = (*iter);
            auto &oldParams = pairInfo.first;
            auto &newParams = pairInfo.second;
            auto found = mChannelInfos.find(oldParams);
            ASSERT(found != mChannelInfos.end())

            if (found == mChannelInfos.end()) continue;

            auto channelInfo = (*found).second;

            mChannelInfos.erase(found);
            mChannelInfos[newParams] = channelInfo;
          }
        }

        // scope: update existing channels
        {
          for (auto iter = updateChannels.begin(); iter != updateChannels.end(); ++iter) {
            auto &pairInfo = (*iter);
            auto &oldParams = pairInfo.first;
            auto &newParams = pairInfo.second;
            auto found = mChannelInfos.find(oldParams);
            ASSERT(found != mChannelInfos.end())

            if (found == mChannelInfos.end()) continue;

            auto channelInfo = (*found).second;

            mChannelInfos.erase(found);
            mChannelInfos[newParams] = channelInfo;

            updateChannel(channelInfo, newParams);
          }
        }

        // scope: add new channels
        {
          for (auto iter = newChannels.begin(); iter != newChannels.end(); ++iter) {
            auto &params = (*iter);
            addChannel(params);
          }
        }

        if (oldShouldLatchAll) {
          if (shouldLatchAll()) {
            if (removeChannels.size() > 0) {
              ZS_LOG_DEBUG(log("old latch-all is being removed (thus need to flush all auto-latched channels)"))
              flushAllAutoLatchedChannels();
            }
          } else {
            ZS_LOG_DEBUG(log("no longer auto-latching all channels (thus need to flush all auto-latched channels)"))
            flushAllAutoLatchedChannels();
          }
        }
        reattemptDelivery();
      } else {
        mParameters = parameters;

        RTPTypesHelper::splitParamsIntoChannels(*parameters, mParametersGroupedIntoChannels);

        for (auto iter = mParametersGroupedIntoChannels.begin(); iter != mParametersGroupedIntoChannels.end(); ++iter) {
          auto &params = (*iter);
          addChannel(params);
        }
      }

      mListener->registerReceiver(mKind, mThisWeak.lock(), *mParameters);

      registerHeaderExtensions(*mParameters);

      promise->resolve();
      return promise;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::stop()
    {
      ZS_EVENTING_1(x, i, Detail, RtpReceiverStop, ol, RtpReceiver, Stop, puid, id, mID);

      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    IRTPReceiverTypes::ContributingSourceList RTPReceiver::getContributingSources() const
    {
      ContributingSourceList result;

      AutoRecursiveLock lock(*this);

      for (auto iter = mContributingSources.begin(); iter != mContributingSources.end(); ++iter) {
        auto &source = (*iter).second;
        result.push_back(source);
      }

      ZS_LOG_TRACE(log("get contributing sources") + ZS_PARAM("total", result.size()))
      return result;
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
      ZS_EVENTING_5(
                    x, i, Trace, RtpReceivedIncomingPacket, ol, RtpReceiver, Receive, 
                    puid, id, mID,
                    enum, viaTransport, zsLib::to_underlying(viaTransport),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );


      ZS_LOG_TRACE(log("received packet") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + packet->toDebug())

      ChannelHolderPtr channelHolder;

      {
        AutoRecursiveLock lock(*this);

        if (isShutdown()) {
          ZS_LOG_WARNING(Debug, log("ignoring packet (already shutdown)"))
          return false;
        }

        String rid;
        if (findMapping(*packet, channelHolder, rid)) {
          postFindMappingProcessPacket(*packet, channelHolder);
          goto process_rtp;
        }

        if (isShuttingDown()) {
          ZS_LOG_WARNING(Debug, log("ignoring unhandled packet (during shutdown process)"))
          return false;
        }

        expireRTPPackets();

        Time tick = zsLib::now();

        // provide some modest buffering
        mBufferedRTPPackets.push_back(TimeRTPPacketPair(tick, packet));

        String muxID = extractMuxID(*packet);

        processUnhandled(muxID, rid, packet->ssrc(), packet->pt());
        return true;
      }

    process_rtp:
      {
        ZS_LOG_TRACE(log("forwarding RTP packet to channel") + ZS_PARAM("channel id", channelHolder->getID()) + ZS_PARAM("ssrc", packet->ssrc()));
        ZS_EVENTING_6(
                      x, i, Trace, RtpReceiverDeliverIncomingPacketToChannel, ol, RtpReceiver, Deliver,
                      puid, id, mID,
                      puid, channelObjectId, channelHolder->getID(),
                      enum, viaTransport, zsLib::to_underlying(viaTransport),
                      enum, packetType, zsLib::to_underlying(IICETypes::Component_RTP),
                      buffer, packet, packet->buffer()->BytePtr(),
                      size, size, packet->buffer()->SizeInBytes()
                      );

        return channelHolder->handle(packet);
      }

      return false; // return true if packet was handled
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::handlePacket(
                                   IICETypes::Components viaTransport,
                                   RTCPPacketPtr packet
                                   )
    {
      ZS_EVENTING_5(
                    x, i, Trace, RtpReceivedIncomingPacket, ol, RtpReceiver, Receive,
                    puid, id, mID,
                    enum, viaTransport, zsLib::to_underlying(viaTransport),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      ZS_LOG_TRACE(log("received packet") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + packet->toDebug());

      ChannelWeakMapPtr channels;

      {
        AutoRecursiveLock lock(*this);
        channels = mChannels; // obtain pointer to COW list while inside a lock

        processByes(*packet);
        processSenderReports(*packet);
      }

      bool clean = false;
      auto result = false;
      for (auto iter = channels->begin(); iter != channels->end(); ++iter)
      {
        auto channelHolder = (*iter).second.lock();
        if (!channelHolder) {
          clean = true;
          continue;
        }

        ZS_EVENTING_6(
                      x, i, Trace, RtpReceiverDeliverIncomingPacketToChannel, ol, RtpReceiver, Deliver,
                      puid, id, mID,
                      puid, channelObjectId, channelHolder->getID(),
                      enum, viaTransport, zsLib::to_underlying(viaTransport),
                      enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP),
                      buffer, packet, packet->buffer()->BytePtr(),
                      size, size, packet->buffer()->SizeInBytes()
                      );

        auto channelResult = channelHolder->handle(packet);
        result = result || channelResult;
      }

      if (clean) {
        AutoRecursiveLock lock(*this);
        cleanChannels();
      }

      return result;
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
      UseSecureTransportPtr rtcpTransport;

      {
        AutoRecursiveLock lock(*this);

        if (isShutdown()) {
          ZS_LOG_WARNING(Debug, log("cannot send packet while shutdown"))
          return false;
        }

        rtcpTransport = mRTCPTransport;
      }

      if (!rtcpTransport) {
        ZS_LOG_WARNING(Debug, log("no rtcp transport is currently attached (thus discarding sent packet)"))
        return false;
      }

      ZS_LOG_TRACE(log("sending rtcp packet over secure transport") + ZS_PARAM("size", packet->size()));

      ZS_EVENTING_5(
                    x, i, Debug, RtpReceiverSendOutgoingPacket, ol, RtpReceiver, Info,
                    puid, id, mID,
                    enum, sendOverTransport, zsLib::to_underlying(mSendRTCPOverTransport),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP), 
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      return rtcpTransport->sendPacket(mSendRTCPOverTransport, IICETypes::Component_RTCP, packet->ptr(), packet->size());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IRTPReceiverForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => ISecureTransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiver::onSecureTransportStateChanged(
                                                    ISecureTransportPtr transport,
                                                    ISecureTransport::States state
                                                    )
    {
      ZS_EVENTING_2(
                    x, i, Debug, RtpReceiverInternalSecureTransportStateChangedEvent, ol, RtpReceiver, InternalEvent,
                    puid, id, mID,
                    string, state, ISecureTransportTypes::toString(state)
                    );

      ZS_LOG_DEBUG(log("on secure transport state changed") + ZS_PARAM("secure transport", transport->getID()) + ZS_PARAM("state", ISecureTransportTypes::toString(state)));

      AutoRecursiveLock lock(*this);
      notifyChannelsOfTransportState();
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
      ZS_EVENTING_1(x, i, Trace, RtpReceiverInternalWakeEvent, ol, RtpReceiver, InternalEvent, puid, id, mID);

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
    void RTPReceiver::onTimer(ITimerPtr timer)
    {
      ZS_EVENTING_2(x, i, Trace, RtpReceiverInternalTimerEvent, ol, RtpReceiver, InternalEvent, puid, id, mID, puid, timerId, timer->getID());

      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()));

      AutoRecursiveLock lock(*this);

      if (timer == mSSRCTableTimer) {

        auto adjustedTick = zsLib::now() - mSSRCTableExpires;

        // now =  N; then = T; expire = E; adjusted = A;    N-E = A; if A > T then expired
        // now = 10; then = 5; expiry = 3;                 10-3 = 7;    7 > 5 = expired (true)
        // now =  6; then = 5; expiry = 3;                  6-3 = 3;    3 > 5 = not expired (false)

        for (auto iter_doNotUse = mSSRCRoutingPayloadTable.begin(); iter_doNotUse != mSSRCRoutingPayloadTable.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &ssrcInfo = (*current).second;

          const Time &lastReceived = ssrcInfo->mLastUsage;

          if (!(adjustedTick > lastReceived)) continue;

          SSRCType ssrc = (*current).first.first;
          RoutingPayloadType routingPayload = (*current).first.second;

          ZS_EVENTING_7(
                        x, i, Debug, RtpReceiverSsrcTableEntryRemoved, ol, RtpReceiver, Info,
                        puid, id, mID,
                        puid, channelObjectId, ((bool)ssrcInfo->mChannelHolder) ? ssrcInfo->mChannelHolder->getID() : 0,
                        dword, ssrc, ssrc, 
                        dword, routingPayload, routingPayload,
                        duration, lastUsage, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(),
                        string, rid, ssrcInfo->mRID,
                        string, reason, "expired"
                        );

          ZS_LOG_TRACE(log("expiring SSRC to RID mapping") + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("routing payload", routingPayload) + ZS_PARAM("last received", lastReceived) + ZS_PARAM("adjusted tick", adjustedTick))
          mSSRCRoutingPayloadTable.erase(current);
        }
        return;
      }

      if (timer == mContributingSourcesTimer) {
        auto tick = zsLib::now();

        for (auto iter_doNotUse = mContributingSources.begin(); iter_doNotUse != mContributingSources.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &source = (*current).second;

          if (source.mTimestamp + mContributingSourcesExpiry > tick) continue;

          ZS_LOG_TRACE(log("expiring contributing source") + source.toDebug());

          ZS_EVENTING_4(
                        x, i, Trace, RtpReceiverRemoveContributingSource, ol, RtpReceiver, Info,
                        puid, id, mID,
                        dword, csrc, source.mCSRC,
                        byte, audioLevel, source.mAudioLevel,
                        bool, voiceActivityFlag, source.mVoiceActivityFlag.hasValue() ? source.mVoiceActivityFlag.value() : false
                        );

          mContributingSources.erase(current);
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
    #pragma mark RTPReceiver => IRTPReceiverAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => (friend RTPReceiver::ChannelHolder)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiver::notifyChannelGone()
    {
      ZS_EVENTING_1(x, i, Trace, RtpReceiverInternalChannelGoneEvent, ol, RtpReceiver, InternalEvent, puid, id, mID);

      AutoRecursiveLock lock(*this);
      cleanChannels();
    }

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

      UseServicesHelper::debugAppend(resultEl, "kind", IMediaStreamTrack::toString(mKind));
      UseServicesHelper::debugAppend(resultEl, "track", mTrack ? mTrack->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "parameters", mParameters ? mParameters->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "listener", mListener ? mListener->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "rtp transport", mRTPTransport ? mRTPTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "rtcp transport", mRTCPTransport ? mRTCPTransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "receive rtp over transport", IICETypes::toString(mReceiveRTPOverTransport));
      UseServicesHelper::debugAppend(resultEl, "receive rtcp over transport", IICETypes::toString(mReceiveRTCPOverTransport));
      UseServicesHelper::debugAppend(resultEl, "send rtcp over transport", IICETypes::toString(mSendRTCPOverTransport));

      UseServicesHelper::debugAppend(resultEl, "last reported transport state to channels", ISecureTransportTypes::toString(mLastReportedTransportStateToChannels));

      UseServicesHelper::debugAppend(resultEl, "params grouped into channels", mParametersGroupedIntoChannels.size());

      UseServicesHelper::debugAppend(resultEl, "channels", mChannels->size());
      UseServicesHelper::debugAppend(resultEl, "clean channels", mCleanChannels);

      UseServicesHelper::debugAppend(resultEl, "channel infos", mChannelInfos.size());

      UseServicesHelper::debugAppend(resultEl, "ssrc table", mSSRCRoutingPayloadTable.size());
      UseServicesHelper::debugAppend(resultEl, "registered ssrcs", mRegisteredSSRCRoutingPayloads.size());

      UseServicesHelper::debugAppend(resultEl, "rid channel map", mRIDTable.size());

      UseServicesHelper::debugAppend(resultEl, "ssrc table timer", mSSRCTableTimer ? mSSRCTableTimer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "ssrc table expires", mSSRCTableExpires);

      UseServicesHelper::debugAppend(resultEl, "max buffered rtp packets", mMaxBufferedRTPPackets);
      UseServicesHelper::debugAppend(resultEl, "max rtp packet age", mMaxRTPPacketAge);

      UseServicesHelper::debugAppend(resultEl, "buffered rtp packets", mBufferedRTPPackets.size());
      UseServicesHelper::debugAppend(resultEl, "reattempt delivery", mReattemptRTPDelivery);

      UseServicesHelper::debugAppend(resultEl, "contributing sources", mContributingSources.size());
      UseServicesHelper::debugAppend(resultEl, "contributing sources expiry", mContributingSourcesExpiry);
      UseServicesHelper::debugAppend(resultEl, "contributing source timer", mContributingSourcesTimer ? mContributingSourcesTimer->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "current channel", mCurrentChannel ? mCurrentChannel->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "last switched current channel", mLastSwitchedCurrentChannel);
      UseServicesHelper::debugAppend(resultEl, "lock after switch time", mLockAfterSwitchTime);

      UseServicesHelper::debugAppend(resultEl, "ambiguous payload mapping min difference", mAmbiguousPayloadMappingMinDifference);

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

      ZS_EVENTING_1(x, i, Debug, RtpReceiverStep, ol, RtpReceiver, Step, puid, id, mID);

      // ... other steps here ...
      if (!stepAttemptDelivery()) goto not_ready;
      if (!stepCleanChannels()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("receiver is not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::stepAttemptDelivery()
    {
      ZS_EVENTING_1(x, i, Debug, RtpReceiverStep, ol, RtpReceiver, Step, puid, id, mID);

      if (!mReattemptRTPDelivery) {
        ZS_LOG_TRACE(log("no need to reattempt deliver at this time"))
        return true;
      }

      ZS_LOG_DEBUG(log("will attempt to deliver any buffered RTP packets"))

      mReattemptRTPDelivery = false;

      expireRTPPackets();

      size_t beforeSize = 0;

      do
      {
        beforeSize = mBufferedRTPPackets.size();
        for (auto iter_doNotUse = mBufferedRTPPackets.begin(); iter_doNotUse != mBufferedRTPPackets.end();) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          RTPPacketPtr packet = (*current).second;

          ChannelHolderPtr channelHolder;
          String rid;
          if (!findMapping(*packet, channelHolder, rid)) continue;

          postFindMappingProcessPacket(*packet, channelHolder);

          ZS_LOG_TRACE(log("will attempt to deliver buffered RTP packet") + ZS_PARAM("channel", channelHolder->getID()) + ZS_PARAM("ssrc", packet->ssrc()))
          channelHolder->notify(packet);

          mBufferedRTPPackets.erase(current);
        }

      // NOTE: need to repetitively attempt to deliver packets as it's possible
      //       processinging some packets will then allow delivery of other
      //       packets
      } while ((beforeSize != mBufferedRTPPackets.size()) &&
               (0 != mBufferedRTPPackets.size()));

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::stepCleanChannels()
    {
      ZS_EVENTING_1(x, i, Debug, RtpReceiverStep, ol, RtpReceiver, Step, puid, id, mID);

      if (!mCleanChannels) {
        ZS_LOG_TRACE(log("no need to reattempt clean channels at this time"));
        return true;
      }

      ZS_LOG_DEBUG(log("will attempt to clean channels"));

      ChannelWeakMapPtr replacement(make_shared<ChannelWeakMap>(*mChannels));

      for (auto iter_doNotUse = replacement->begin(); iter_doNotUse != replacement->end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto channelHolder = (*current).second.lock();
        if (channelHolder) continue;

        replacement->erase(current);
      }

      mChannels = replacement;
      mCleanChannels = false;

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, RtpReceiverCancel, ol, RtpReceiver, Cancel, puid, id, mID);

      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

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

      resetActiveReceiverChannel();

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
        auto channelHolder = (*iter).second.lock();
        if (!channelHolder) continue;

        ZS_EVENTING_2(x, i, Debug, RtpReceiverCloseChannel, ol, RtpReceiver, Close, puid, id, mID, puid, channelObjectId, channelHolder->getID());

        channelHolder->notify(ISecureTransport::State_Closed);
      }

      ChannelWeakMapPtr channels = ChannelWeakMapPtr(make_shared<ChannelWeakMap>());
      mChannels = channels;

      if (mParameters) {
        mListener->unregisterReceiver(*this);
      }

      mRegisteredExtensions.clear();

      mChannelInfos.clear();
      mSSRCRoutingPayloadTable.clear();
      mRegisteredSSRCRoutingPayloads.clear();
      mRIDTable.clear();

      if (mSSRCTableTimer) {
        mSSRCTableTimer->cancel();
        mSSRCTableTimer.reset();
      }

      mBufferedRTPPackets.clear();

      mContributingSources.clear();
      if (mContributingSourcesTimer) {
        mContributingSourcesTimer->cancel();
        mContributingSourcesTimer.reset();
      }

      mRTPTransport.reset();
      mRTCPTransport.reset();

      if (mRTCPTransportSubscription) {
        mRTCPTransportSubscription->cancel();
        mRTCPTransportSubscription.reset();
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
      ZS_EVENTING_2(x, i, Debug, RtpReceiverStateChangedEvent, ol, RtpReceiver, StateEvent, puid, id, mID, string, state, toString(state));

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
      ZS_EVENTING_3(
                    x, e, Debug, RtpReceiverErrorEvent, ol, RtpReceiver, ErrorEvent,
                    puid, id, mID,
                    word, errorCode, errorCode,
                    string, reason, reason
                    );

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::shouldLatchAll()
    {
      if (1 != mChannelInfos.size()) return false;
      return (mChannelInfos.begin())->second->shouldLatchAll();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::notifyChannelsOfTransportState()
    {
      ISecureTransport::States currentState = ISecureTransport::State_Pending;

      if (mRTCPTransport) {
        currentState = mRTCPTransport->state();
        if (ISecureTransport::State_Closed == currentState) currentState = ISecureTransport::State_Disconnected;
      } else {
        currentState = ISecureTransport::State_Disconnected;
      }

      if (currentState == mLastReportedTransportStateToChannels) {
        ZS_LOG_TRACE(log("no change in secure transport state to notify") + ZS_PARAM("state", ISecureTransportTypes::toString(currentState)));
        return;
      }

      ZS_LOG_TRACE(log("notify secure transport state change") + ZS_PARAM("new state", ISecureTransportTypes::toString(currentState)) + ZS_PARAM("old state", ISecureTransportTypes::toString(mLastReportedTransportStateToChannels)));

      mLastReportedTransportStateToChannels = currentState;

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
        auto channelHolder = (*iter).second.lock();

        if (shouldCleanChannel((bool)channelHolder)) continue;

        channelHolder->notify(mLastReportedTransportStateToChannels);
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::flushAllAutoLatchedChannels()
    {
      ZS_LOG_TRACE(log("flushing all auto-latched channels") + ZS_PARAM("channels", mChannels->size()))

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter)
      {
        auto channel = (*iter).second.lock();

        ZS_EVENTING_2(x, i, Debug, RtpReceiverCloseChannel, ol, RtpReceiver, Close, puid, id, mID, puid, channelObjectId, channel->getID());

        channel->notify(ISecureTransport::State_Closed);
      }

      resetActiveReceiverChannel();

      mChannels = make_shared<ChannelWeakMap>();  // all channels are now gone (COW with empty replacement list)
      mSSRCRoutingPayloadTable.clear();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::addChannel(ParametersPtr params)
    {
      ChannelInfoPtr channelInfo(make_shared<ChannelInfo>());
      channelInfo->mOriginalParameters = params;
      channelInfo->mFilledParameters = make_shared<Parameters>(*params);  // make a filled duplicate

      ZS_EVENTING_2(x, i, Debug, RtpReceiverAddChannel, ol, RtpReceiver, Info, puid, id, mID, puid, channelObjectId, channelInfo->mID);

      // don't create the channel until its actually needed

      mChannelInfos[params] = channelInfo;

      if (channelInfo->shouldLatchAll()) {
        ZS_LOG_TRACE(log("auto latching channel added") + channelInfo->toDebug())
        return;
      }

      auto &encodingParams = channelInfo->mOriginalParameters->mEncodings.front();

      setRIDUsage(encodingParams.mEncodingID, channelInfo);

      ChannelHolderPtr channelHolder;
      registerUsage(encodingParams, channelInfo, channelHolder);

      channelInfo->registerHolder(channelHolder);

      ZS_LOG_DEBUG(log("added channel") + channelInfo->toDebug())
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::updateChannel(
                                    ChannelInfoPtr channelInfo,
                                    ParametersPtr newParams
                                    )
    {
      ZS_EVENTING_2(x, i, Debug, RtpReceiverUpdateChannel, ol, RtpReceiver, Info, puid, id, mID, puid, channelObjectId, channelInfo->mID);

      bool wasLatchAll = channelInfo->shouldLatchAll();

      ParametersPtr oldOriginalParams = channelInfo->mOriginalParameters;
      ParametersPtr oldFilledParams = channelInfo->mFilledParameters;
      SSRCRoutingMap oldRegisteredSSRCs(channelInfo->mRegisteredSSRCs);

      channelInfo->mOriginalParameters = newParams;
      channelInfo->mFilledParameters = make_shared<Parameters>(*newParams);
      channelInfo->mRegisteredSSRCs.clear();

      if (wasLatchAll) {
        ZS_LOG_DEBUG(log("nothing to copy from old channel (thus skipping)"))

        if (channelInfo->shouldLatchAll()) {
          ZS_LOG_DEBUG(log("nothing to register (thus skipping)"))
          return;
        }

        auto &encodingParams = channelInfo->mOriginalParameters->mEncodings.front();

        setRIDUsage(encodingParams.mEncodingID, channelInfo);

        ChannelHolderPtr channelHolder = channelInfo->mChannelHolder.lock();

        registerUsage(encodingParams, channelInfo, channelHolder);

        channelInfo->registerHolder(channelHolder);
        return;
      }

      auto &baseOldOriginalEncoding = (*(oldOriginalParams->mEncodings.begin()));
      auto &baseOldFilledEncoding = (*(oldFilledParams->mEncodings.begin()));

      if (channelInfo->shouldLatchAll()) {
        ZS_LOG_DEBUG(log("new params now a latch all for all encoding for this channel"))

        if (baseOldOriginalEncoding.mEncodingID.hasData()) {
          auto found = mRIDTable.find(baseOldOriginalEncoding.mEncodingID);
          if (found != mRIDTable.end()) {
            mRIDTable.erase(found);
          }
        }
        return;
      }

      auto &baseNewOriginalEncoding = (*(newParams->mEncodings.begin()));
      auto &baseNewFilledEncoding = (*(channelInfo->mFilledParameters->mEncodings.begin()));

      ChannelHolderPtr channelHolder = channelInfo->mChannelHolder.lock();

      // scope: deregister the changed or removed SSRCs, register the new SSRC
      {
        registerUsage(baseNewOriginalEncoding, channelInfo, channelHolder);

        channelInfo->registerHolder(channelHolder);
      }

      // scope: re-fill previously filled SSRCs with old values
      {
        if ((!baseOldOriginalEncoding.mSSRC.hasValue()) &&
            (baseOldFilledEncoding.mSSRC.hasValue())) {
          if (!baseNewFilledEncoding.mSSRC.hasValue()) {
            baseNewFilledEncoding.mSSRC = baseOldFilledEncoding.mSSRC;
          }
        }

        if (((baseOldOriginalEncoding.mRTX.hasValue()) &&
              (!baseOldOriginalEncoding.mRTX.value().mSSRC.hasValue())) &&
             (baseOldFilledEncoding.mRTX.value().mSSRC.hasValue())) {
          if ((baseNewFilledEncoding.mRTX.hasValue()) &&
              (!baseNewFilledEncoding.mRTX.value().mSSRC.hasValue())) {
            baseNewFilledEncoding.mRTX.value().mSSRC = baseOldFilledEncoding.mRTX.value().mSSRC;
          }
        }

        if (((baseOldOriginalEncoding.mFEC.hasValue()) &&
             (!baseOldOriginalEncoding.mFEC.value().mSSRC.hasValue())) &&
            (baseOldFilledEncoding.mFEC.value().mSSRC.hasValue())) {
          if ((baseNewFilledEncoding.mFEC.hasValue()) &&
              (!baseNewFilledEncoding.mFEC.value().mSSRC.hasValue())) {
            baseNewFilledEncoding.mFEC.value().mSSRC = baseOldFilledEncoding.mFEC.value().mSSRC;
          }
        }
      }

      // scope: copy rid from previous filled value
      {
        if ((!baseOldOriginalEncoding.mEncodingID.hasData()) &&
            (baseOldFilledEncoding.mEncodingID.hasData())) {
          if (!baseNewFilledEncoding.mEncodingID.hasData()) {
            baseNewFilledEncoding.mEncodingID = baseOldFilledEncoding.mEncodingID;
          }
        }
      }

      if (channelHolder) {
        channelHolder->update(*(channelInfo->mOriginalParameters));
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::removeChannel(const ChannelInfo &channelInfo)
    {
      ZS_EVENTING_2(x, i, Debug, RtpReceiverRemoveChannel, ol, RtpReceiver, Info, puid, id, mID, puid, channelObjectId, channelInfo.mID);

      // scope: clean out any SSRCs pointing to this channel
      {
        for (auto iter_doNotUse = mSSRCRoutingPayloadTable.begin(); iter_doNotUse != mSSRCRoutingPayloadTable.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &ssrcInfo = (*current).second;
          auto &channelHolder = ssrcInfo->mChannelHolder;

          if (!channelHolder) continue;

          auto &existingChannelInfo = channelHolder->mChannelInfo;

          if (!existingChannelInfo) continue;
          if (existingChannelInfo->mID != channelInfo.mID) continue;

          ZS_EVENTING_7(
                        x, i, Debug, RtpReceiverSsrcTableEntryRemoved, ol, RtpReceiver, Info,
                        puid, id, mID,
                        puid, channelObjectId, ((bool)ssrcInfo->mChannelHolder) ? ssrcInfo->mChannelHolder->getID() : 0,
                        dword, ssrc, ssrcInfo->mSSRC,
                        dword, routingPayload, ssrcInfo->mRoutingPayload,
                        duration, lastUsage, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(),
                        string, rid, ssrcInfo->mRID,
                        string, reason, "channel removed"
                        );

          mSSRCRoutingPayloadTable.erase(current);
        }
      }

      // scope: clean out any registered SSRCs pointing to this channel
      {
        for (auto iter_doNotUse = mRegisteredSSRCRoutingPayloads.begin(); iter_doNotUse != mRegisteredSSRCRoutingPayloads.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto ssrcInfo = (*current).second.lock();
          if (!ssrcInfo) {
            mRegisteredSSRCRoutingPayloads.erase(current);
            continue;
          }

          auto &channelHolder = ssrcInfo->mChannelHolder;

          if (!channelHolder) continue;

          auto &existingChannelInfo = channelHolder->mChannelInfo;

          if (!existingChannelInfo) continue;
          if (existingChannelInfo->mID != channelInfo.mID) continue;

          mRegisteredSSRCRoutingPayloads.erase(current);
        }
      }

      // scope: clean out any RIDs pointing to this channel
      {
        for (auto iter_doNotUse = mRIDTable.begin(); iter_doNotUse != mRIDTable.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &ridInfo = (*current).second;

          if (!ridInfo.mChannelInfo) continue;
          if (ridInfo.mChannelInfo->mID != channelInfo.mID) continue;

          mRIDTable.erase(current);
        }
      }

      if (mCurrentChannel) {
        if (mCurrentChannel->mChannelInfo->mID == channelInfo.mID) {
          resetActiveReceiverChannel();
        }
      }

      ChannelWeakMapPtr replacementChannels(make_shared<ChannelWeakMap>(*mChannels));

      for (auto iter_doNotUse = replacementChannels->begin(); iter_doNotUse != replacementChannels->end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto channelHolder = (*current).second.lock();
        if (!channelHolder) {
          replacementChannels->erase(current);
          continue;
        }

        if (channelHolder->mChannelInfo->mID != channelInfo.mID) continue;

        // shutdown the channel
        ZS_EVENTING_2(x, i, Debug, RtpReceiverCloseChannel, ol, RtpReceiver, Close, puid, id, mID, puid, channelObjectId, channelHolder->getID());

        channelHolder->notify(ISecureTransport::State_Closed);

        replacementChannels->erase(current);
      }

      mChannels = replacementChannels;

      // already cleaned out channels so don't do again
      mCleanChannels = false;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::registerHeaderExtensions(const Parameters &params)
    {
      mRegisteredExtensions.clear();

      for (auto iter = mParameters->mHeaderExtensions.begin(); iter != mParameters->mHeaderExtensions.end(); ++iter) {
        auto &ext = (*iter);

        auto uri = IRTPTypes::toHeaderExtensionURI(ext.mURI);
        if (shouldFilter(uri)) {
          ZS_LOG_TRACE(log("header extension is not important to receiver (thus filtering)") + ext.toDebug())
          continue;
        }

        RegisteredHeaderExtension newExt;

        newExt.mLocalID = ext.mID;
        newExt.mEncrypted = ext.mEncrypt;
        newExt.mHeaderExtensionURI = IRTPTypes::toHeaderExtensionURI(ext.mURI);

        ZS_EVENTING_4(
                      x, i, Debug, RtpReceiverRegisterHeaderExtension, ol, RtpReceiver, Info,
                      puid, id, mID,
                      string, uri, ext.mURI,
                      ushort, extendionId, ext.mID,
                      bool, encrypt, ext.mEncrypt
                      );

        mRegisteredExtensions[newExt.mLocalID] = newExt;
      }
    }

    //-------------------------------------------------------------------------
    RTPReceiver::SSRCInfoPtr RTPReceiver::setSSRCUsage(
                                                       SSRCType ssrc,
                                                       RoutingPayloadType routingPayload,
                                                       String &ioRID,
                                                       ChannelHolderPtr &ioChannelHolder
                                                       )
    {
      SSRCInfoPtr ssrcInfo;

      SSRCRoutingPair routingPair(ssrc, routingPayload);

      auto found = mSSRCRoutingPayloadTable.find(routingPair);

      if (found == mSSRCRoutingPayloadTable.end()) {
        auto foundWeak = mRegisteredSSRCRoutingPayloads.find(routingPair);
        if (foundWeak != mRegisteredSSRCRoutingPayloads.end()) {
          ssrcInfo = (*foundWeak).second.lock();
          if (!ssrcInfo) {
            mRegisteredSSRCRoutingPayloads.erase(foundWeak);
          }
        }
      } else {
        ssrcInfo = (*found).second;
      }

      if (!ssrcInfo) {
        ssrcInfo = make_shared<SSRCInfo>();
        ssrcInfo->mSSRC = ssrc;
        ssrcInfo->mRoutingPayload = routingPayload;
        ssrcInfo->mChannelHolder = ioChannelHolder;

        if (ioRID.hasData()) {
          ssrcInfo->mRID = ioRID;
        } else {
          if (ioChannelHolder) {
            ioRID = ssrcInfo->mRID = ioChannelHolder->mChannelInfo->rid();
          }
        }

        ZS_EVENTING_6(
                      x, i, Debug, RtpReceiverSsrcTableEntryAdded, ol, RtpReceiver, Info,
                      puid, id, mID,
                      puid, channelObjectId, ((bool)ssrcInfo->mChannelHolder) ? ssrcInfo->mChannelHolder->getID() : 0,
                      dword, ssrc, ssrcInfo->mSSRC,
                      dword, routingPayload, ssrcInfo->mRoutingPayload,
                      duration, lastUsageInSeconds, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(),
                      string, rid, ssrcInfo->mRID
                      );

        mSSRCRoutingPayloadTable[routingPair] = ssrcInfo;
        reattemptDelivery();
        return ssrcInfo;
      }

      ssrcInfo->mLastUsage = zsLib::now();

      if (ioChannelHolder) {
        ssrcInfo->mChannelHolder = ioChannelHolder;
      } else {
        ioChannelHolder = ssrcInfo->mChannelHolder;
      }

      if (ioRID.hasData()) {
        ssrcInfo->mRID = ioRID;
      } else {
        if (ssrcInfo->mRID.isEmpty()) {
          if (ioChannelHolder) {
            ioRID = ssrcInfo->mRID = ioChannelHolder->mChannelInfo->rid();
          }
        } else {
          ioRID = ssrcInfo->mRID;
        }
      }

      ZS_EVENTING_6(
                    x, i, Debug, RtpReceiverSsrcTableEntryUpdated, ol, RtpReceiver, Info,
                    puid, id, mID,
                    puid, channelObjectId, ((bool)ssrcInfo->mChannelHolder) ? ssrcInfo->mChannelHolder->getID() : 0,
                    dword, ssrc, ssrcInfo->mSSRC,
                    dword, routingPayload, ssrcInfo->mRoutingPayload,
                    duration, lastUsageInSeconds, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(),
                    string, rid, ssrcInfo->mRID
                    );
      return ssrcInfo;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::setRIDUsage(
                                  const String &rid,
                                  ChannelInfoPtr &ioChannelInfo
                                  )
    {
      if (rid.isEmpty()) return;

      auto found = mRIDTable.find(rid);
      if (found == mRIDTable.end()) {

        // don't add if there's nothing useful to associate with it
        if (!ioChannelInfo) return;

        RIDInfo ridInfo;
        ridInfo.mRID = rid;
        ridInfo.mChannelInfo = ioChannelInfo;

        mRIDTable[rid] = ridInfo;
        return;
      }

      RIDInfo &ridInfo = (*found).second;

      if (ioChannelInfo) {
        ridInfo.mChannelInfo = ioChannelInfo;
      } else {
        ioChannelInfo = ridInfo.mChannelInfo;
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::registerSSRCUsage(SSRCInfoPtr ssrcInfo)
    {
      mSSRCRoutingPayloadTable[SSRCRoutingPair(ssrcInfo->mSSRC, ssrcInfo->mRoutingPayload)] = ssrcInfo;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::registerUsage(
                                    const IRTPTypes::EncodingParameters &encodingParams,
                                    ChannelInfoPtr channelInfo,
                                    ChannelHolderPtr &ioChannelHolder
                                    )
    {
      if (!encodingParams.mCodecPayloadType.hasValue()) return;

      String rid = encodingParams.mEncodingID;

      bool hasFEC = encodingParams.mFEC.hasValue();
      bool hasRTX = encodingParams.mRTX.hasValue();
      IRTPTypes::KnownFECMechanisms fecMechanism = IRTPTypes::KnownFECMechanism_Unknown;

      PayloadType basePayloadType = encodingParams.mCodecPayloadType.value();

      Optional<SSRCType> baseSSRC = encodingParams.mSSRC;
      Optional<SSRCType> fecSSRC;
      Optional<SSRCType> rtxSSRC;
      if (encodingParams.mFEC.hasValue()) {
        fecMechanism = IRTPTypes::toKnownFECMechanism(encodingParams.mFEC.value().mMechanism);
        if (encodingParams.mFEC.value().mSSRC.hasValue()) {
          fecSSRC = encodingParams.mFEC.value().mSSRC;
        } else {
          fecSSRC = baseSSRC;
        }
      }
      if (encodingParams.mRTX.hasValue()) {
        if (encodingParams.mRTX.value().mSSRC.hasValue()) {
          rtxSSRC = encodingParams.mRTX.value().mSSRC;
        } else {
          rtxSSRC = baseSSRC;
        }
      }

      if (baseSSRC.hasValue()) {
        if (hasFEC) {
          Optional<RoutingPayloadType> routingPayloadType1;
          Optional<RoutingPayloadType> routingPayloadType2;
          getFECMediaCodecRoutingPayload(basePayloadType, fecMechanism, routingPayloadType1, routingPayloadType2);
          if (routingPayloadType1.hasValue()) {
            registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(fecSSRC.value(), routingPayloadType1.value(), rid, ioChannelHolder)));
          }
          if (routingPayloadType1.hasValue()) {
            registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(fecSSRC.value(), routingPayloadType2.value(), rid, ioChannelHolder)));
          }
        }

        if (hasRTX) {
          if (hasFEC) {
            Optional<RoutingPayloadType> routingPayloadType1;
            Optional<RoutingPayloadType> routingPayloadType2;
            getRtxFECMediaCodecRoutingPayload(basePayloadType, fecMechanism, routingPayloadType1, routingPayloadType2);
            if (routingPayloadType1.hasValue()) {
              registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(rtxSSRC.value(), routingPayloadType1.value(), rid, ioChannelHolder)));
            }
            if (routingPayloadType1.hasValue()) {
              registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(rtxSSRC.value(), routingPayloadType2.value(), rid, ioChannelHolder)));
            }
          }

          Optional<RoutingPayloadType> rtxRoutingPayload = getRtxMediaCodecRoutingPayload(basePayloadType);
          if (rtxRoutingPayload.hasValue()) {
            registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(rtxSSRC.value(), rtxRoutingPayload.value(), rid, ioChannelHolder)));
          }
        }

        registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseSSRC.value(), getMediaCodecRoutingPayload(basePayloadType), rid, ioChannelHolder)));
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::reattemptDelivery()
    {
      if (mReattemptRTPDelivery) return;
      mReattemptRTPDelivery = true;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::expireRTPPackets()
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
    bool RTPReceiver::shouldCleanChannel(bool objectExists)
    {
      if (!objectExists) cleanChannels();
      return !objectExists;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::cleanChannels()
    {
      if (mCleanChannels) return;
      mCleanChannels = true;

      auto pThis = mThisWeak.lock();  // NOTE: possible to be called during destruction
      if (pThis) {
        IWakeDelegateProxy::create(pThis)->onWake();
      }
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMapping(
                                  const RTPPacket &rtpPacket,
                                  ChannelHolderPtr &outChannelHolder,
                                  String &outRID
                                  )
    {
      ChannelInfoPtr channelInfo;

      RoutingPayloadType routingPayload = getRoutingPayloadType(rtpPacket);

      outRID = extractRID(routingPayload, rtpPacket, outChannelHolder);

      ZS_EVENTING_4(
                    x, i, Trace, RtpReceiverFindMapping, ol, RtpReceiver, Info,
                    puid, id, mID,
                    string, rid, outRID,
                    buffer, packet, rtpPacket.buffer()->BytePtr(),
                    size, size, rtpPacket.buffer()->SizeInBytes()
                    );

      {
        if (outChannelHolder) goto fill_rid;

        if (findMappingUsingRID(outRID, routingPayload, rtpPacket, channelInfo, outChannelHolder)) goto fill_rid;

        if (findMappingUsingSSRCInEncodingParams(outRID, routingPayload, rtpPacket, channelInfo, outChannelHolder)) goto fill_rid;

        if (findMappingUsingPayloadType(outRID, routingPayload, rtpPacket, channelInfo, outChannelHolder)) goto fill_rid;

        return false;
      }

    fill_rid:
      {
        if (!outChannelHolder) {
          ASSERT((bool)channelInfo)

          createChannel(rtpPacket.ssrc(), routingPayload, outRID, channelInfo, outChannelHolder);

          outChannelHolder = channelInfo->mChannelHolder.lock();
          ASSERT(outChannelHolder)
        }

        if (channelInfo) {
          if (!fillRIDParameters(outRID, channelInfo)) {
            outChannelHolder = ChannelHolderPtr();
            return false;
          }
        }
      }
      
      return true;
    }

    //-------------------------------------------------------------------------
    String RTPReceiver::extractRID(
                                   RoutingPayloadType routingPayload,
                                   const RTPPacket &rtpPacket,
                                   ChannelHolderPtr &outChannelHolder
                                   )
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

        setSSRCUsage(rtpPacket.ssrc(), routingPayload, ridStr, outChannelHolder);
        return ridStr;
      }

      String result;
      setSSRCUsage(rtpPacket.ssrc(), routingPayload, result, outChannelHolder);

      return result;
    }

    //-------------------------------------------------------------------------
    String RTPReceiver::extractMuxID(const RTPPacket &rtpPacket)
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

        return muxID;
      }

      return String();
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMappingUsingRID(
                                          const String &rid,
                                          RoutingPayloadType routingPayload,
                                          const RTPPacket &rtpPacket,
                                          ChannelInfoPtr &outChannelInfo,
                                          ChannelHolderPtr &outChannelHolder
                                          )
    {
      if (!rid.hasData()) return false;

      auto found = mRIDTable.find(rid);
      if (found == mRIDTable.end()) return false;

      auto &ridInfo = (*found).second;

      outChannelInfo = ridInfo.mChannelInfo;
      outChannelHolder = outChannelInfo->mChannelHolder.lock();

      ZS_LOG_DEBUG(log("creating new SSRC table entry (based on rid mapping to existing receiver)") + ZS_PARAM("rid", rid) + ridInfo.toDebug())

      String inRID = rid;
      setSSRCUsage(rtpPacket.ssrc(), routingPayload, inRID, outChannelHolder);
      if (!outChannelInfo) return false;

      ZS_EVENTING_3(
                    x, i, Trace, RtpReceiverFoundMappingByRid, ol, RtpReceiver, Info,
                    puid, id, mID,
                    puid, channelObjectId, ((bool)outChannelHolder) ? outChannelHolder->getID() : 0,
                    string, rid, rid
                    );
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMappingUsingSSRCInEncodingParams(
                                                           const String &rid,
                                                           RoutingPayloadType routingPayload,
                                                           const RTPPacket &rtpPacket,
                                                           ChannelInfoPtr &outChannelInfo,
                                                           ChannelHolderPtr &outChannelHolder
                                                           )
    {
      for (auto iter = mChannelInfos.begin(); iter != mChannelInfos.end(); ++iter)
      {
        ChannelInfoPtr &channelInfo = (*iter).second;

        // first check to see if this SSRC is inside this channel's
        // encoding parameters if this value was auto-filled in those encoding
        // paramters or set by the application developer.

        {
          bool first = true;
          for (auto iterParm = channelInfo->mFilledParameters->mEncodings.begin(); iterParm != channelInfo->mFilledParameters->mEncodings.end(); ++iterParm, first = false)
          {
            EncodingParameters &encoding = (*iterParm);

            if (encoding.mEncodingID.hasData()) {
              if (rid.hasData()) {
                if (first) {
                  if (encoding.mEncodingID != rid) {
                    // Cannot consider any channel that has an encoding ID that
                    // does not match the rid specified (for the base encoding).
                    continue;
                  }
                  // rid and encoding ID match this consider this encoding
                  // a match
                  goto map_ssrc;
                }
              }
            }

            if (encoding.mSSRC.hasValue()) {
              if (rtpPacket.ssrc() == encoding.mSSRC.value()) goto map_ssrc;
            }

            if ((encoding.mRTX.hasValue()) &&
                (encoding.mRTX.value().mSSRC.hasValue())) {
              if (rtpPacket.ssrc() == encoding.mRTX.value().mSSRC.value()) goto map_ssrc;
            }

            if ((encoding.mFEC.hasValue()) &&
                (encoding.mFEC.value().mSSRC.hasValue())) {
              if (rtpPacket.ssrc() == encoding.mFEC.value().mSSRC.value()) goto map_ssrc;
            }
          }

          // no SSRC match was found
          continue;
        }

      map_ssrc:
        {
          outChannelInfo = channelInfo;

          ZS_LOG_DEBUG(log("creating a new SSRC entry in SSRC table (based on associated SSRC being found)") + outChannelInfo->toDebug())

          // the associated SSRC was found in table thus must route to same receiver
          String inRID = rid;
          setSSRCUsage(rtpPacket.ssrc(), routingPayload, inRID, outChannelHolder);
          ZS_EVENTING_3(
                        x, i, Trace, RtpReceiverFoundMappingBySsrc, ol, RtpReceiver, Info,
                        puid, id, mID,
                        puid, channelObjectId, ((bool)outChannelHolder) ? outChannelHolder->getID() : 0,
                        puid, ssrc, rtpPacket.ssrc()
                        );
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMappingUsingPayloadType(
                                                  const String &rid,
                                                  RoutingPayloadType routingPayload,
                                                  const RTPPacket &rtpPacket,
                                                  ChannelInfoPtr &outChannelInfo,
                                                  ChannelHolderPtr &outChannelHolder
                                                  )
    {
      EncodingParameters *foundEncoding = NULL;

      Time lastMatchUsageTime {};

      Optional<IMediaStreamTrack::Kinds> kind = mKind;
      RTPTypesHelper::DecodedCodecInfo decodedCodec;
      if (!RTPTypesHelper::decodePacketCodecs(kind, rtpPacket, *mParameters, decodedCodec)) {
        ZS_LOG_WARNING(Debug, log("unable to find a codec for packet") + ZS_PARAM("packet ssrc", rtpPacket.ssrc()) + ZS_PARAM("payload type", rtpPacket.pt()) + mParameters->toDebug())
        return false;
      }

      for (auto iter = mChannelInfos.begin(); iter != mChannelInfos.end(); ++iter) {

        auto &channelInfo = (*iter).second;

        EncodingParameters *baseEncoding = NULL;
        auto matchEncoding = RTPTypesHelper::pickEncodingToFill(mKind, rtpPacket, *(channelInfo->mFilledParameters), decodedCodec, baseEncoding);

        if (channelInfo->shouldLatchAll()) {
          // special case where this is a "latch all" for the codec
          if (findBestExistingLatchAllOrCreateNew(decodedCodec, rid, routingPayload, rtpPacket, outChannelInfo, outChannelHolder)) goto insert_ssrc_into_table;
          ZS_LOG_WARNING(Debug, log("unable to find a good latch candidate for packet") + ZS_PARAM("ssrc", rtpPacket.ssrc()))
          return false;
        }

        if (NULL == matchEncoding) continue; // did not find an appropriate encoding
        ASSERT(NULL != baseEncoding)  // has to always have a base

        if (baseEncoding->mEncodingID.hasData()) {
          if (rid.hasData()) {
            if (baseEncoding->mEncodingID != rid) {
              ZS_LOG_TRACE(log("cannot match as encoding ID does not match rid") + baseEncoding->toDebug() + ZS_PARAM("rid", rid))
              continue; // cannot consider packets with mismatched RID / base encoding ID
            }
          }
        }

        {
          switch (decodedCodec.mDepth[0].mCodecKind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_AudioSupplemental:
            case CodecKind_Video:
            case CodecKind_AV:
            case CodecKind_Data:
            case CodecKind_FEC:     break;

            case CodecKind_RTX:     {
              auto ssrc = baseEncoding->mSSRC.value();
              auto baseRoutingPayload = getMediaCodecRoutingPayload(baseEncoding->mCodecPayloadType.value());

              auto foundSSRC = mSSRCRoutingPayloadTable.find(SSRCRoutingPair(ssrc, baseRoutingPayload));
              if (foundSSRC == mSSRCRoutingPayloadTable.end()) {
                ZS_LOG_WARNING(Trace, log("catch not match encoding as master SSRC was not active recently") + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("base routing payload", baseRoutingPayload) + channelInfo->toDebug())
                continue;
              }

              auto &ssrcInfo = (*foundSSRC).second;

              if (outChannelInfo) {
                // look at the latest time the master SSRC was used

                auto tick = zsLib::now();

                auto diffLast = tick - lastMatchUsageTime;
                auto diffCurrent = tick - ssrcInfo->mLastUsage;

                if ((diffLast < mAmbiguousPayloadMappingMinDifference) &&
                    (diffCurrent < mAmbiguousPayloadMappingMinDifference)) {
                  ZS_LOG_WARNING(Debug, log("ambiguity exists to which receiver channel the packet should match because both channels have been recendly active (thus cannot pick any encoding)") + ZS_PARAM("tick", tick) + ZS_PARAM("match time", lastMatchUsageTime) + ZS_PARAM("ambiguity window", mAmbiguousPayloadMappingMinDifference) + ZS_PARAM("diff last", diffLast) + ZS_PARAM("diff current", diffCurrent) + ssrcInfo->toDebug() + ZS_PARAM("previous find", outChannelInfo->toDebug()) + ZS_PARAM("found", channelInfo->toDebug()))
                  return false;
                }

                if (ssrcInfo->mLastUsage < lastMatchUsageTime) {
                  ZS_LOG_WARNING(Trace, log("possible ambiguity in match (but going with previous more recent usage)") + ZS_PARAM("match time", lastMatchUsageTime) + ssrcInfo->toDebug())
                  continue;
                }

                ZS_LOG_WARNING(Trace, log("possible ambiguity in match (going with this as more recent in usage)") + ZS_PARAM("match time", lastMatchUsageTime) + ssrcInfo->toDebug() + ZS_PARAM("using", channelInfo->toDebug()) + ZS_PARAM("previous found", outChannelInfo->toDebug()))

                lastMatchUsageTime = ssrcInfo->mLastUsage;
                outChannelInfo = channelInfo;
                foundEncoding = matchEncoding;
              } else {
                ZS_LOG_TRACE(log("found likely match") + channelInfo->toDebug() + ssrcInfo->toDebug())

                lastMatchUsageTime = ssrcInfo->mLastUsage;
                outChannelInfo = channelInfo;
                foundEncoding = matchEncoding;
              }
              break;
            }
          }

          if (!outChannelInfo) outChannelInfo = channelInfo;
          if (outChannelInfo->mID < channelInfo->mID) continue; // smaller = older (and thus better match)

          // this is a better match
          outChannelInfo = channelInfo;
          foundEncoding = matchEncoding;
        }
      }

      if (!outChannelInfo) return false;

      // scope: fill in SSRC in encoding parameters
      {
        ASSERT(foundEncoding)

        switch (decodedCodec.mDepth[0].mCodecKind) {
          case CodecKind_Unknown:  ASSERT(false) break;
          case CodecKind_Audio:
          case CodecKind_Video:
          case CodecKind_AV:
          case CodecKind_Data:
          {
            foundEncoding->mSSRC = rtpPacket.ssrc();
            foundEncoding->mCodecPayloadType = rtpPacket.pt();
            goto insert_ssrc_into_table;
          }
          case CodecKind_AudioSupplemental:
          {
            goto insert_ssrc_into_table;
          }
          case CodecKind_RTX:
          {
            foundEncoding->mRTX.value().mSSRC = rtpPacket.ssrc();
            goto insert_ssrc_into_table;
          }
          case CodecKind_FEC:
          {
            foundEncoding->mFEC.value().mSSRC = rtpPacket.ssrc();

            if (!foundEncoding->mCodecPayloadType.hasValue()) {
              if (IRTPTypes::CodecKind_AudioSupplemental != decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecKind) {
                foundEncoding->mCodecPayloadType = decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecParameters->mPayloadType;
              }
            }
            goto insert_ssrc_into_table;
          }
        }

        ASSERT(false)
        return false;
      }

  insert_ssrc_into_table:
      {
        ZS_LOG_DEBUG(log("creating a new SSRC entry in SSRC table (based on payload type matching)") + ZS_PARAM("routing payload", routingPayload) + outChannelInfo->toDebug())

        String inRID = rid;
        setSSRCUsage(rtpPacket.ssrc(), routingPayload, inRID, outChannelHolder);

        ZS_EVENTING_3(
                      x, i, Trace, RtpReceiverFoundMappingByPayloadType, ol, RtpReceiver, Info,
                      puid, id, mID,
                      puid, channelObjectId, ((bool)outChannelHolder) ? outChannelHolder->getID() : 0,
                      byte, payloadType, rtpPacket.pt()
                      );
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findBestExistingLatchAllOrCreateNew(
                                                          const RTPTypesHelper::DecodedCodecInfo &decodedCodec,
                                                          const String &rid,
                                                          RoutingPayloadType routingPayload,
                                                          const RTPPacket &rtpPacket,
                                                          ChannelInfoPtr &outChannelInfo,
                                                          ChannelHolderPtr &outChannelHolder
                                                          )
    {
      Time lastMatchUsageTime;
      ChannelInfoPtr foundChannelInfo;
      ChannelHolderPtr foundChannelHolder;

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter)
      {
        auto channelHolder = (*iter).second.lock();

        if (shouldCleanChannel((bool)channelHolder)) continue;

        auto &channelInfo = channelHolder->mChannelInfo;

        ASSERT(channelInfo->mFilledParameters->mEncodings.size() > 0)

        auto &filledEncoding = *(channelInfo->mFilledParameters->mEncodings.begin());

        if (filledEncoding.mEncodingID.hasData()) {
          if (rid.hasData()) {
            if (filledEncoding.mEncodingID != rid) {
              ZS_LOG_TRACE(log("cannot match as encoding ID does not match rid") + filledEncoding.toDebug() + ZS_PARAM("rid", rid))
              continue;
            }

            // this is a match
            outChannelInfo = channelInfo;
            outChannelHolder = channelHolder;
            return true;
          }
        }

        EncodingParameters *baseEncoding = NULL;
        auto matchEncoding = RTPTypesHelper::pickEncodingToFill(mKind, rtpPacket, *(channelInfo->mFilledParameters), decodedCodec, baseEncoding);

        if (NULL == matchEncoding) continue;

        switch (decodedCodec.mDepth[0].mCodecKind) {
          case CodecKind_Unknown:  ASSERT(false) break;
          case CodecKind_Audio:
          case CodecKind_Video:
          case CodecKind_AV:
          case CodecKind_Data:
          {
            ZS_LOG_TRACE(log("found match (thus using encoding slot)") + channelHolder->toDebug())

            filledEncoding.mSSRC = rtpPacket.ssrc();
            filledEncoding.mCodecPayloadType = rtpPacket.pt();

            outChannelInfo = channelInfo;
            outChannelHolder = channelHolder;
            return true;
          }
          case CodecKind_AudioSupplemental:
          {
            if (filledEncoding.mSSRC.hasValue()) {
              if (filledEncoding.mSSRC.value() == rtpPacket.ssrc()) {
                ZS_LOG_TRACE(log("found previous match (for supplemental audio data)") + channelHolder->toDebug())

                outChannelInfo = channelInfo;
                outChannelHolder = channelHolder;
                return true;
              }
            }
            goto found_possible_match;
          }
          case CodecKind_RTX:
          {
            if ((filledEncoding.mRTX.hasValue()) &&
                (filledEncoding.mRTX.value().mSSRC.hasValue())) {

              ZS_LOG_TRACE(log("found previous RTX match") + channelHolder->toDebug())

              outChannelInfo = channelInfo;
              outChannelHolder = channelHolder;
              return true;
            }

            goto found_possible_match;
          }
          case CodecKind_FEC:
          {
            if ((filledEncoding.mFEC.hasValue()) &&
                (filledEncoding.mFEC.value().mSSRC.hasValue())) {

              if (filledEncoding.mFEC.value().mSSRC.value() != rtpPacket.ssrc()) {
                ZS_LOG_INSANE(log("cannot match as FEC encoding already has matched main SSRC") + channelInfo->toDebug() + ZS_PARAM("packet ssrc", rtpPacket.ssrc()))
                break;
              }

              ZS_LOG_TRACE(log("found previous FEC match") + channelHolder->toDebug())

              outChannelInfo = channelInfo;
              outChannelHolder = channelHolder;
              return true;
            }

            goto found_possible_match;
          }
        }

        // did not find match
        continue;

      found_possible_match:
        {

          Optional<RoutingPayloadType> routingPayloadType = getRoutingPayload(decodedCodec);

          if (!routingPayloadType) {
            ZS_LOG_WARNING(Trace, log("could not obtain routing payload type") + channelInfo->toDebug())
            continue;
          }

          auto foundSSRC = mSSRCRoutingPayloadTable.find(SSRCRoutingPair(rtpPacket.ssrc(), routingPayloadType.value()));
          if (foundSSRC == mSSRCRoutingPayloadTable.end()) {
            ZS_LOG_WARNING(Trace, log("could not match encoding as master SSRC was not active recently") + channelInfo->toDebug())
            continue;
          }

          auto &ssrcInfo = (*foundSSRC).second;

          if (foundChannelInfo) {
            // look at the latest time the master SSRC was used

            auto tick = zsLib::now();

            auto diffLast = tick - lastMatchUsageTime;
            auto diffCurrent = tick - ssrcInfo->mLastUsage;

            if ((diffLast > mAmbiguousPayloadMappingMinDifference) &&
                (diffCurrent > mAmbiguousPayloadMappingMinDifference)) {
              ZS_LOG_WARNING(Debug, log("ambiguity exists to which receiver channel the packet should match because both channels have been recendly active (thus cannot pick any encoding)") + ZS_PARAM("tick", tick) + ZS_PARAM("match time", lastMatchUsageTime) + ZS_PARAM("ambiguity window", mAmbiguousPayloadMappingMinDifference) + ZS_PARAM("diff last", diffLast) + ZS_PARAM("diff current", diffCurrent) + ssrcInfo->toDebug() + ZS_PARAM("previous find", foundChannelHolder->toDebug()) + ZS_PARAM("found", channelHolder->toDebug()))
              return false;
            }

            if (ssrcInfo->mLastUsage < lastMatchUsageTime) {
              ZS_LOG_WARNING(Trace, log("possible ambiguity in match (but going with previous more recent usage)") + ZS_PARAM("match time", lastMatchUsageTime) + ssrcInfo->toDebug())
              continue;
            }

            ZS_LOG_WARNING(Trace, log("possible ambiguity in match (going with this as more recent in usage)") + ZS_PARAM("match time", lastMatchUsageTime) + ssrcInfo->toDebug() + ZS_PARAM("using", channelInfo->toDebug()) + ZS_PARAM("previous found", foundChannelInfo->toDebug()))

            lastMatchUsageTime = ssrcInfo->mLastUsage;
            foundChannelInfo = channelInfo;
            foundChannelHolder = channelHolder;

          } else {
            ZS_LOG_TRACE(log("found likely match") + channelHolder->toDebug() + ssrcInfo->toDebug())

            lastMatchUsageTime = ssrcInfo->mLastUsage;
            foundChannelInfo = channelInfo;
            foundChannelHolder = channelHolder;
          }
        }

      }

      if (foundChannelInfo) {

        ASSERT(foundChannelInfo->mFilledParameters->mEncodings.size() > 0)

        auto &filledEncoding = *(foundChannelInfo->mFilledParameters->mEncodings.begin());

        switch (decodedCodec.mDepth[0].mCodecKind) {
          case CodecKind_Unknown:  ASSERT(false) break;
          case CodecKind_Audio:
          case CodecKind_Video:
          case CodecKind_AV:
          case CodecKind_Data:
          {
            ASSERT(false);
            break;
          }
          case CodecKind_AudioSupplemental:
          {
            // no SSRC slot to "fill" for supplemental audio data
            break;
          }
          case CodecKind_RTX:
          {
            if (!filledEncoding.mRTX.hasValue()) {
              RTXParameters rtx;
              filledEncoding.mRTX = rtx;
            }

            if (!filledEncoding.mRTX.value().mSSRC.hasValue()) {
              filledEncoding.mRTX.value().mSSRC = rtpPacket.ssrc();
            }

            if (IRTPTypes::CodecKind_FEC == decodedCodec.mDepth[1].mCodecKind) {
              if (!filledEncoding.mFEC.value().mMechanism.isEmpty()) {
                switch (decodedCodec.mDepth[1].mSupportedCodec) {
                  case IRTPTypes::SupportedCodec_RED:     {
                    filledEncoding.mFEC.value().mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
                    break;
                  }
                  case IRTPTypes::SupportedCodec_FlexFEC: {
                    filledEncoding.mFEC.value().mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC);
                    break;
                  }
                  default:
                  {
                    ASSERT(false);
                    break;
                  }
                }
              }
            }

            ZS_LOG_DEBUG(log("filled RTX codec") + filledEncoding.toDebug())
            break;
          }
          case CodecKind_FEC:
          {
            if (!filledEncoding.mFEC.value().mMechanism.isEmpty()) {
              switch (decodedCodec.mDepth[0].mSupportedCodec) {
                case IRTPTypes::SupportedCodec_RED:     {
                  filledEncoding.mFEC.value().mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
                  break;
                }
                case IRTPTypes::SupportedCodec_FlexFEC: {
                  filledEncoding.mFEC.value().mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC);
                  break;
                }
                default:
                {
                  ASSERT(false);
                  break;
                }
              }
            }

            if (!filledEncoding.mFEC.value().mSSRC.hasValue()) {
              filledEncoding.mFEC.value().mSSRC = rtpPacket.ssrc();
            }

            ZS_LOG_DEBUG(log("filled FEC codec") + filledEncoding.toDebug())
            break;
          }
        }

        outChannelInfo = foundChannelInfo;
        outChannelHolder = foundChannelHolder;
        return true;
      }

      EncodingParameters encoding;

      // no match was found at all
      switch (decodedCodec.mDepth[0].mCodecKind) {
        case CodecKind_Unknown:
        {
          ASSERT(false);
          break;
        }
        case CodecKind_Audio:
        case CodecKind_Video:
        case CodecKind_AV:
        case CodecKind_Data:
        {
          encoding.mSSRC = rtpPacket.ssrc();
          encoding.mCodecPayloadType = rtpPacket.pt();

          {
            bool foundRED = false;
            bool foundULPFEC = false;

            bool foundREDULPFEC = false;
            bool foundFlexFEC = false;

            {
              RTPTypesHelper::FindCodecOptions options;
              options.mSupportedCodec = IRTPTypes::SupportedCodec_RED;
              options.mClockRate = decodedCodec.mDepth[0].mCodecParameters->mClockRate;
              options.mMatchClockRateNotSet = true;
              RTPTypesHelper::FindCodecOptions::PayloadTypeSet payloads;
              payloads.insert(rtpPacket.pt());
              options.mREDCodecPayloadTypes = payloads;
              options.mAllowREDMatchEmptyList = true;

              foundRED = (NULL != RTPTypesHelper::findCodec(*mParameters, options));
            }
            {
              RTPTypesHelper::FindCodecOptions options;
              options.mSupportedCodec = IRTPTypes::SupportedCodec_ULPFEC;
              options.mClockRate = decodedCodec.mDepth[0].mCodecParameters->mClockRate;
              options.mMatchClockRateNotSet = true;

              foundULPFEC = (NULL != RTPTypesHelper::findCodec(*mParameters, options));
            }

            foundREDULPFEC = foundRED && foundULPFEC;

            {
              RTPTypesHelper::FindCodecOptions options;
              options.mSupportedCodec = IRTPTypes::SupportedCodec_FlexFEC;
              options.mClockRate = decodedCodec.mDepth[0].mCodecParameters->mClockRate;
              options.mMatchClockRateNotSet = true;
              foundFlexFEC = (NULL != RTPTypesHelper::findCodec(*mParameters, options));
            }

            if (foundREDULPFEC || foundULPFEC)
            {
              FECParameters fec;

              if (foundREDULPFEC) {
                if (!foundFlexFEC) fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
              } else {
                fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC);
              }

              encoding.mFEC = fec;
            }
          }

          goto make_new_channel;
        }
        case CodecKind_AudioSupplemental:
        {
          goto not_appropriate_latch;
        }
        case CodecKind_RTX:
        {
          // do not allow RTX until the base encoding is known
          goto not_appropriate_latch;
        }
        case CodecKind_FEC:
        {
          encoding.mCodecPayloadType = rtpPacket.pt();
          FECParameters fec;
          fec.mSSRC = rtpPacket.ssrc();

          switch (decodedCodec.mDepth[0].mSupportedCodec)
          {
            case IRTPTypes::SupportedCodec_RED:
            {
              fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
              break;
            }
            case IRTPTypes::SupportedCodec_FlexFEC:
            {
              fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC);
              break;
            }
            default:
            {
              ASSERT(false);
              goto not_appropriate_latch;
            }
          }

          switch (decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecKind)
          {
            case IRTPTypes::CodecKind_Audio:
            case IRTPTypes::CodecKind_Video:
            case IRTPTypes::CodecKind_AV:
            case IRTPTypes::CodecKind_Data:
            {
              encoding.mCodecPayloadType = decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecParameters->mPayloadType;
              break;
            }
            case IRTPTypes::CodecKind_Unknown:
            case IRTPTypes::CodecKind_AudioSupplemental:
            case IRTPTypes::CodecKind_RTX:
            case IRTPTypes::CodecKind_FEC:
            {
              goto not_appropriate_latch;
            }
          }

          goto make_new_channel;
        }
      }

    make_new_channel:
      {
        encoding.mEncodingID = rid;
        encoding.mActive = true;

        {
          RTPTypesHelper::FindCodecOptions options;
          options.mSupportedCodec = IRTPTypes::SupportedCodec_RTX;
          options.mClockRate = decodedCodec.mDepth[0].mCodecParameters->mClockRate;
          options.mMatchClockRateNotSet = true;
          options.mRTXAptPayloadType = rtpPacket.pt();
          if (NULL != RTPTypesHelper::findCodec(*mParameters, options)) {
            RTXParameters rtx;
            encoding.mRTX = rtx;
          }
        }

        ChannelInfoPtr channelInfo(make_shared<ChannelInfo>());

        if (mChannelInfos.size() > 0) {
          // force sharing of the same channel ID
          auto &baseOnChannelInfo = (*(mChannelInfos.begin())).second;
          channelInfo->mID.reset(baseOnChannelInfo->mID);
        }

        channelInfo->mOriginalParameters = make_shared<Parameters>(*mParameters);
        channelInfo->mFilledParameters = make_shared<Parameters>(*mParameters);

        channelInfo->mFilledParameters->mEncodings.push_back(encoding);

        outChannelInfo = channelInfo;

        RTCPPacketList historicalPackets;
        mListener->getPackets(historicalPackets);

        ChannelHolderPtr channelHolder(make_shared<ChannelHolder>());
        channelHolder->mHolder = mThisWeak.lock();
        channelHolder->mChannelInfo = channelInfo;
        channelHolder->mChannel = UseChannel::create(mThisWeak.lock(), MediaStreamTrack::convert(mTrack), *(channelInfo->mOriginalParameters), historicalPackets);
        channelHolder->notify(mLastReportedTransportStateToChannels);

        ZS_EVENTING_2(
                      x, i, Debug, RtpReceiverCreateChannel, ol, RtpReceiver, Info,
                      puid, id, mID,
                      puid, channelObjectId, channelHolder->getID()
                      );

        channelInfo->mChannelHolder = channelHolder;

        // remember the channel (mChannels is using COW pattern)
        ChannelWeakMapPtr replacementChannels(make_shared<ChannelWeakMap>(*mChannels));
        (*replacementChannels)[channelHolder->getID()] = channelHolder;
        mChannels = replacementChannels;

        String inRID = rid;
        setSSRCUsage(rtpPacket.ssrc(), routingPayload, inRID, channelHolder);

        outChannelInfo = channelInfo;
        outChannelHolder = channelHolder;
        return true;
      }

    not_appropriate_latch:
      {
      }

      ZS_LOG_WARNING(Debug, log("failed to find an appropriate previously latched encoding to use") + ZS_PARAM("ssrc", rtpPacket.ssrc()) + ZS_PARAM("pt", rtpPacket.pt()))
      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::fillRIDParameters(
                                        const String &rid,
                                        ChannelInfoPtr &ioChannelInfo
                                        )
    {
      ASSERT((bool)ioChannelInfo)

      if (!rid.hasData()) return true;

      if (!ioChannelInfo->shouldLatchAll()) {
        auto &encoding = ioChannelInfo->mFilledParameters->mEncodings.front();

        if (encoding.mEncodingID.hasData()) {
          if (rid != encoding.mEncodingID) {
            // already has a RID and this isn't it!
            ZS_LOG_WARNING(Debug, log("receiver channel encoding id and packet rid are mis-matched") + ZS_PARAM("rid", rid) + ioChannelInfo->toDebug())
            return false;
          }

          setRIDUsage(rid, ioChannelInfo);
          return true;
        }

        encoding.mEncodingID = rid;
      }

      setRIDUsage(rid, ioChannelInfo);
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::createChannel(
                                    SSRCType ssrc,
                                    RoutingPayloadType routingPayload,
                                    const String &rid,
                                    ChannelInfoPtr channelInfo,
                                    ChannelHolderPtr &ioChannelHolder
                                    )
    {
      ASSERT(channelInfo)

      if (ioChannelHolder) return;

      ioChannelHolder = channelInfo->mChannelHolder.lock();
      if (ioChannelHolder) {
        String inRID = rid;
        setSSRCUsage(ssrc, routingPayload, inRID, ioChannelHolder);
        return;
      }

      RTCPPacketList historicalPackets;
      mListener->getPackets(historicalPackets);

      ioChannelHolder = make_shared<ChannelHolder>();

      ioChannelHolder->mHolder = mThisWeak.lock();
      ioChannelHolder->mChannelInfo = channelInfo;
      ioChannelHolder->mChannel = UseChannel::create(mThisWeak.lock(), MediaStreamTrack::convert(mTrack), *(channelInfo->mOriginalParameters), historicalPackets);
      ioChannelHolder->notify(mLastReportedTransportStateToChannels);

      ZS_EVENTING_2(
                    x, i, Debug, RtpReceiverCreateChannel, ol, RtpReceiver, Info,
                    puid, id, mID,
                    puid, channelObjectId, ioChannelHolder->getID()
                    );

      channelInfo->mChannelHolder = ioChannelHolder;

      // remember the channel (mChannels is using COW pattern)
      ChannelWeakMapPtr replacementChannels(make_shared<ChannelWeakMap>(*mChannels));
      (*replacementChannels)[ioChannelHolder->getID()] = ioChannelHolder;
      mChannels = replacementChannels;

      String inRID = rid;
      setSSRCUsage(ssrc, routingPayload, inRID, ioChannelHolder);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::processUnhandled(
                                       const String &muxID,
                                       const String &rid,
                                       IRTPTypes::SSRCType ssrc,
                                       IRTPTypes::PayloadType payloadType
                                       )
    {
      ZS_LOG_TRACE(log("notifying listener of unhandled SSRC") + ZS_PARAM("mux id", muxID) + ZS_PARAM("rid", rid) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType));

      ZS_EVENTING_5(
                    x, i, Debug, RtpReceiverUnhandledEvent, ol, RtpReceiver, Event,
                    puid, id, mID,
                    string, muxId, muxID,
                    string, rid, rid,
                    dword, ssrc, ssrc,
                    byte, payloadType, payloadType
                    );

      mListener->notifyUnhandled(muxID, rid, ssrc, payloadType);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::processByes(const RTCPPacket &rtcpPacket)
    {
      for (auto bye = rtcpPacket.firstBye(); NULL != bye; bye = bye->nextBye()) {
        for (size_t index = 0; index < bye->sc(); ++index) {
          auto byeSSRC = bye->ssrc(index);

          // scope: clean normal SSRC table
          {
            for (auto iter_doNotUse = mSSRCRoutingPayloadTable.begin(); iter_doNotUse != mSSRCRoutingPayloadTable.end(); )
            {
              auto current = iter_doNotUse;
              ++iter_doNotUse;

              auto &ssrc = (*current).first.first;
              if (ssrc != byeSSRC) continue;

              auto ssrcInfo = (*current).second;

              ZS_EVENTING_7(
                            x, i, Debug, RtpReceiverSsrcTableEntryRemoved, ol, RtpReceiver, Info,
                            puid, id, mID,
                            puid, channelObjectId, ((bool)ssrcInfo->mChannelHolder) ? ssrcInfo->mChannelHolder->getID() : 0,
                            dword, ssrc, ssrcInfo->mSSRC,
                            dword, routingPayload, ssrcInfo->mRoutingPayload,
                            duration, lastUsage, zsLib::timeSinceEpoch<Seconds>(ssrcInfo->mLastUsage).count(),
                            string, rid, ssrcInfo->mRID,
                            string, reason, "bye"
                            );

              ZS_LOG_TRACE(log("removing ssrc table entry due to BYE") + ZS_PARAM("ssrc", byeSSRC) + ssrcInfo->toDebug())
              mSSRCRoutingPayloadTable.erase(current);
            }
          }

          // scope: clean out any channels that have this SSRCs
          {
            for (auto iter = mChannelInfos.begin(); iter != mChannelInfos.end(); ++iter) {
              auto &channelInfo = (*iter).second;

              // Check to see if this SSRC is inside this channel's
              // encoding parameters but if this value was auto-filled in
              // those encoding paramters and not set by the application
              // developer and reset those parameters back to the original.
              auto iterFilledParams = channelInfo->mFilledParameters->mEncodings.begin();
              auto iterOriginalParams = channelInfo->mOriginalParameters->mEncodings.begin();

              for (; iterFilledParams != channelInfo->mFilledParameters->mEncodings.end(); ++iterFilledParams, ++iterOriginalParams)
              {
                ASSERT(iterOriginalParams != channelInfo->mOriginalParameters->mEncodings.end())
                EncodingParameters &filledParams = (*iterFilledParams);
                EncodingParameters &originalEncParams = (*iterOriginalParams);

                if ((filledParams.mSSRC.hasValue()) &&
                    (!originalEncParams.mSSRC.hasValue())) {
                  if (byeSSRC == filledParams.mSSRC.value()) {
                    filledParams.mSSRC = originalEncParams.mSSRC;
                  }
                }

                if (((filledParams.mRTX.hasValue()) &&
                     (filledParams.mRTX.value().mSSRC.hasValue())) &&
                    (!  ((originalEncParams.mRTX.hasValue()) &&
                         (originalEncParams.mRTX.value().mSSRC.hasValue()))
                     )) {
                      if (byeSSRC == filledParams.mRTX.value().mSSRC.value()) {
                        filledParams.mRTX.value().mSSRC = originalEncParams.mRTX.value().mSSRC;
                      }
                    }

                if (((filledParams.mFEC.hasValue()) &&
                     (filledParams.mFEC.value().mSSRC.hasValue())) &&
                    (! ((originalEncParams.mFEC.hasValue()) &&
                        (originalEncParams.mFEC.value().mSSRC.hasValue()))
                     )) {
                      if (byeSSRC == filledParams.mFEC.value().mSSRC.value()) {
                        filledParams.mFEC.value().mSSRC = originalEncParams.mFEC.value().mSSRC;
                      }
                    }
              }
            }
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::processSenderReports(const RTCPPacket &rtcpPacket)
    {
      for (auto sr = rtcpPacket.firstSenderReport(); NULL != sr; sr = sr->nextSenderReport()) {
        for (auto iter = mSSRCRoutingPayloadTable.begin(); iter != mSSRCRoutingPayloadTable.end(); ++iter)
        {
          auto &ssrc = (*iter).first.first;
          if (ssrc != sr->ssrcOfSender()) continue;

          auto &ssrcInfo = (*iter).second;

          String ignoredRID;
          ChannelHolderPtr channelHolder;
          setSSRCUsage(ssrc, ssrcInfo->mRoutingPayload, ignoredRID, channelHolder);
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::extractCSRCs(const RTPPacket &rtpPacket)
    {
      for (auto ext = rtpPacket.firstHeaderExtension(); NULL != ext; ext = ext->mNext) {
        LocalID localID = static_cast<LocalID>(ext->mID);
        auto found = mRegisteredExtensions.find(localID);
        if (found == mRegisteredExtensions.end()) continue; // header extension is not understood

        RegisteredHeaderExtension &headerInfo = (*found).second;

        switch (headerInfo.mHeaderExtensionURI) {
          case IRTPTypes::HeaderExtensionURI_ClienttoMixerAudioLevelIndication:   {
            RTPPacket::ClientToMixerExtension levelExt(*ext);
            auto level = levelExt.level();
            Optional<bool> voiceActivity(levelExt.voiceActivity());
            setContributingSource(rtpPacket.ssrc(), level, voiceActivity);
            break;
          }
          case IRTPTypes::HeaderExtensionURI_MixertoClientAudioLevelIndication:   {
            RTPPacket::MixerToClientExtension levelExt(*ext);
            for (size_t index = 0; (index < levelExt.levelsCount()) && (index < rtpPacket.cc()); ++index) {
              auto level = levelExt.level(index);
              Optional<bool> voiceActivity {};
              setContributingSource(rtpPacket.getCSRC(index) , level, voiceActivity);
            }
            break;
          }
          default:
            // ignored
            break;
        }
      }
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiver::setContributingSource(
                                            SSRCType csrc,
                                            BYTE level,
                                            const Optional<bool> &voiceActivityFlag
                                            )
    {
      auto found = mContributingSources.find(csrc);
      if (found == mContributingSources.end()) {
        ContributingSource source;
        source.mCSRC = csrc;
        source.mTimestamp = zsLib::now();
        source.mAudioLevel = level;
        source.mVoiceActivityFlag = voiceActivityFlag;
        mContributingSources[csrc] = source;
        ZS_EVENTING_4(
                      x, i, Trace, RtpReceiverAddContributingSource, ol, RtpReceiver, Info,
                      puid, id, mID,
                      dword, csrc, csrc,
                      byte, level, level,
                      bool, voiceActivityFlag, voiceActivityFlag.hasValue() ? voiceActivityFlag.value() : false
                      );
        return;
      }

      auto &source = (*found).second;

      source.mTimestamp = zsLib::now();
      source.mAudioLevel = level;
      source.mVoiceActivityFlag = voiceActivityFlag;
      ZS_EVENTING_4(
                    x, i, Trace, RtpReceiverUpdateContributingSource, ol, RtpReceiver, Info,
                    puid, id, mID,
                    dword, csrc, csrc,
                    byte, level, level,
                    bool, voiceActivityFlag, voiceActivityFlag.hasValue() ? voiceActivityFlag.value() : false
                    );
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::postFindMappingProcessPacket(
                                                   const RTPPacket &rtpPacket,
                                                   ChannelHolderPtr &channelHolder
                                                   )
    {
      ASSERT(channelHolder)
      ASSERT(channelHolder->mChannelInfo->mFilledParameters->mEncodings.size() > 0)

      auto &encoding = *(channelHolder->mChannelInfo->mFilledParameters->mEncodings.begin());

      if (!encoding.mActive) {
        ZS_LOG_WARNING(Trace, log("encoding is not active thus do not process information from this channel"))
        return;
      }

      extractCSRCs(rtpPacket);

      if (channelHolder == mCurrentChannel) return;

      Time tick = zsLib::now();

      if (mCurrentChannel) {
        if (Time() != mLastSwitchedCurrentChannel) {
          if (mLastSwitchedCurrentChannel + mLockAfterSwitchTime > tick) {
            ZS_LOG_INSANE(log("cannot switch channel (as locked out after last switch)"))
            return;
          }
        }
      }

      mLastSwitchedCurrentChannel = tick;
      mCurrentChannel = channelHolder;

      ZS_EVENTING_3(
                    x, i, Trace, RtpReceiverTrackActiveChannelChangedEvent, ol, RtpReceiver, InternalEvent,
                    puid, id, mID,
                    puid, trackId, mTrack->getID(),
                    puid, channelObjectId, channelHolder->getID()
                    );
      mTrack->notifyActiveReceiverChannel(RTPReceiverChannel::convert(channelHolder->mChannel));
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::resetActiveReceiverChannel()
    {
      if (!mCurrentChannel) return;

      mCurrentChannel.reset();
      mLastSwitchedCurrentChannel = Time();

      ZS_EVENTING_3(
                    x, i, Trace, RtpReceiverTrackActiveChannelChangedEvent, ol, RtpReceiver, InternalEvent,
                    puid, id, mID,
                    puid, trackId, mTrack->getID(),
                    puid, channelObjectId, 0
                    );

      mTrack->notifyActiveReceiverChannel(RTPReceiverChannelPtr());
    }

    //-------------------------------------------------------------------------
    Optional<RTPReceiver::RoutingPayloadType> RTPReceiver::decodeREDRoutingPayloadType(
                                                                                       const BYTE *buffer,
                                                                                       size_t bufferSizeInBytes
                                                                                       )
    {
      Optional<RTPReceiver::RoutingPayloadType> result;

      Optional<PayloadType> innerREDPayloadType = RTPTypesHelper::getRedCodecPayload(buffer, bufferSizeInBytes, buffer, bufferSizeInBytes);

      if (!innerREDPayloadType.hasValue()) return result;

      auto found = mCodecInfos.find(innerREDPayloadType.value());
      if (found == mCodecInfos.end()) return result;

      auto &codecInfo = (*found).second;

      switch (codecInfo.mCodecType)
      {
        case CodecType_Normal:
        {
          result = getMediaCodecRoutingPayload(innerREDPayloadType.value());
          return result;
        }
        case CodecType_ULPFEC:
        case CodecType_FlexFEC:
        {
          result = decodeFECPayloadType(buffer, bufferSizeInBytes);
          if (!result.hasValue()) return result;

          result = static_cast<RoutingPayloadType>((static_cast<RoutingPayloadType>(innerREDPayloadType.value()) << 8) | result.value());
          return result;
        }
        case CodecType_RED:
        case CodecType_RTX: break;
      }

      ZS_LOG_WARNING(Trace, log("RED packet contains unsupported codec") + codecInfo.toDebug())
      return result;
    }

    //-------------------------------------------------------------------------
    Optional<RTPReceiver::RoutingPayloadType> RTPReceiver::decodeFECPayloadType(
                                                                                const BYTE *buffer,
                                                                                size_t bufferSizeInBytes
                                                                                )
    {
      Optional<RTPReceiver::RoutingPayloadType> result;
      Optional<PayloadType> recoveryPayloadType = RTPTypesHelper::getFecRecoveryPayloadType(buffer, bufferSizeInBytes);
      if (recoveryPayloadType.hasValue()) return result;

      result = static_cast<RoutingPayloadType>(recoveryPayloadType.value());
      return result;
    }

    //-------------------------------------------------------------------------
    Optional<RTPReceiver::RoutingPayloadType> RTPReceiver::getRoutingPayloadType(const RTPPacket &rtpPacket)
    {
      Optional<RoutingPayloadType> result;

      auto found = mCodecInfos.find(rtpPacket.pt());
      if (found == mCodecInfos.end()) return result;

      auto &codecInfo = (*found).second;

      switch (codecInfo.mCodecType) {
        case CodecType_Normal:
        {
          result = getMediaCodecRoutingPayload(rtpPacket.pt());
          return result;
        }
        case CodecType_RED:
        {
          result = decodeREDRoutingPayloadType(rtpPacket.payload(), rtpPacket.payloadSize());
          if (!result.hasValue()) return result;

          result = static_cast<RoutingPayloadType>((static_cast<RoutingPayloadType>(rtpPacket.pt()) << 16) | result.value());
          return result;
        }
        case CodecType_ULPFEC:
        case CodecType_FlexFEC:
        {
          result = decodeFECPayloadType(rtpPacket.payload(), rtpPacket.payloadSize());
          if (!result.hasValue()) return result;

          result = static_cast<RoutingPayloadType>((static_cast<RoutingPayloadType>(rtpPacket.pt()) << 8) | result.value());
          return result;
        }
        case CodecType_RTX:     break;
      }

      RoutingPayloadType aptPayloadType = static_cast<RoutingPayloadType>(codecInfo.mOriginalCodec->mPayloadType);

      switch (codecInfo.mOriginalCodec->mCodecType)
      {
        case CodecType_Normal:
        {
          result = static_cast<RoutingPayloadType>((static_cast<RoutingPayloadType>(rtpPacket.pt()) << 24) | aptPayloadType);
          return result;
        }
        case CodecType_RED:
        {
          result = decodeREDRoutingPayloadType(rtpPacket.payload(), rtpPacket.payloadSize());
          if (!result.hasValue()) return result;

          result = static_cast<RoutingPayloadType>((static_cast<RoutingPayloadType>(rtpPacket.pt()) << 24) | (aptPayloadType << 16) | result.value());
          return result;
        }
        case CodecType_ULPFEC:
        case CodecType_FlexFEC:
        {
          result = decodeFECPayloadType(rtpPacket.payload(), rtpPacket.payloadSize());
          if (!result.hasValue()) return result;

          result = static_cast<RoutingPayloadType>((static_cast<RoutingPayloadType>(rtpPacket.pt()) << 24) | (aptPayloadType << 8) | result.value());
          return result;
        }
        case CodecType_RTX:  break;
      }

      // should not be possible
      ASSERT(false);
      return result;
    }

    //-------------------------------------------------------------------------
    Optional<RTPReceiver::RoutingPayloadType> RTPReceiver::getRoutingPayload(const RTPTypesHelper::DecodedCodecInfo &decodedCodec)
    {
      Optional<RoutingPayloadType> result;

      switch (decodedCodec.mDepth[0].mCodecKind) {
        case IRTPTypes::CodecKind_Unknown:
        {
          ASSERT(false);
          return result;
        }
        case IRTPTypes::CodecKind_Audio:
        case IRTPTypes::CodecKind_AudioSupplemental:
        case IRTPTypes::CodecKind_Video:
        case IRTPTypes::CodecKind_AV:
        case IRTPTypes::CodecKind_Data:
        {
          result = getMediaCodecRoutingPayload(decodedCodec.mDepth[0].mCodecParameters->mPayloadType);
          return result;
        }
        case IRTPTypes::CodecKind_FEC:
        {
          switch (decodedCodec.mFilledDepth)
          {
            case 0: break;
            case 1:
            {
              result = (static_cast<RoutingPayloadType>(decodedCodec.mDepth[0].mCodecParameters->mPayloadType) << 8) | static_cast<RoutingPayloadType>(decodedCodec.mDepth[1].mCodecParameters->mPayloadType);
              return result;
            }
            case 2:
            {
              result = (static_cast<RoutingPayloadType>(decodedCodec.mDepth[0].mCodecParameters->mPayloadType) << 16) | (static_cast<RoutingPayloadType>(decodedCodec.mDepth[1].mCodecParameters->mPayloadType) << 8) | static_cast<RoutingPayloadType>(decodedCodec.mDepth[2].mCodecParameters->mPayloadType);
              return result;
            }
            case 3: break;
          }
          return result;
        }
        case IRTPTypes::CodecKind_RTX:
        {
          break;
        }
      }

      switch (decodedCodec.mDepth[1].mCodecKind) {
        case IRTPTypes::CodecKind_Unknown:
        case IRTPTypes::CodecKind_RTX:
        {
          ASSERT(false);
          break;
        }
        case IRTPTypes::CodecKind_Audio:
        case IRTPTypes::CodecKind_AudioSupplemental:
        case IRTPTypes::CodecKind_Video:
        case IRTPTypes::CodecKind_AV:
        case IRTPTypes::CodecKind_Data:
        {
          result = (static_cast<RoutingPayloadType>(decodedCodec.mDepth[0].mCodecParameters->mPayloadType) << 24) | static_cast<RoutingPayloadType>(decodedCodec.mDepth[1].mCodecParameters->mPayloadType);
          return result;
        }
        case IRTPTypes::CodecKind_FEC:
        {
          switch (decodedCodec.mFilledDepth)
          {
            case 0: break;
            case 1: break;
            case 2: break;
            {
              result = (static_cast<RoutingPayloadType>(decodedCodec.mDepth[0].mCodecParameters->mPayloadType) << 24) | (static_cast<RoutingPayloadType>(decodedCodec.mDepth[1].mCodecParameters->mPayloadType) << 8) | static_cast<RoutingPayloadType>(decodedCodec.mDepth[2].mCodecParameters->mPayloadType);
              return result;
            }
            case 3:
            {
              result = (static_cast<RoutingPayloadType>(decodedCodec.mDepth[0].mCodecParameters->mPayloadType) << 24) | (static_cast<RoutingPayloadType>(decodedCodec.mDepth[1].mCodecParameters->mPayloadType) << 16) | (static_cast<RoutingPayloadType>(decodedCodec.mDepth[2].mCodecParameters->mPayloadType) << 8) | static_cast<RoutingPayloadType>(decodedCodec.mDepth[3].mCodecParameters->mPayloadType);
              return result;
            }
          }
          break;
        }
      }
      return result;
    }

    //-------------------------------------------------------------------------
    RTPReceiver::RoutingPayloadType RTPReceiver::getMediaCodecRoutingPayload(PayloadType originalPayload)
    {
      return static_cast<RoutingPayloadType>(originalPayload);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::getFECMediaCodecRoutingPayload(
                                                     PayloadType originalPayload,
                                                     IRTPTypes::KnownFECMechanisms mechanism,
                                                     Optional<RoutingPayloadType> &resultFECv1,
                                                     Optional<RoutingPayloadType> &resultFECv2,
                                                     PayloadType *outPayloadTypeFECv1,
                                                     PayloadType *outPayloadTypeFECv2
                                                     )
    {
      auto found = mCodecInfos.find(originalPayload);
      if (found == mCodecInfos.end()) return;

      auto &codecInfo = (*found).second;
      
      switch (mechanism) {
        case IRTPTypes::KnownFECMechanism_Unknown:
        case IRTPTypes::KnownFECMechanism_RED:        return;
        case IRTPTypes::KnownFECMechanism_FlexFEC:
        {
          RTPTypesHelper::FindCodecOptions findOptions;
          findOptions.mSupportedCodec = IRTPTypes::SupportedCodec_FlexFEC;
          findOptions.mClockRate = codecInfo.mOriginalCodecParams->mClockRate;
          findOptions.mMatchClockRateNotSet = true;

          auto flexFECCodec = RTPTypesHelper::findCodec(*mParameters, findOptions);
          if (NULL == flexFECCodec) return;

          if (NULL != outPayloadTypeFECv1) {
            *outPayloadTypeFECv1 = flexFECCodec->mPayloadType;
          }

          resultFECv1 = (static_cast<RoutingPayloadType>(flexFECCodec->mPayloadType) << 8) | static_cast<RoutingPayloadType>(originalPayload);
          return;
        }
        case IRTPTypes::KnownFECMechanism_RED_ULPFEC: break;
      }

      const CodecParameters *foundULPFECCodec = NULL;
      const CodecParameters *foundRED1Codec = NULL;
      const CodecParameters *foundRED2Codec = NULL;

      {
        RTPTypesHelper::FindCodecOptions findOptions;
        findOptions.mSupportedCodec = IRTPTypes::SupportedCodec_ULPFEC;
        findOptions.mClockRate = codecInfo.mOriginalCodecParams->mClockRate;
        findOptions.mMatchClockRateNotSet = true;

        foundULPFECCodec = RTPTypesHelper::findCodec(*mParameters, findOptions);
        if (NULL == foundULPFECCodec) return;
      }

      {
        RTPTypesHelper::FindCodecOptions findOptions;
        findOptions.mSupportedCodec = IRTPTypes::SupportedCodec_RED;
        findOptions.mClockRate = codecInfo.mOriginalCodecParams->mClockRate;
        findOptions.mMatchClockRateNotSet = true;

        RTPTypesHelper::FindCodecOptions::PayloadTypeSet payloads;
        payloads.insert(originalPayload);

        findOptions.mREDCodecPayloadTypes = payloads;
        findOptions.mAllowREDMatchEmptyList = true;

        foundRED1Codec = RTPTypesHelper::findCodec(*mParameters, findOptions);
        if (NULL == foundRED1Codec) return;
      }
      {
        RTPTypesHelper::FindCodecOptions findOptions;
        findOptions.mSupportedCodec = IRTPTypes::SupportedCodec_RED;
        findOptions.mClockRate = codecInfo.mOriginalCodecParams->mClockRate;
        findOptions.mMatchClockRateNotSet = true;

        RTPTypesHelper::FindCodecOptions::PayloadTypeSet payloads;
        payloads.insert(foundULPFECCodec->mPayloadType);

        findOptions.mREDCodecPayloadTypes = payloads;
        findOptions.mAllowREDMatchEmptyList = true;

        foundRED2Codec = RTPTypesHelper::findCodec(*mParameters, findOptions);
        if (NULL == foundRED2Codec) return;
      }

      if (NULL != outPayloadTypeFECv1) {
        *outPayloadTypeFECv1 = foundRED1Codec->mPayloadType;
      }
      if (NULL != outPayloadTypeFECv2) {
        *outPayloadTypeFECv2 = foundRED2Codec->mPayloadType;
      }

      resultFECv1 = (static_cast<RoutingPayloadType>(foundRED1Codec->mPayloadType) << 16) | static_cast<RoutingPayloadType>(originalPayload);
      resultFECv2 = (static_cast<RoutingPayloadType>(foundRED2Codec->mPayloadType) << 16) | (static_cast<RoutingPayloadType>(foundULPFECCodec->mPayloadType) << 8) | static_cast<RoutingPayloadType>(originalPayload);
    }

    //-------------------------------------------------------------------------
    Optional<RTPReceiver::RoutingPayloadType> RTPReceiver::getRtxMediaCodecRoutingPayload(PayloadType originalPayload)
    {
      Optional<RoutingPayloadType> result;

      auto found = mCodecInfos.find(originalPayload);
      if (found == mCodecInfos.end()) return result;

      auto &codecInfo = (*found).second;

      RTPTypesHelper::FindCodecOptions findOptions;
      findOptions.mSupportedCodec = IRTPTypes::SupportedCodec_RTX;
      findOptions.mClockRate = codecInfo.mOriginalCodecParams->mClockRate;
      findOptions.mMatchClockRateNotSet = true;
      findOptions.mRTXAptPayloadType = originalPayload;

      auto *foundRTXCodec = RTPTypesHelper::findCodec(*mParameters, findOptions);
      if (NULL == foundRTXCodec) return result;

      result = (static_cast<RoutingPayloadType>(foundRTXCodec->mPayloadType) << 24) | static_cast<RoutingPayloadType>(originalPayload);
      return result;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::getRtxFECMediaCodecRoutingPayload(
                                                     PayloadType originalPayload,
                                                     IRTPTypes::KnownFECMechanisms mechanism,
                                                     Optional<RoutingPayloadType> &resultFECv1,
                                                     Optional<RoutingPayloadType> &resultFECv2
                                                     )
    {
      auto found = mCodecInfos.find(originalPayload);
      if (found == mCodecInfos.end()) return;

      auto &codecInfo = (*found).second;

      PayloadType payloadTypeFECv1 {};
      PayloadType payloadTypeFECv2 {};

      getFECMediaCodecRoutingPayload(originalPayload, mechanism, resultFECv1, resultFECv2, &payloadTypeFECv1, &payloadTypeFECv2);

      RTPTypesHelper::FindCodecOptions findOptions;
      findOptions.mSupportedCodec = IRTPTypes::SupportedCodec_RTX;
      findOptions.mClockRate = codecInfo.mOriginalCodecParams->mClockRate;
      findOptions.mMatchClockRateNotSet = true;

      if (resultFECv1.hasValue()) {
        findOptions.mRTXAptPayloadType = payloadTypeFECv1;
        auto *foundRTXCodec = RTPTypesHelper::findCodec(*mParameters, findOptions);
        if (NULL == foundRTXCodec) return;

        resultFECv1 = (static_cast<RoutingPayloadType>(foundRTXCodec->mPayloadType) << 24) | resultFECv1.value();
      }
      if (resultFECv2.hasValue()) {
        findOptions.mRTXAptPayloadType = payloadTypeFECv2;
        auto *foundRTXCodec = RTPTypesHelper::findCodec(*mParameters, findOptions);
        if (NULL == foundRTXCodec) return;

        resultFECv2 = (static_cast<RoutingPayloadType>(foundRTXCodec->mPayloadType) << 24) | resultFECv2.value();
      }
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
                                               Kinds kind,
                                               IRTPTransportPtr transport,
                                               IRTCPTransportPtr rtcpTransport
                                               )
    {
      if (this) {}
      return internal::RTPReceiver::create(delegate, kind, transport, rtcpTransport);
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
    UseServicesHelper::debugAppend(resultEl, "voice activity flag", mVoiceActivityFlag);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPReceiverTypes::ContributingSource::hash() const
  {
    auto hasher = IHasher::sha1();
    hasher->update("IRTPReceiverTypes:ContributingSource:");
    hasher->update(mTimestamp);
    hasher->update(":");
    hasher->update(mCSRC);
    hasher->update(":");
    hasher->update(mAudioLevel);
    hasher->update(":");
    hasher->update(mVoiceActivityFlag);
    return hasher->finalizeAsString();
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
                                       Kinds kind,
                                       IRTPTransportPtr transport,
                                       IRTCPTransportPtr rtcpTransport
                                       )
  {
    return internal::IRTPReceiverFactory::singleton().create(delegate, kind, transport, rtcpTransport);
  }

  //---------------------------------------------------------------------------
  IRTPReceiverTypes::CapabilitiesPtr IRTPReceiver::getCapabilities(Optional<Kinds> kind)
  {
    return internal::IRTPReceiverFactory::singleton().getCapabilities(kind);
  }

}
