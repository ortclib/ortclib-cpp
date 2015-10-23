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
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_RTPTypes.h>
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
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IRTPReceiverForSettings::applyDefaults()
    {
      UseSettings::setUInt(ORTC_SETTING_RTP_RECEIVER_SSRC_TIMEOUT_IN_SECONDS, 60);

      UseSettings::setUInt(ORTC_SETTING_RTP_RECEIVER_MAX_RTP_PACKETS_IN_BUFFER, 100);
      UseSettings::setUInt(ORTC_SETTING_RTP_RECEIVER_MAX_AGE_RTP_PACKETS_IN_SECONDS, 30);
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
    
    //---------------------------------------------------------------------------
    ElementPtr RTPReceiver::RegisteredHeaderExtension::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::RegisteredHeaderExtension");

      UseServicesHelper::debugAppend(resultEl, "header extension uri", IRTPTypes::toString(mHeaderExtensionURI));
      UseServicesHelper::debugAppend(resultEl, "local id", mLocalID);
      UseServicesHelper::debugAppend(resultEl, "encrypted", mEncrypted);

      return resultEl;
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

      ASSERT((bool)mChannel)

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
      mChannel->update(params);
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
      if (mOriginalParameters->mEncodingParameters.size() < 1) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    String RTPReceiver::ChannelInfo::rid() const
    {
      if (shouldLatchAll()) return String();
      return mFilledParameters->mEncodingParameters.front().mEncodingID;
    }

    //-------------------------------------------------------------------------
    RTPReceiver::SSRCInfoPtr RTPReceiver::ChannelInfo::registerSSRCUsage(SSRCInfoPtr ssrcInfo)
    {
      mRegisteredSSRCs[ssrcInfo->mSSRC] = ssrcInfo;
      return ssrcInfo;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::ChannelInfo::unregisterSSRCUsage(SSRCType ssrc)
    {
      auto found = mRegisteredSSRCs.find(ssrc);
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
          UseServicesHelper::debugAppend(ssrcsEl, ssrcInfo->toDebug());
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
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mChannels(make_shared<ChannelWeakMap>()),
      mMaxBufferedRTPPackets(SafeInt<decltype(mMaxBufferedRTPPackets)>(UseSettings::getUInt(ORTC_SETTING_RTP_RECEIVER_MAX_RTP_PACKETS_IN_BUFFER))),
      mMaxRTPPacketAge(UseSettings::getUInt(ORTC_SETTING_RTP_RECEIVER_MAX_AGE_RTP_PACKETS_IN_SECONDS))
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

      mSSRCTableExpires = Seconds(UseSettings::getUInt(ORTC_SETTING_RTP_RECEIVER_SSRC_TIMEOUT_IN_SECONDS));
      if (mSSRCTableExpires < Seconds(1)) {
        mSSRCTableExpires = Seconds(1);
      }

      mSSRCTableTimer = Timer::create(mThisWeak.lock(), (zsLib::toMilliseconds(mSSRCTableExpires) / 2));

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
        mListener->registerReceiver(mThisWeak.lock(), *mParameters, &historicalRTCPPackets);

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

      notifyChannelsOfTransportState();
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
      typedef RTPTypesHelper::ParametersPtrPairList ParametersPtrPairList;

      AutoRecursiveLock lock(*this);

      Optional<IMediaStreamTrack::Kinds> foundKind;

      // scope: figure out codec "kind"
      {
        for (auto iter = parameters.mCodecs.begin(); iter != parameters.mCodecs.end(); ++iter) {
          auto &codec = (*iter);

          auto codecKind = IRTPTypes::toCodecKind(codec.mName);

          switch (codecKind) {
            case IRTPTypes::CodecKind_Audio:
            case IRTPTypes::CodecKind_AudioSupplemental:
            {
              if (foundKind.hasValue()) {
                ORTC_THROW_INVALID_PARAMETERS_IF(foundKind.value() != IMediaStreamTrack::Kind_Audio)
              }
              foundKind = IMediaStreamTrack::Kind_Audio;
              break;
            }
            case IRTPTypes::CodecKind_Video:
            {
              if (foundKind.hasValue()) {
                ORTC_THROW_INVALID_PARAMETERS_IF(foundKind.value() != IMediaStreamTrack::Kind_Video)
              }
              foundKind = IMediaStreamTrack::Kind_Video;
              break;
            }
            case IRTPTypes::CodecKind_Unknown:
            case IRTPTypes::CodecKind_AV:
            case IRTPTypes::CodecKind_RTX:
            case IRTPTypes::CodecKind_FEC:
            case IRTPTypes::CodecKind_Data:
            {
              // codec kind is not a media kind
              break;
            }
          }
        }
      }

      if (!mTrack) {
        ORTC_THROW_INVALID_PARAMETERS_IF(!foundKind.hasValue())

        ZS_LOG_DEBUG(log("creating media stream track") + ZS_PARAM("kind", IMediaStreamTrack::toString(foundKind.value())))

        mTrack = UseMediaStreamTrack::create(foundKind.value());

        ZS_LOG_DEBUG(log("created media stream track") + ZS_PARAM("kind", IMediaStreamTrack::toString(foundKind.value())) + ZS_PARAM("track", mTrack ? mTrack->getID() : 0))
      }

      if (mParameters) {
        auto hash = parameters.hash();
        auto previousHash = mParameters->hash();
        if (hash == previousHash) {
          ZS_LOG_TRACE(log("receive has not changed (noop)"))
          return;
        }

        bool oldShouldLatchAll = shouldLatchAll();
        ParametersPtrList oldGroupedParams = mParametersGroupedIntoChannels;

        mParameters = make_shared<Parameters>(parameters);

        mParametersGroupedIntoChannels.clear();
        RTPTypesHelper::splitParamsIntoChannels(parameters, mParametersGroupedIntoChannels);

        ParametersPtrList unchangedChannels;
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
      } else {
        mParameters = make_shared<Parameters>(parameters);

        RTPTypesHelper::splitParamsIntoChannels(parameters, mParametersGroupedIntoChannels);

        for (auto iter = mParametersGroupedIntoChannels.begin(); iter != mParametersGroupedIntoChannels.end(); ++iter) {
          auto &params = (*iter);
          addChannel(params);
        }
      }

      mListener->registerReceiver(mThisWeak.lock(), *mParameters);

      registerHeaderExtensions(*mParameters);
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

      ChannelHolderPtr channelHolder;

      {
        AutoRecursiveLock lock(*this);

        if (isShutdown()) {
          ZS_LOG_WARNING(Debug, log("ignoring packet (already shutdown)"))
          return false;
        }

        String rid;
        if (findMapping(*packet, channelHolder, rid)) goto process_rtp;

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
        ZS_LOG_TRACE(log("forwarding RTP packet to channel") + ZS_PARAM("channel id", channelHolder->getID()) + ZS_PARAM("ssrc", packet->ssrc()))
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
      ZS_LOG_TRACE(log("received packet") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + packet->toDebug())

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

      if (timer == mSSRCTableTimer) {

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

          SSRCType ssrc = (*current).first;

          ZS_LOG_TRACE(log("expiring SSRC to RID mapping") + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("last received", lastReceived) + ZS_PARAM("adjusted tick", adjustedTick))
          mSSRCTable.erase(current);
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

      UseServicesHelper::debugAppend(resultEl, "ssrc table", mSSRCTable.size());
      UseServicesHelper::debugAppend(resultEl, "registered ssrcs", mRegisteredSSRCs.size());

      UseServicesHelper::debugAppend(resultEl, "rid channel map", mRIDTable.size());

      UseServicesHelper::debugAppend(resultEl, "ssrc table timer", mSSRCTableTimer ? mSSRCTableTimer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "ssrc table expires", mSSRCTableExpires);

      UseServicesHelper::debugAppend(resultEl, "max buffered rtp packets", mMaxBufferedRTPPackets);
      UseServicesHelper::debugAppend(resultEl, "max rtp packet age", mMaxRTPPacketAge);

      UseServicesHelper::debugAppend(resultEl, "buffered rtp packets", mBufferedRTPPackets.size());
      UseServicesHelper::debugAppend(resultEl, "reattempt delivery", mReattemptRTPDelivery);

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
      }
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::stepAttemptDelivery()
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

        ChannelHolderPtr channelHolder;
        String rid;
        if (!findMapping(*packet, channelHolder, rid)) continue;

        ZS_LOG_TRACE(log("will attempt to deliver buffered RTP packet") + ZS_PARAM("channel", channelHolder->getID()) + ZS_PARAM("ssrc", packet->ssrc()))
        channelHolder->notify(packet);

        mBufferedRTPPackets.erase(current);
      }
      
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::stepCleanChannels()
    {
      if (!mCleanChannels) {
        ZS_LOG_TRACE(log("no need to reattempt clean channels at this time"))
        return true;
      }

      ZS_LOG_DEBUG(log("will attempt to clean channels"))

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
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
//        return;
      }

      //.......................................................................
      // final cleanup
#define TODO 1
#define TODO 2

      setState(State_Shutdown);

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      if (mParameters) {
        mListener->unregisterReceiver(*this);
      }

      mRegisteredExtensions.clear();

      mSSRCTable.clear();
      mRIDTable.clear();

      if (mSSRCTableTimer) {
        mSSRCTableTimer->cancel();
        mSSRCTableTimer.reset();
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
    bool RTPReceiver::shouldLatchAll()
    {
      if (1 != mChannelInfos.size()) return false;
      return (mChannelInfos.begin())->second->shouldLatchAll();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::notifyChannelsOfTransportState()
    {
      ISecureTransport::States currentState = ISecureTransport::State_Pending;

      if (mRTPTransport) {
        currentState = mRTPTransport->state();
        if (ISecureTransport::State_Closed == currentState) currentState = ISecureTransport::State_Disconnected;
      } else {
        currentState = ISecureTransport::State_Disconnected;
      }

      if (currentState == mLastReportedTransportStateToChannels) {
        ZS_LOG_TRACE(log("no change in secure transport state to notify") + ZS_PARAM("state", ISecureTransportTypes::toString(currentState)))
        return;
      }

      ZS_LOG_TRACE(log("notify secure transport state change") + ZS_PARAM("new state", ISecureTransportTypes::toString(currentState)) + ZS_PARAM("old state", ISecureTransportTypes::toString(mLastReportedTransportStateToChannels)))

      mLastReportedTransportStateToChannels = currentState;

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
        auto channelHolder = (*iter).second.lock();

        if (!channelHolder) {
        }

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

        channel->notify(ISecureTransport::State_Closed);
      }

      mChannels = make_shared<ChannelWeakMap>();  // all channels are now gone (COW with empty replacement list)
      mSSRCTable.clear();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::addChannel(ParametersPtr params)
    {
      ChannelInfoPtr channelInfo(make_shared<ChannelInfo>());
      channelInfo->mOriginalParameters = params;
      channelInfo->mFilledParameters = make_shared<Parameters>(*params);  // make a filled duplicate

      // don't create the channel until its actually needed

      mChannelInfos[params] = channelInfo;

      if (channelInfo->shouldLatchAll()) {
        ZS_LOG_TRACE(log("auto latching channel added") + channelInfo->toDebug())
        return;
      }

      auto &encodingParmas = channelInfo->mOriginalParameters->mEncodingParameters.front();

      setRIDUsage(encodingParmas.mEncodingID, channelInfo);

      ChannelHolderPtr channelHolder;
      if (encodingParmas.mSSRC.hasValue()) {
        registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(encodingParmas.mSSRC.value(), encodingParmas.mEncodingID, channelHolder)));
      }
      if ((encodingParmas.mRTX.hasValue()) &&
          (encodingParmas.mRTX.value().mSSRC.hasValue())) {
        registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(encodingParmas.mRTX.value().mSSRC.value(), encodingParmas.mEncodingID, channelHolder)));
      }
      if ((encodingParmas.mFEC.hasValue()) &&
          (encodingParmas.mFEC.value().mSSRC.hasValue())) {
        registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(encodingParmas.mFEC.value().mSSRC.value(), encodingParmas.mEncodingID, channelHolder)));
      }

      channelInfo->registerHolder(channelHolder);

      ZS_LOG_DEBUG(log("added channel") + channelInfo->toDebug())
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::updateChannel(
                                    ChannelInfoPtr channelInfo,
                                    ParametersPtr newParams
                                    )
    {
      bool wasLatchAll = channelInfo->shouldLatchAll();

      ParametersPtr oldOriginalParams = channelInfo->mOriginalParameters;
      ParametersPtr oldFilledParams = channelInfo->mFilledParameters;
      SSRCMap oldRegisteredSSRCs(channelInfo->mRegisteredSSRCs);

      channelInfo->mOriginalParameters = newParams;
      channelInfo->mFilledParameters = make_shared<Parameters>(*newParams);
      channelInfo->mRegisteredSSRCs.clear();

      if (wasLatchAll) {
        ZS_LOG_DEBUG(log("nothing to copy from old channel (thus skipping)"))

        if (channelInfo->shouldLatchAll()) {
          ZS_LOG_DEBUG(log("nothing to resgister (thus skipping)"))
          return;
        }

        auto &encodingParmas = channelInfo->mOriginalParameters->mEncodingParameters.front();

        setRIDUsage(encodingParmas.mEncodingID, channelInfo);

        ChannelHolderPtr channelHolder;
        if (encodingParmas.mSSRC.hasValue()) {
          registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(encodingParmas.mSSRC.value(), encodingParmas.mEncodingID, channelHolder)));
        }
        if ((encodingParmas.mRTX.hasValue()) &&
            (encodingParmas.mRTX.value().mSSRC.hasValue())) {
          registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(encodingParmas.mRTX.value().mSSRC.value(), encodingParmas.mEncodingID, channelHolder)));
        }
        if ((encodingParmas.mFEC.hasValue()) &&
            (encodingParmas.mFEC.value().mSSRC.hasValue())) {
          registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(encodingParmas.mFEC.value().mSSRC.value(), encodingParmas.mEncodingID, channelHolder)));
        }

        channelInfo->registerHolder(channelHolder);
        return;
      }

      auto &baseOldOriginalEncoding = (*(oldOriginalParams->mEncodingParameters.begin()));
      auto &baseOldFilledEncoding = (*(oldFilledParams->mEncodingParameters.begin()));

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

      auto &baseNewOriginalEncoding = (*(newParams->mEncodingParameters.begin()));
      auto &baseNewFilledEncoding = (*(channelInfo->mFilledParameters->mEncodingParameters.begin()));

      ChannelHolderPtr channelHolder;

      // scope: deregister the changed or removed SSRCs, register the new SSRC
      {
        if (baseOldOriginalEncoding.mSSRC.hasValue()) {
          if (baseNewOriginalEncoding.mSSRC.hasValue()) {
            if (baseOldOriginalEncoding.mSSRC.value() != baseNewOriginalEncoding.mSSRC.value()) {
              registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseNewOriginalEncoding.mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelHolder)));
            }
          }
        } else if (baseNewOriginalEncoding.mSSRC.hasValue()) {
          registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseNewOriginalEncoding.mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelHolder)));
        }

        if ((baseOldOriginalEncoding.mRTX.hasValue()) &&
            (baseOldOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
          if ((baseNewOriginalEncoding.mRTX.hasValue()) &&
              (baseNewOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
            if (baseOldOriginalEncoding.mRTX.value().mSSRC.value() != baseNewOriginalEncoding.mRTX.value().mSSRC.value()) {
              registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseNewOriginalEncoding.mRTX.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelHolder)));
            }
          }
        } else if ((baseNewOriginalEncoding.mRTX.hasValue()) &&
                   (baseNewOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
          registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseNewOriginalEncoding.mRTX.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelHolder)));
        }

        if ((baseOldOriginalEncoding.mFEC.hasValue()) &&
            (baseOldOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
          if ((baseNewOriginalEncoding.mFEC.hasValue()) &&
              (baseNewOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
            if (baseOldOriginalEncoding.mFEC.value().mSSRC.value() != baseNewOriginalEncoding.mRTX.value().mSSRC.value()) {
              registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseNewOriginalEncoding.mFEC.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelHolder)));
            }
          }
        } else if ((baseNewOriginalEncoding.mFEC.hasValue()) &&
                   (baseNewOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
          registerSSRCUsage(channelInfo->registerSSRCUsage(setSSRCUsage(baseNewOriginalEncoding.mFEC.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelHolder)));
        }

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
      // scope: clean out any SSRCs pointing to this channel
      {
        for (auto iter_doNotUse = mSSRCTable.begin(); iter_doNotUse != mSSRCTable.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &ssrcInfo = (*current).second;
          auto &channelHolder = ssrcInfo->mChannelHolder;

          if (!channelHolder) continue;

          auto &existingChannelInfo = channelHolder->mChannelInfo;

          if (!existingChannelInfo) continue;
          if (existingChannelInfo->mID != channelInfo.mID) continue;

          mSSRCTable.erase(current);
        }
      }

      // scope: clean out any registered SSRCs pointing to this channel
      {
        for (auto iter_doNotUse = mRegisteredSSRCs.begin(); iter_doNotUse != mRegisteredSSRCs.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto ssrcInfo = (*current).second.lock();
          if (!ssrcInfo) {
            mRegisteredSSRCs.erase(current);
            continue;
          }

          auto &channelHolder = ssrcInfo->mChannelHolder;

          if (!channelHolder) continue;

          auto &existingChannelInfo = channelHolder->mChannelInfo;

          if (!existingChannelInfo) continue;
          if (existingChannelInfo->mID != channelInfo.mID) continue;

          mRegisteredSSRCs.erase(current);
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

        mRegisteredExtensions[newExt.mLocalID] = newExt;
      }
    }

    //-------------------------------------------------------------------------
    RTPReceiver::SSRCInfoPtr RTPReceiver::setSSRCUsage(
                                                       SSRCType ssrc,
                                                       String &ioRID,
                                                       ChannelHolderPtr &ioChannelHolder
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
        ssrcInfo->mChannelHolder = ioChannelHolder;

        if (ioRID.hasData()) {
          ssrcInfo->mRID = ioRID;
        } else {
          if (ioChannelHolder) {
            ioRID = ssrcInfo->mRID = ioChannelHolder->mChannelInfo->rid();
          }
        }

        mSSRCTable[ssrc] = ssrcInfo;
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
      mRegisteredSSRCs[ssrcInfo->mSSRC] = ssrcInfo;
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
    bool RTPReceiver::shouldCleanChannel(bool shouldClean)
    {
      if (shouldClean) cleanChannels();
      return shouldClean;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::cleanChannels()
    {
      if (mCleanChannels) return;
      mCleanChannels = true;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMapping(
                                  const RTPPacket &rtpPacket,
                                  ChannelHolderPtr &outChannelHolder,
                                  String &outRID
                                  )
    {
      ChannelInfoPtr channelInfo;

      outRID = extractRID(rtpPacket, outChannelHolder);

      {
        if (outChannelHolder) goto fill_rid;

        if (findMappingUsingRID(outRID, rtpPacket, channelInfo, outChannelHolder)) goto fill_rid;

        if (findMappingUsingSSRCInEncodingParams(outRID, rtpPacket, channelInfo, outChannelHolder)) goto fill_rid;

        if (findMappingUsingPayloadType(outRID, rtpPacket, channelInfo, outChannelHolder)) goto fill_rid;

        return false;
      }

    fill_rid:
      {
        if (!outChannelHolder) {
          ASSERT((bool)channelInfo)

          createChannel(rtpPacket.ssrc(), outRID, channelInfo, outChannelHolder);

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

        setSSRCUsage(rtpPacket.ssrc(), ridStr, outChannelHolder);
        return ridStr;
      }

      String result;
      setSSRCUsage(rtpPacket.ssrc(), result, outChannelHolder);

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
      setSSRCUsage(rtpPacket.ssrc(), inRID, outChannelHolder);
      if (!outChannelInfo) return false;
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMappingUsingSSRCInEncodingParams(
                                                           const String &rid,
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
          for (auto iterParm = channelInfo->mFilledParameters->mEncodingParameters.begin(); iterParm != channelInfo->mFilledParameters->mEncodingParameters.end(); ++iterParm, first = false)
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
          setSSRCUsage(rtpPacket.ssrc(), inRID, outChannelHolder);
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::findMappingUsingPayloadType(
                                                  const String &rid,
                                                  const RTPPacket &rtpPacket,
                                                  ChannelInfoPtr &outChannelInfo,
                                                  ChannelHolderPtr &outChannelHolder
                                                  )
    {
      auto payloadType = rtpPacket.pt();

      CodecKinds foundKind = CodecKind_Unknown;

      CodecKinds kind = CodecKind_Unknown;
      CodecKinds initialKind = CodecKind_Unknown;

      for (auto iterCodec = mParameters->mCodecs.begin(); iterCodec != mParameters->mCodecs.end(); ++iterCodec) {
        auto &codecInfo = (*iterCodec);
        if (payloadType != codecInfo.mPayloadType) continue;

        auto supportedType = toSupportedCodec(codecInfo.mName);
        if (SupportedCodec_Unknown == supportedType) continue;

        initialKind = kind = getCodecKind(supportedType);
        if (CodecKind_Unknown == kind) continue;

        break;
      }

      for (auto iter = mChannelInfos.begin(); iter != mChannelInfos.end(); ++iter) {
        auto &channelInfo = (*iter).second;

        kind = initialKind; // reset to previous state before searching last channel's encoding

        if (CodecKind_Unknown == kind) {
          for (auto iterEncodings = channelInfo->mFilledParameters->mEncodingParameters.begin(); iterEncodings != channelInfo->mFilledParameters->mEncodingParameters.end(); ++iterEncodings) {
            auto &encoding = (*iterEncodings);

            if (!encoding.mRTX.hasValue()) continue;
            if (!encoding.mRTX.value().mPayloadType.hasValue()) continue;
            if (encoding.mRTX.value().mPayloadType.value() != payloadType) continue;

            // found a match based to a particular encoding's RTX payload type (thus assume it must be RTX)
            kind = CodecKind_RTX;
            break;
          }

          if (CodecKind_Unknown == kind) continue; // make sure this codec type is understood
        }

        if (channelInfo->mFilledParameters->mEncodingParameters.size() < 1) {
          // special case where this is a "match all" for the codec
#define TODO_MATCH_BEST_CHANNEL_INFO_OR_CREATE_A_NEW_CHANNEL 1
#define TODO_MATCH_BEST_CHANNEL_INFO_OR_CREATE_A_NEW_CHANNEL 2
//          outChannelInfo = channelInfo;
          goto found_channel;
        }

        for (auto encodingIter = channelInfo->mFilledParameters->mEncodingParameters.begin(); encodingIter != channelInfo->mFilledParameters->mEncodingParameters.end(); ++encodingIter) {

          auto &encodingInfo = (*encodingIter);

          if (encodingInfo.mCodecPayloadType.hasValue()) {
            bool foundMatch = false;
            if ((CodecKind_RTX == kind) &&
                ((encodingInfo.mRTX.hasValue()) &&
                 (encodingInfo.mRTX.value().mPayloadType.hasValue()) &&
                 (encodingInfo.mRTX.value().mPayloadType.value() == payloadType))) {
              foundMatch = true;
            }
            if (encodingInfo.mCodecPayloadType.value() == payloadType) foundMatch = true;
            if (!foundMatch) continue;  // do not allow non-matching codec types to match
          }

          switch (kind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_Video:
            case CodecKind_AV:
            case CodecKind_AudioSupplemental:
            case CodecKind_Data:
            {
              if (encodingInfo.mSSRC.hasValue()) break;                 // cannot match if there was an SSRC already attached
              goto found_channel;
            }
            case CodecKind_RTX:    {
              if ((encodingInfo.mRTX.hasValue()) &&
                  (encodingInfo.mRTX.value().mSSRC.hasValue())) break;  // cannot match if there was an SSRC already attached
              goto found_channel;
            }
            case CodecKind_FEC:    {
              if ((encodingInfo.mFEC.hasValue()) &&
                  (encodingInfo.mFEC.value().mSSRC.hasValue())) break;  // cannot match if there was an SSRC already attached
              goto found_channel;
            }
          }
        }

        continue;

      found_channel:
        {
          if (!outChannelInfo) outChannelInfo = channelInfo;
          if (outChannelInfo->mID < channelInfo->mID) continue; // smaller = older (and thus better match)

          // this is a better match
          outChannelInfo = channelInfo;
          foundKind = kind;
        }
      }

      if (!outChannelInfo) return false;

      // scope: fill in SSRC in encoding parameters
      {
        if (outChannelInfo->mFilledParameters->mEncodingParameters.size() < 1) goto insert_ssrc_into_table;

        for (auto encodingIter = outChannelInfo->mFilledParameters->mEncodingParameters.begin(); encodingIter != outChannelInfo->mFilledParameters->mEncodingParameters.end(); ++encodingIter) {

          auto &encodingInfo = (*encodingIter);

          switch (foundKind) {
            case CodecKind_Unknown:  ASSERT(false) break;
            case CodecKind_Audio:
            case CodecKind_Video:
            case CodecKind_AV:
            case CodecKind_AudioSupplemental:
            case CodecKind_Data:
            {
              if (encodingInfo.mSSRC.hasValue()) break;               // cannot match if there was an SSRC already attached
              encodingInfo.mSSRC = rtpPacket.ssrc();
              goto insert_ssrc_into_table;
            }
            case CodecKind_RTX:
            {
              if ((encodingInfo.mRTX.hasValue()) &&
                  (encodingInfo.mRTX.value().mSSRC.hasValue())) break; // cannot match if there was an SSRC already attached

              encodingInfo.mRTX.value().mSSRC = rtpPacket.ssrc();;
              goto insert_ssrc_into_table;
            }
            case CodecKind_FEC:
            {
              if ((encodingInfo.mFEC.hasValue()) &&
                  (encodingInfo.mFEC.value().mSSRC.hasValue())) break; // cannot match if there was an SSRC already attached

              encodingInfo.mFEC.value().mSSRC = rtpPacket.ssrc();;
              goto insert_ssrc_into_table;
            }
          }
        }

        ASSERT(false)
        return false;
      }

    insert_ssrc_into_table:
      {
        ZS_LOG_DEBUG(log("creating a new SSRC entry in SSRC table (based on payload type matching)") + outChannelInfo->toDebug())

        String inRID = rid;
        setSSRCUsage(rtpPacket.ssrc(), inRID, outChannelHolder);
      }

      return true;
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
        auto &encoding = ioChannelInfo->mFilledParameters->mEncodingParameters.front();

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
        setSSRCUsage(ssrc, inRID, ioChannelHolder);
        return;
      }

      RTCPPacketList historicalPackets;
      mListener->getPackets(historicalPackets);

      ioChannelHolder = make_shared<ChannelHolder>();

      ioChannelHolder->mHolder = mThisWeak.lock();
      ioChannelHolder->mChannelInfo = channelInfo;
      ioChannelHolder->mChannel = UseChannel::create(mThisWeak.lock(), *(channelInfo->mOriginalParameters), historicalPackets);
      ioChannelHolder->notify(mLastReportedTransportStateToChannels);

      // remember the channel (mChannels is using COW pattern)
      ChannelWeakMapPtr replacementChannels(make_shared<ChannelWeakMap>(*mChannels));
      (*replacementChannels)[ioChannelHolder->getID()] = ioChannelHolder;
      mChannels = replacementChannels;

      String inRID = rid;
      setSSRCUsage(ssrc, inRID, ioChannelHolder);
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::processUnhandled(
                                       const String &muxID,
                                       const String &rid,
                                       IRTPTypes::SSRCType ssrc,
                                       IRTPTypes::PayloadType payloadType
                                       )
    {
      ZS_LOG_TRACE(log("notifying listener of unhandled SSRC") + ZS_PARAM("mux id", muxID) + ZS_PARAM("rid", rid) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType))

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
            auto found = mSSRCTable.find(byeSSRC);
            if (found != mSSRCTable.end()) {
              auto &ssrcInfo = (*found).second;
              ZS_LOG_TRACE(log("removing ssrc table entry due to BYE") + ZS_PARAM("ssrc", byeSSRC) + ssrcInfo->toDebug())

              mSSRCTable.erase(found);
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
              auto iterFilledParams = channelInfo->mFilledParameters->mEncodingParameters.begin();
              auto iterOriginalParams = channelInfo->mOriginalParameters->mEncodingParameters.begin();

              for (; iterFilledParams != channelInfo->mFilledParameters->mEncodingParameters.end(); ++iterFilledParams, ++iterOriginalParams)
              {
                ASSERT(iterOriginalParams != channelInfo->mOriginalParameters->mEncodingParameters.end())
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
        auto found = mSSRCTable.find(sr->ssrcOfSender());
        if (found == mSSRCTable.end()) continue;

        String ignoredRID;
        ChannelHolderPtr channelHolder;
        setSSRCUsage(sr->ssrcOfSender(), ignoredRID, channelHolder);
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
