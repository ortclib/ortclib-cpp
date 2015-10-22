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
        case IRTPTypes::HeaderExtensionURI_MuxID:                             return true;
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
    #pragma mark RTPReceiver::ChannelInfo
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::ChannelInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver::ChannelInfo");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "channel params", mOriginalParams ? mOriginalParams->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "filled params", mFilledParams ? mFilledParams->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "channel", mChannel ? mChannel->getID() : 0);

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

      UseServicesHelper::debugAppend(resultEl, "last usage", mLastUsage);
      UseServicesHelper::debugAppend(resultEl, "rid", mRID);
      UseServicesHelper::debugAppend(resultEl, "registered usage count", mRegisteredUsageCount);
      UseServicesHelper::debugAppend(resultEl, mChannelInfo ? mChannelInfo->toDebug() : ElementPtr());

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
      mChannels(make_shared<ChannelMap>())
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

          if (shouldLatchAll()) {
            PUIDSet processed;

            for (auto iter = mSSRCTable.begin(); iter != mSSRCTable.end(); ++iter) {
              SSRCInfo &ssrcInfo = (*iter).second;
              auto &channelInfo = ssrcInfo.mChannelInfo;

              if (!channelInfo) continue;
              if (!channelInfo->mChannel) continue;

              auto found = processed.find(channelInfo->mID);
              if (found != processed.end()) continue;

              processed.insert(channelInfo->mID);

              if (!notifyPackets) {
                notifyPackets = make_shared<RTCPPacketList>(historicalRTCPPackets);
              }

              channelInfo->mChannel->notifyPackets(notifyPackets);
            }
          } else {
            for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
              auto &channel = (*iter).second;
              if (!notifyPackets) {
                notifyPackets = make_shared<RTCPPacketList>(historicalRTCPPackets);
              }

              channel->notifyPackets(notifyPackets);
            }
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

        ChannelMap removedChannels;

        // scope: remove dead channels
        {
          for (auto iter = removeChannels.begin(); iter != removeChannels.end(); ++iter) {
            auto &params = (*iter);
            auto found = mChannelInfos.find(params);
            ASSERT(found != mChannelInfos.end())

            if (found == mChannelInfos.end()) continue;

            auto channelInfo = (*found).second;
            if (channelInfo->mChannel) {
              removedChannels[channelInfo->mChannel->getID()] = channelInfo->mChannel;
            }

            removeChannel(*channelInfo);
            mChannelInfos.erase(found);
          }
        }

        // scope: perform COW for all removed channels
        {
          if (removedChannels.size() > 0) {
            ChannelMapPtr replacementChannels(make_shared<ChannelMap>(*mChannels));
            for (auto iter = removedChannels.begin(); iter != removedChannels.end(); ++iter) {
              ChannelID id = (*iter).second->getID();

              auto found = replacementChannels->find(id);
              if (found == replacementChannels->end()) {
                ZS_LOG_ERROR(Detail, log("channel existed in channel info list but not in channel list") + ZS_PARAM("channel", id))
                continue;
              }

              replacementChannels->erase(found);
            }

            mChannels = replacementChannels;  // finalize COW
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

      ChannelMapPtr channels;

      {
        AutoRecursiveLock lock(*this);
        channels = mChannels; // obtain pointer to COW list while inside a lock

#define TODO_PROCESS_BYES_AND_MAYBE_OTHER_STUFF 1
#define TODO_PROCESS_BYES_AND_MAYBE_OTHER_STUFF 2

      }

      auto result = false;
      for (auto iter = channels->begin(); iter != channels->end(); ++iter)
      {
        auto &channel = (*iter).second;
        auto channelResult = channel->handlePacket(packet);
        result = result || channelResult;
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

          SSRCInfo &ssrcInfo = (*current).second;

          if (ssrcInfo.mRegisteredUsageCount > 0) {
            ZS_LOG_INSANE(log("cannot expire registered SSRC usages") + ssrcInfo.toDebug())
            continue;
          }

          const Time &lastReceived = ssrcInfo.mLastUsage;

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
      UseServicesHelper::debugAppend(resultEl, "channel infos", mChannelInfos.size());

      UseServicesHelper::debugAppend(resultEl, "ssrc table", mSSRCTable.size());

      UseServicesHelper::debugAppend(resultEl, "rid channel map", mRIDToChannelMap.size());

      UseServicesHelper::debugAppend(resultEl, "ssrc table timer", mSSRCTableTimer ? mSSRCTableTimer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "ssrc table expires", mSSRCTableExpires);

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

      mRegisteredExtensions.clear();

      mSSRCTable.clear();
      mRIDToChannelMap.clear();

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

      auto &params = (mChannelInfos.begin())->first;

      if (params->mEncodingParameters.size() < 1) return true;

      return false;
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
        auto &channel = (*iter).second;

        channel->notifyTransportState(mLastReportedTransportStateToChannels);
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::flushAllAutoLatchedChannels()
    {
      ZS_LOG_TRACE(log("flushing all auto-latched channels") + ZS_PARAM("channels", mChannels->size()))

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter)
      {
        auto &channel = (*iter).second;

        channel->notifyTransportState(ISecureTransport::State_Closed);
      }

      mChannels = make_shared<ChannelMap>();  // all channels are now gone (COW with empty replacement list)
      mSSRCTable.clear();
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::addChannel(ParametersPtr params)
    {
      ChannelInfoPtr channelInfo(make_shared<ChannelInfo>());
      channelInfo->mOriginalParams = params;
      channelInfo->mFilledParams = make_shared<Parameters>(*params);  // make a filled duplicate

      // don't create the channel until its actually needed

      mChannelInfos[params] = channelInfo;

      if (channelInfo->mOriginalParams->mEncodingParameters.size() < 1) {
        ZS_LOG_TRACE(log("auto latching channel added") + channelInfo->toDebug())
        return;
      }

      auto &encodingParmas = channelInfo->mOriginalParams->mEncodingParameters.front();

      if (encodingParmas.mEncodingID.hasData()) {
        mRIDToChannelMap[encodingParmas.mEncodingID] = channelInfo;
      }

      if (encodingParmas.mSSRC.hasValue()) {
        setSSRCUsage(encodingParmas.mSSRC.value(), encodingParmas.mEncodingID, channelInfo, true);
      }
      if ((encodingParmas.mRTX.hasValue()) &&
          (encodingParmas.mRTX.value().mSSRC.hasValue())) {
        setSSRCUsage(encodingParmas.mRTX.value().mSSRC.value(), encodingParmas.mEncodingID, channelInfo, true);
      }
      if ((encodingParmas.mFEC.hasValue()) &&
          (encodingParmas.mFEC.value().mSSRC.hasValue())) {
        setSSRCUsage(encodingParmas.mFEC.value().mSSRC.value(), encodingParmas.mEncodingID, channelInfo, true);
      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::updateChannel(
                                    ChannelInfoPtr channelInfo,
                                    ParametersPtr newParams
                                    )
    {
      ParametersPtr oldOriginalParams = channelInfo->mOriginalParams;
      ParametersPtr oldFilledParams = channelInfo->mFilledParams;

      channelInfo->mOriginalParams = newParams;
      channelInfo->mFilledParams = make_shared<Parameters>(*newParams);

      if (oldOriginalParams->mEncodingParameters.size() < 1) {
        ZS_LOG_DEBUG(log("nothing to copy from old channel (this skipping)"))
        return;
      }

      auto iterOldOriginalEncodings = oldOriginalParams->mEncodingParameters.begin();
      auto iterOldFilledEncodings = oldFilledParams->mEncodingParameters.begin();

      auto &baseOldOriginalEncoding = (*iterOldOriginalEncodings);
      auto &baseOldFilledEncoding = (*iterOldFilledEncodings);

      if (newParams->mEncodingParameters.size() < 1) {
        ZS_LOG_DEBUG(log("new params now a catch all for all encoding for this channel"))

        if (baseOldOriginalEncoding.mEncodingID.hasData()) {
          auto found = mRIDToChannelMap.find(baseOldOriginalEncoding.mEncodingID);
          if (found != mRIDToChannelMap.end()) {
            mRIDToChannelMap.erase(found);
          }
        }

        if (baseOldOriginalEncoding.mSSRC.hasValue()) {
          unregisterSSRCUsage(baseOldOriginalEncoding.mSSRC.value());
        }

        if ((baseOldOriginalEncoding.mRTX.hasValue()) &&
            (baseOldOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
          unregisterSSRCUsage(baseOldOriginalEncoding.mRTX.value().mSSRC.value());
        }

        if ((baseOldOriginalEncoding.mFEC.hasValue()) &&
            (baseOldOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
          unregisterSSRCUsage(baseOldOriginalEncoding.mFEC.value().mSSRC.value());
        }
        return;
      }

      auto iterNewOriginalEncodings = newParams->mEncodingParameters.begin();
      auto iterNewFilledEncodings = channelInfo->mFilledParams->mEncodingParameters.begin();

      auto &baseNewOriginalEncoding = (*iterNewOriginalEncodings);
      auto &baseNewFilledEncoding = (*iterNewFilledEncodings);

      // scope: deregister the changed or removed SSRCs, register the new SSRC
      {
        if (baseOldOriginalEncoding.mSSRC.hasValue()) {
          if (baseNewOriginalEncoding.mSSRC.hasValue()) {
            if (baseOldOriginalEncoding.mSSRC.value() != baseNewOriginalEncoding.mSSRC.value()) {
              unregisterSSRCUsage(baseOldOriginalEncoding.mSSRC.value());
              setSSRCUsage(baseNewOriginalEncoding.mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelInfo, true);
            }
          } else {
            unregisterSSRCUsage(baseOldOriginalEncoding.mSSRC.value());
          }
        } else if (baseNewOriginalEncoding.mSSRC.hasValue()) {
          setSSRCUsage(baseNewOriginalEncoding.mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelInfo, true);
        }

        if ((baseOldOriginalEncoding.mRTX.hasValue()) &&
            (baseOldOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
          if ((baseNewOriginalEncoding.mRTX.hasValue()) &&
              (baseNewOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
            if (baseOldOriginalEncoding.mRTX.value().mSSRC.value() != baseNewOriginalEncoding.mRTX.value().mSSRC.value()) {
              unregisterSSRCUsage(baseOldOriginalEncoding.mRTX.value().mSSRC.value());
              setSSRCUsage(baseNewOriginalEncoding.mRTX.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelInfo, true);
            }
          } else {
            unregisterSSRCUsage(baseOldOriginalEncoding.mRTX.value().mSSRC.value());
          }
        } else if ((baseNewOriginalEncoding.mRTX.hasValue()) &&
                   (baseNewOriginalEncoding.mRTX.value().mSSRC.hasValue())) {
          setSSRCUsage(baseNewOriginalEncoding.mRTX.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelInfo, true);
        }

        if ((baseOldOriginalEncoding.mFEC.hasValue()) &&
            (baseOldOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
          if ((baseNewOriginalEncoding.mFEC.hasValue()) &&
              (baseNewOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
            if (baseOldOriginalEncoding.mFEC.value().mSSRC.value() != baseNewOriginalEncoding.mRTX.value().mSSRC.value()) {
              unregisterSSRCUsage(baseOldOriginalEncoding.mFEC.value().mSSRC.value());
              setSSRCUsage(baseNewOriginalEncoding.mFEC.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelInfo, true);
            }
          } else {
            unregisterSSRCUsage(baseOldOriginalEncoding.mFEC.value().mSSRC.value());
          }
        } else if ((baseNewOriginalEncoding.mFEC.hasValue()) &&
                   (baseNewOriginalEncoding.mFEC.value().mSSRC.hasValue())) {
          setSSRCUsage(baseNewOriginalEncoding.mFEC.value().mSSRC.value(), baseNewOriginalEncoding.mEncodingID, channelInfo, true);
        }
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
          auto &existingChannelInfo = ssrcInfo.mChannelInfo;

          if (!existingChannelInfo) continue;
          if (existingChannelInfo->mID != channelInfo.mID) continue;

          mSSRCTable.erase(current);
        }
      }

      // scope: clean out any RIDs pointing to this channel
      {
        for (auto iter_doNotUse = mRIDToChannelMap.begin(); iter_doNotUse != mRIDToChannelMap.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &existingChannel = (*current).second;

          if (existingChannel->mID != channelInfo.mID) continue;

          mRIDToChannelMap.erase(current);
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
    String RTPReceiver::extractRID(const RTPPacket &rtpPacket)
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

        ChannelInfoPtr ignored;
        setSSRCUsage(rtpPacket.ssrc(), ridStr, ignored, false);
        return ridStr;
      }

      String result;
      ChannelInfoPtr ignored;
      setSSRCUsage(rtpPacket.ssrc(), result, ignored, false);

      return result;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::setSSRCUsage(
                                   SSRCType ssrc,
                                   String &ioRID,
                                   ChannelInfoPtr &ioChannelInfo,
                                   bool registerUsage
                                   )
    {
      auto found = mSSRCTable.find(ssrc);

      if (found == mSSRCTable.end()) {
        SSRCInfo ssrcInfo;
        if (ioRID.hasData()) {
          ssrcInfo.mRID = ioRID;
        } else if (ioChannelInfo) {
          if (ioChannelInfo->mFilledParams->mEncodingParameters.size() > 0) {
            ioRID = ssrcInfo.mRID = ioChannelInfo->mFilledParams->mEncodingParameters.front().mEncodingID;
          }
        }
        ssrcInfo.mChannelInfo = ioChannelInfo;
        ssrcInfo.mRegisteredUsageCount = (registerUsage ? 1 : 0);
        mSSRCTable[ssrc] = ssrcInfo;
        reattemptDelivery();
        return true;
      }

      SSRCInfo &ssrcInfo = (*found).second;

      ssrcInfo.mLastUsage = zsLib::now();

      if (ioChannelInfo) {
        ssrcInfo.mChannelInfo = ioChannelInfo;
      } else {
        ioChannelInfo = ssrcInfo.mChannelInfo;
      }

      if (ioRID.hasData()) {
        if (ioRID != ssrcInfo.mRID) ssrcInfo.mRID = ioRID;
      } else if (ssrcInfo.mChannelInfo) {
        if (ssrcInfo.mChannelInfo->mFilledParams->mEncodingParameters.size() > 0) {
          if (ssrcInfo.mChannelInfo->mFilledParams->mEncodingParameters.front().mEncodingID.hasData()) {
            if (ssrcInfo.mRID != ssrcInfo.mChannelInfo->mFilledParams->mEncodingParameters.front().mEncodingID) {
              ioRID = ssrcInfo.mRID = ssrcInfo.mChannelInfo->mFilledParams->mEncodingParameters.front().mEncodingID;
            } else {
              ioRID = ssrcInfo.mRID;
            }
          }
        }
      }

      if (registerUsage) {
        ++ssrcInfo.mRegisteredUsageCount;
      }

      return false;
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiver::unregisterSSRCUsage(SSRCType ssrc)
    {
      auto found = mSSRCTable.find(ssrc);
      if (found == mSSRCTable.end()) return;

      auto &ssrcInfo = (*found).second;

      if (ssrcInfo.mRegisteredUsageCount <= 1) {
        ZS_LOG_TRACE(log("removing entry from SSRC table") + ZS_PARAM("ssrc", ssrc) + ssrcInfo.toDebug())

        mSSRCTable.erase(found);
        return;
      }

      --(ssrcInfo.mRegisteredUsageCount);
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiver::reattemptDelivery()
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
