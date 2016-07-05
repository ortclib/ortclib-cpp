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
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_SRTPSDESTransport.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_RTPTypes.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc_Tracing.h>
#include <ortc/internal/platform.h>

#include <ortc/IRTPReceiver.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>


#ifdef _DEBUG
#ifdef _WIN32
#define ASSERT(x) {assert(x);}
#else
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#endif //_WIN32
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpsender) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

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
    ElementPtr IRTPSenderForRTPListener::toDebug(ForRTPListenerPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderForRTPSenderChannel::toDebug(ForRTPSenderChannelPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForDTMFSender
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
    #pragma mark RTPReceiver::ChannelHolder
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSender::ChannelHolder::ChannelHolder()
    {
    }

    //-------------------------------------------------------------------------
    RTPSender::ChannelHolder::~ChannelHolder()
    {
      notify(ISecureTransport::State_Closed);

      ASSERT((bool)mChannel)

      auto outer = mHolder.lock();
      if (outer) {
        outer->notifyChannelGone();
      }
    }

    //-------------------------------------------------------------------------
    PUID RTPSender::ChannelHolder::getID() const
    {
      return mChannel->getID();
    }

    //-------------------------------------------------------------------------
    void RTPSender::ChannelHolder::notify(ISecureTransport::States state)
    {
      if (state == mLastReportedState) return;

      mLastReportedState = state;
      mChannel->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSender::ChannelHolder::notify(RTCPPacketListPtr packets)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return;
      mChannel->notifyPackets(packets);
    }

    //-------------------------------------------------------------------------
    void RTPSender::ChannelHolder::update(const Parameters &params)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return;
      mChannel->notifyUpdate(params);
    }

    //-------------------------------------------------------------------------
    bool RTPSender::ChannelHolder::handle(RTCPPacketPtr packet)
    {
      if (ISecureTransport::State_Closed == mLastReportedState) return false;
      return mChannel->handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPSender::ChannelHolder::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      mChannel->requestStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    void RTPSender::ChannelHolder::insertDTMF(
                                              const char *tones,
                                              Milliseconds duration,
                                              Milliseconds interToneGap
                                              )
    {
      mChannel->insertDTMF(tones, duration, interToneGap);
    }

    //-------------------------------------------------------------------------
    String RTPSender::ChannelHolder::toneBuffer() const
    {
      return mChannel->toneBuffer();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSender::ChannelHolder::duration() const
    {
      return mChannel->duration();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSender::ChannelHolder::interToneGap() const
    {
      return mChannel->interToneGap();
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSender::ChannelHolder::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPSender::ChannelHolder");

      auto outer = mHolder.lock();
      UseServicesHelper::debugAppend(resultEl, "outer", outer ? outer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "channel", mChannel ? mChannel->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "last reported state", ISecureTransport::toString(mLastReportedState));
      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender
    #pragma mark
    
    //-------------------------------------------------------------------------
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
      mChannels(make_shared<ParametersToChannelHolderMap>()),
      mTrack(MediaStreamTrack::convert(track))
    {
      ZS_LOG_DETAIL(debug("created"))

      mListener = UseListener::getListener(transport);
      ORTC_THROW_INVALID_STATE_IF(!mListener)

      UseSecureTransport::getSendingTransport(transport, rtcpTransport, mSendRTPOverTransport, mSendRTCPOverTransport, mRTPTransport, mRTCPTransport);

      EventWriteOrtcRtpSenderCreate(
                                    __func__,
                                    mID,
                                    ((bool)mTrack) ? mTrack->getID() : 0,
                                    ((bool)mListener) ? mListener->getID() : 0,
                                    ((bool)mRTPTransport) ? mRTPTransport->getID() : 0,
                                    ((bool)mRTCPTransport) ? mRTPTransport->getID() : 0
                                    );
    }

    //-------------------------------------------------------------------------
    void RTPSender::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      mRTCPTransportSubscription = mRTCPTransport->subscribe(mThisWeak.lock());
    }

    //-------------------------------------------------------------------------
    RTPSender::~RTPSender()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      EventWriteOrtcRtpSenderDestroy(__func__, mID);
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
    RTPSenderPtr RTPSender::convert(ForRTPSenderChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(ForDTMFSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSender, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(ForMediaStreamTrackPtr object)
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
    IStatsProvider::PromiseWithStatsReportPtr RTPSender::getStats(const StatsTypeSet &stats) const
    {
      if (!stats.hasStatType(IStatsReportTypes::StatsType_OutboundRTP)) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      ParametersToChannelHolderMapPtr channels;

      UseStatsReport::PromiseWithStatsReportList promises;

      {
        AutoRecursiveLock lock(*this);
        channels = mChannels; // obtain pointer to COW list while inside a lock
      }

      bool result = false;
      for (auto iter = channels->begin(); iter != channels->end(); ++iter)
      {
        auto channel = (*iter).second;

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

        ASSERT((bool)pThis)

        for (auto iter = mConflicts.begin(); iter != mConflicts.end(); ++iter) {
          auto &ssrc = (*iter);

          delegate->onRTPSenderSSRCConflict(pThis, ssrc);
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RTPSender::track() const
    {
      AutoRecursiveLock lock(*this);
      return MediaStreamTrack::convert(mTrack);
    }

    //-------------------------------------------------------------------------
    IRTPTransportPtr RTPSender::transport() const
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
    IRTCPTransportPtr RTPSender::rtcpTransport() const
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
    void RTPSender::setTransport(
                                 IRTPTransportPtr transport,
                                 IRTCPTransportPtr rtcpTransport
                                 )
    {
      typedef UseListener::RTCPPacketList RTCPPacketList;

      AutoRecursiveLock lock(*this);

      auto listener = UseListener::getListener(transport);
      ORTC_THROW_INVALID_STATE_IF(!listener)

      if (mParameters) {
        if (listener->getID() != mListener->getID()) {
          // unregister from previous transport
          mListener->unregisterSender(*this);

          mListener = listener;

          // register with new transport
          RTCPPacketList historicalRTCPPackets;
          mListener->registerSender(mThisWeak.lock(), *mParameters, historicalRTCPPackets);

          if (historicalRTCPPackets.size() > 0) {
            RTCPPacketListPtr notifyPackets;

            for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter)
            {
              auto channel = (*iter).second;

              if (!notifyPackets) {
                notifyPackets = make_shared<RTCPPacketList>(historicalRTCPPackets);
              }

              channel->notify(notifyPackets);
            }
          }
        }
      }

      UseSecureTransport::getSendingTransport(transport, rtcpTransport, mSendRTPOverTransport, mSendRTCPOverTransport, mRTPTransport, mRTCPTransport);

      EventWriteOrtcRtpSenderSetTransport(__func__, mID, ((bool)mListener) ? mListener->getID() : 0, ((bool)mRTPTransport) ? mRTPTransport->getID() : 0, ((bool)mRTCPTransport) ? mRTCPTransport->getID() : 0);

      if (mRTCPTransportSubscription) {
        mRTCPTransportSubscription->cancel();
        mRTCPTransportSubscription.reset();
      }

      mRTCPTransportSubscription = mRTCPTransport->subscribe(mThisWeak.lock());

      notifyChannelsOfTransportState();
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPSender::setTrack(IMediaStreamTrackPtr inTrack)
    {
      UseMediaStreamTrackPtr track = MediaStreamTrack::convert(inTrack);

      Kinds kind = (track ? track->kind() : IMediaStreamTrackTypes::Kind_Audio);

      EventWriteOrtcRtpSenderSetTrack(__func__, mID, ((bool)track) ? track->getID() : 0, ((bool)track) ? IMediaStreamTrackTypes::toString(kind) : NULL);

      {
        AutoRecursiveLock lock(*this);

        if (mKind.hasValue()) {
          if (track) {
            if (mKind.value() != kind) {
              return Promise::createRejected(make_shared<IncompatibleMediaStreamTrackError>(), IORTCForInternal::queueDelegate());
            }
          }
        } else {
          if (track) {
            mKind = track->kind();
          }
        }

        if (mTrack) {
          if (track) {
            if (track->getID() == mTrack->getID()) {
              ZS_LOG_DEBUG(log("setting track to same track (noop)") + ZS_PARAM("track id", track->getID()))
              return Promise::createResolved(IORTCForInternal::queueDelegate());
            }
          }
          for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
            auto &channel = (*iter).second;
            EventWriteOrtcRtpSenderDetachTrackSenderChannel(__func__, mID, mTrack->getID(), channel->getID());
            mTrack->notifyDetachSenderChannel(RTPSenderChannel::convert(channel->mChannel));
          }
        }

        mTrack = track;

        if (mTrack) {
          for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
            auto &channel = (*iter).second;
            EventWriteOrtcRtpSenderAttachTrackSenderChannel(__func__, mID, mTrack->getID(), channel->getID());
            mTrack->notifyAttachSenderChannel(RTPSenderChannel::convert(channel->mChannel));
          }
        }

      }

      return Promise::createResolved(IORTCForInternal::queueDelegate());
    }

    //-------------------------------------------------------------------------
    IRTPSenderTypes::CapabilitiesPtr RTPSender::getCapabilities(Optional<Kinds> kind)
    {
      return IRTPReceiver::getCapabilities(kind);
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPSender::send(const Parameters &parameters)
    {
      typedef RTPTypesHelper::ParametersPtrPairList ParametersPtrPairList;

      PromisePtr promise = Promise::create(IORTCForInternal::queueDelegate());

      EventWriteOrtcRtpSenderSend(__func__, mID);

      ZS_LOG_DEBUG(log("send called") + parameters.toDebug())

      AutoRecursiveLock lock(*this);

      Optional<IMediaStreamTrackTypes::Kinds> foundKind;
      RTPTypesHelper::validateCodecParameters(parameters, foundKind);

      ORTC_THROW_INVALID_PARAMETERS_IF(!foundKind.hasValue()) // only one media kind is allowed to be specified to a sender

      if (mKind.hasValue()) {
        ORTC_THROW_INVALID_PARAMETERS_IF(mKind.value() != foundKind.value())  // not allowed to change codec kind
      } else {
        mKind = foundKind;
      }

      if (mParameters) {
        auto hash = parameters.hash();
        auto previousHash = mParameters->hash();
        if (hash == previousHash) {
          ZS_LOG_TRACE(log("send parameters have not changed (noop)") + parameters.toDebug())
          promise->resolve();
          return promise;
        }

        ParametersPtrList oldGroupedParams = mParametersGroupedIntoChannels;

        mParameters = make_shared<Parameters>(parameters);

        mParametersGroupedIntoChannels.clear();
        RTPTypesHelper::splitParamsIntoChannels(parameters, mParametersGroupedIntoChannels);

        ParametersToChannelHolderMapPtr replacementChannels = make_shared<ParametersToChannelHolderMap>(*mChannels);

        ParametersPtrPairList unchangedChannels;
        ParametersPtrList newChannels;
        ParametersPtrPairList updateChannels;
        ParametersPtrList removeChannels;

        RTPTypesHelper::calculateDeltaChangesInChannels(mKind, oldGroupedParams, mParametersGroupedIntoChannels, unchangedChannels, newChannels, updateChannels, removeChannels);

        // scope: remove dead channels
        {
          for (auto iter = removeChannels.begin(); iter != removeChannels.end(); ++iter) {
            auto &params = (*iter);
            auto found = replacementChannels->find(params);

            if (found == replacementChannels->end()) continue;  // possible it will not be found (when channel self destructs)

            auto &channel = (*found).second;

            removeChannel(channel);
            replacementChannels->erase(found);
          }
        }

        // scope: swap out new / old parameters
        {
          for (auto iter = unchangedChannels.begin(); iter != unchangedChannels.end(); ++iter) {
            auto &pairInfo = (*iter);
            auto &oldParams = pairInfo.first;
            auto &newParams = pairInfo.second;
            auto found = replacementChannels->find(oldParams);

            if (found == replacementChannels->end()) continue;  // possible it will not be found (when channel self destructs)

            auto channel = (*found).second;

            replacementChannels->erase(found);
            (*replacementChannels)[newParams] = channel;
          }
        }

        // scope: update existing channels
        {
          for (auto iter = updateChannels.begin(); iter != updateChannels.end(); ++iter) {
            auto &pairInfo = (*iter);
            auto &oldParams = pairInfo.first;
            auto &newParams = pairInfo.second;
            auto found = replacementChannels->find(oldParams);

            if (found == replacementChannels->end()) continue;  // possible it will not be found (when channel self destructs)

            auto channel = (*found).second;

            replacementChannels->erase(found);
            (*replacementChannels)[newParams] = channel;

            updateChannel(channel, newParams);
          }
        }

        // scope: add new channels
        {
          for (auto iter = newChannels.begin(); iter != newChannels.end(); ++iter) {
            auto &newParams = (*iter);
            auto channel = addChannel(newParams);

            (*replacementChannels)[newParams] = channel;
          }
        }

        mChannels = replacementChannels;  // COW replacement
      } else {
        mParameters = make_shared<Parameters>(parameters);

        RTPTypesHelper::splitParamsIntoChannels(parameters, mParametersGroupedIntoChannels);

        ParametersToChannelHolderMapPtr replacementChannels = make_shared<ParametersToChannelHolderMap>();

        for (auto iter = mParametersGroupedIntoChannels.begin(); iter != mParametersGroupedIntoChannels.end(); ++iter) {
          auto &params = (*iter);
          auto channel = addChannel(params);

          (*replacementChannels)[params] = channel;
        }
        mChannels = replacementChannels;
      }

      mParameters = make_shared<Parameters>(parameters);

      RTCPPacketList historicalRTCPPackets;
      mListener->registerSender(mThisWeak.lock(), *mParameters, historicalRTCPPackets);

      promise->resolve();
      return promise;
    }

    //-------------------------------------------------------------------------
    void RTPSender::stop()
    {
      EventWriteOrtcRtpSenderStop(__func__, mID);

      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderForRTPListener
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSender::handlePacket(
                                 IICETypes::Components viaTransport,
                                 RTCPPacketPtr packet
                                 )
    {
      EventWriteOrtcRtpSenderIncomingPacket(__func__, mID, zsLib::to_underlying(viaTransport), zsLib::to_underlying(IICETypes::Component_RTCP), SafeInt<unsigned int>(packet->buffer()->SizeInBytes()), packet->buffer()->BytePtr());

      ZS_LOG_TRACE(log("received packet") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + packet->toDebug())

      ParametersToChannelHolderMapPtr channels;

      {
        AutoRecursiveLock lock(*this);
        channels = mChannels; // obtain pointer to COW list while inside a lock
      }

      bool result = false;
      for (auto iter = channels->begin(); iter != channels->end(); ++iter)
      {
        auto channel = (*iter).second;

        EventWriteOrtcRtpSenderDeliverIncomingPacketToChannel(__func__, mID, channel->getID(), zsLib::to_underlying(viaTransport), zsLib::to_underlying(IICETypes::Component_RTCP), SafeInt<unsigned int>(packet->buffer()->SizeInBytes()), packet->buffer()->BytePtr());

        auto channelResult = channel->handle(packet);
        result = result || channelResult;
      }

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSender::sendPacket(RTPPacketPtr packet)
    {
      UseSecureTransportPtr rtpTransport;

      {
        AutoRecursiveLock lock(*this);

        if (isShutdown()) {
          ZS_LOG_WARNING(Debug, log("cannot send packet while shutdown"))
          return false;
        }

        rtpTransport = mRTPTransport;
      }

      if (!rtpTransport) {
        ZS_LOG_WARNING(Debug, log("no rtp transport is currently attached (thus discarding sent packet)"))
        return false;
      }

      ZS_LOG_TRACE(log("sending rtp packet over secure transport") + ZS_PARAM("size", packet->size()))

      EventWriteOrtcRtpSenderSendOutgoingPacket(__func__, mID, zsLib::to_underlying(mSendRTPOverTransport), zsLib::to_underlying(IICETypes::Component_RTP), SafeInt<unsigned int>(packet->buffer()->SizeInBytes()), packet->buffer()->BytePtr());

      return rtpTransport->sendPacket(mSendRTPOverTransport, IICETypes::Component_RTP, packet->ptr(), packet->size());
    }

    //-------------------------------------------------------------------------
    bool RTPSender::sendPacket(RTCPPacketPtr packet)
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

      ZS_LOG_TRACE(log("sending rtcp packet over secure transport") + ZS_PARAM("size", packet->size()))

      EventWriteOrtcRtpSenderSendOutgoingPacket(__func__, mID, zsLib::to_underlying(mSendRTCPOverTransport), zsLib::to_underlying(IICETypes::Component_RTCP), SafeInt<unsigned int>(packet->buffer()->SizeInBytes()), packet->buffer()->BytePtr());

      return rtcpTransport->sendPacket(mSendRTCPOverTransport, IICETypes::Component_RTCP, packet->ptr(), packet->size());
    }

    //-------------------------------------------------------------------------
    void RTPSender::notifyConflict(
                                   UseChannelPtr channel,
                                   IRTPTypes::SSRCType ssrc,
                                   bool selfDestruct
                                   )
    {
      ZS_LOG_DEBUG(log("notify ssrc conflict") + ZS_PARAM("channel", channel ? channel->getID() : 0) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("self destruct", selfDestruct))

      AutoRecursiveLock lock(*this);

      bool found = false;
      for (auto iter = mConflicts.begin(); iter != mConflicts.end(); ++iter) {
        auto &existingSSRC = (*iter);
        if (existingSSRC == ssrc) {
          ZS_LOG_WARNING(Detail, log("conflict already registered") + ZS_PARAM("ssrc", ssrc))
          found = true;
          break;
        }
      }

      if (!found) {
        mConflicts.push_back(ssrc);
      }

      EventWriteOrtcRtpSenderSsrcConflictEventFired(__func__, mID, ((bool)channel) ? channel->getID() : 0, ssrc, selfDestruct);

      mSubscriptions.delegate()->onRTPSenderSSRCConflict(mThisWeak.lock(), ssrc);

      if ((selfDestruct) &&
          (channel)) {
        IRTPSenderAsyncDelegateProxy::create(mThisWeak.lock())->onDestroyChannel(channel);
      }
    }

    //-------------------------------------------------------------------------
    void RTPSender::notifyDTMFSenderToneChanged(const char *tone)
    {
      AutoRecursiveLock lock(*this);

      mDTMFSubscriptions.delegate()->onDTMFSenderToneChanged(mThisWeak.lock(), String(tone));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderForDTMFSender
    #pragma mark

    //-------------------------------------------------------------------------
    IDTMFSenderSubscriptionPtr RTPSender::subscribeDTMF(IDTMFSenderDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to dtmf"));

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return IDTMFSenderSubscriptionPtr();

      IDTMFSenderSubscriptionPtr subscription = mDTMFSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IDTMFSenderDelegatePtr delegate = mDTMFSubscriptions.delegate(subscription, true);

      if (delegate) {
        //DTMFSenderPtr pThis = mThisWeak.lock();

      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    bool RTPSender::canInsertDTMF() const
    {
      AutoRecursiveLock lock(*this);

      if (!mKind.hasValue()) {
        ZS_LOG_TRACE(log("kind is not set (thus cannot insert DTMF tone)"));
        return false;
      }

      if (IMediaStreamTrackTypes::Kind_Audio != mKind.value()) {
        ZS_LOG_TRACE(log("kind is not audio (thus cannot insert DTMF tone)"));
        return false;
      }

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_TRACE(log("cannot insert tone while shutting down / shutdown"));
        return false;
      }

      if (!mParameters) {
        ZS_LOG_TRACE(log("parameters are not set (thus cannot insert DTMF tone)"));
        return false;
      }

      RTPTypesHelper::FindCodecOptions options;
      options.mSupportedCodec = IRTPTypes::SupportedCodec_TelephoneEvent;

      auto result = RTPTypesHelper::findCodec(*mParameters, options);

      if (NULL == result) {
        ZS_LOG_TRACE(log("could not find telephone event codec (thus cannot insert DTMF tone)"));
        return false;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSender::insertDTMF(
                               const char *tones,
                               Milliseconds duration,
                               Milliseconds interToneGap
                               ) throw (
                               InvalidStateError,
                               InvalidCharacterError
                               )
    {
      ZS_THROW_CUSTOM_IF(InvalidCharacterError, NULL == tones);
      ZS_THROW_CUSTOM_IF(InvalidCharacterError, '\0' == *tones);

      for (const char *pos = tones; '\0' != *pos; ++pos)
      {
        switch (*pos)
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case '*':
          case '#': break;
          default: {
            ZS_THROW_CUSTOM(InvalidCharacterError, (String("tone(s) specified are not valid: ") + tones));
          }
        }
      }

      ChannelHolderPtr holder;

      {
        AutoRecursiveLock lock(*this);
        ORTC_THROW_INVALID_STATE_IF(!canInsertDTMF());

        holder = getDTMFChannelHolder();
      }

      ORTC_THROW_INVALID_STATE_IF(!holder);

      holder->insertDTMF(tones, duration, interToneGap);
    }

    //-------------------------------------------------------------------------
    IRTPSenderPtr RTPSender::sender() const
    {
      return mThisWeak.lock();
    }

    //-------------------------------------------------------------------------
    String RTPSender::toneBuffer() const
    {
      ChannelHolderPtr holder;

      {
        AutoRecursiveLock lock(*this);
        holder = getDTMFChannelHolder();
      }

      if (!holder) return String();

      return holder->toneBuffer();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSender::duration() const
    {
      ChannelHolderPtr holder;

      {
        AutoRecursiveLock lock(*this);
        holder = getDTMFChannelHolder();
      }

      if (!holder) return Milliseconds();

      return holder->duration();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSender::interToneGap() const
    {
      ChannelHolderPtr holder;

      {
        AutoRecursiveLock lock(*this);
        holder = getDTMFChannelHolder();
      }

      if (!holder) return Milliseconds();

      return holder->interToneGap();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => ISecureTransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onSecureTransportStateChanged(
                                                  ISecureTransportPtr transport,
                                                  ISecureTransport::States state
                                                  )
    {
      EventWriteOrtcRtpSenderInternalSecureTransportStateChangedEventFired(__func__, mID, transport->getID(), ISecureTransportTypes::toString(state));

      ZS_LOG_DEBUG(log("on secure transport state changed") + ZS_PARAM("secure transport", transport->getID()) + ZS_PARAM("state", ISecureTransportTypes::toString(state)))

      AutoRecursiveLock lock(*this);
      notifyChannelsOfTransportState();
    }

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
      EventWriteOrtcRtpSenderInternalWakeEventFired(__func__, mID);

      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IDTMFSenderDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onDTMFSenderToneChanged(
                                            IDTMFSenderPtr sender,
                                            String tone
                                            )
    {
      ZS_LOG_DEBUG(log("forwarding DTMF tone event") + ZS_PARAM("tone", tone));

      AutoRecursiveLock(*this);
      mDTMFSubscriptions.delegate()->onDTMFSenderToneChanged(mThisWeak.lock(), tone);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSenderAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onDestroyChannel(UseChannelPtr channel)
    {
      EventWriteOrtcRtpSenderInternalDestroyChannelEventFired(__func__, mID, channel->getID());

      ZS_LOG_TRACE(log("on destroy channel") + ZS_PARAM("channel", channel->getID()))

      AutoRecursiveLock lock(*this);

      ParametersToChannelHolderMapPtr replacementChannels(make_shared<ParametersToChannelHolderMap>(*mChannels));

      for (auto iter_doNotUse = replacementChannels->begin(); iter_doNotUse != replacementChannels->end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &params = (*current).first;
        auto &existingChannel = (*current).second;
        if (channel->getID() != existingChannel->getID()) continue;

        ZS_LOG_DEBUG(log("destroying channel") + ZS_PARAM("channel id", channel->getID()) + ZS_PARAM("channel params", params->toDebug()))

        existingChannel->notify(ISecureTransportTypes::State_Closed);

        if (mTrack) {
          EventWriteOrtcRtpSenderDetachTrackSenderChannel(__func__, mID, mTrack->getID(), existingChannel->getID());
          mTrack->notifyDetachSenderChannel(RTPSenderChannel::convert(existingChannel->mChannel));
        }

        replacementChannels->erase(current);
        break;
      }

      mChannels = replacementChannels;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => (friend ChannelHolder)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::notifyChannelGone()
    {
      EventWriteOrtcRtpSenderInternalChannelGoneEventFired(__func__, mID);
      // nothing to do
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

      UseServicesHelper::debugAppend(resultEl, "parameters", mParameters ? mParameters->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "parameter grouped into channels", mParametersGroupedIntoChannels.size());

      UseServicesHelper::debugAppend(resultEl, "listener", mListener ? mListener->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "rtp transport", mRTPTransport ? mRTPTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "rtcp transport", mRTCPTransport ? mRTCPTransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "rtcp transport subscription", mRTCPTransportSubscription ? mRTCPTransportSubscription->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "send rtp over transport", IICETypes::toString(mSendRTPOverTransport));
      UseServicesHelper::debugAppend(resultEl, "send rtcp over transport", IICETypes::toString(mSendRTCPOverTransport));
      UseServicesHelper::debugAppend(resultEl, "receive rtcp over transport", IICETypes::toString(mReceiveRTCPOverTransport));

      UseServicesHelper::debugAppend(resultEl, "last reported transport state to channels", ISecureTransportTypes::toString(mLastReportedTransportStateToChannels));

      UseServicesHelper::debugAppend(resultEl, "kind", mKind.hasValue() ? IMediaStreamTrackTypes::toString(mKind.value()) : (const char *)NULL);
      UseServicesHelper::debugAppend(resultEl, "track", mTrack ? mTrack->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "channels", mChannels->size());

      UseServicesHelper::debugAppend(resultEl, "conflicts", mConflicts.size());

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
      // ... other steps here ...

      goto ready;

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    void RTPSender::cancel()
    {
      EventWriteOrtcRtpSenderCancel(__func__, mID);

      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
        // return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      for (auto iter = mChannels->begin(); iter != mChannels->end(); ++iter) {
        auto channel = (*iter).second;
        if (!channel) continue;

        removeChannel(channel);
      }

      ParametersToChannelHolderMapPtr channels = ParametersToChannelHolderMapPtr(make_shared<ParametersToChannelHolderMap>());
      mChannels = channels;

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      if (mRTCPTransportSubscription) {
        mRTCPTransportSubscription->cancel();
        mRTCPTransportSubscription.reset();
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
      EventWriteOrtcRtpSenderStateChangedEventFired(__func__, mID, toString(state));

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
      EventWriteOrtcRtpSenderErrorEventFired(__func__, mID, errorCode, reason);

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    RTPSender::ChannelHolderPtr RTPSender::addChannel(ParametersPtr newParams)
    {
      ChannelHolderPtr channel(make_shared<ChannelHolder>());
      channel->mHolder = mThisWeak.lock();
      channel->mChannel = UseChannel::create(mThisWeak.lock(), MediaStreamTrack::convert(mTrack), *newParams);
      channel->notify(mLastReportedTransportStateToChannels);
      if (mTrack) {
        EventWriteOrtcRtpSenderAttachTrackSenderChannel(__func__, mID, mTrack->getID(), channel->getID());
        mTrack->notifyAttachSenderChannel(RTPSenderChannel::convert(channel->mChannel));
      }
      EventWriteOrtcRtpSenderAddChannel(__func__, mID, channel->getID());
      return channel;
    }

    //-------------------------------------------------------------------------
    void RTPSender::updateChannel(
                                  ChannelHolderPtr channel,
                                  ParametersPtr newParams
                                  )
    {
      EventWriteOrtcRtpSenderUpdateChannel(__func__, mID, channel->getID());
      channel->update(*newParams);
    }

    //-------------------------------------------------------------------------
    void RTPSender::removeChannel(ChannelHolderPtr channel)
    {
      EventWriteOrtcRtpSenderRemoveChannel(__func__, mID, channel->getID());

      channel->notify(ISecureTransport::State_Closed);
      if (mTrack) {
        EventWriteOrtcRtpSenderDetachTrackSenderChannel(__func__, mID, mTrack->getID(), channel->getID());
        mTrack->notifyDetachSenderChannel(RTPSenderChannel::convert(channel->mChannel));
      }
    }

    //-------------------------------------------------------------------------
    void RTPSender::notifyChannelsOfTransportState()
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

        channel->notify(mLastReportedTransportStateToChannels);
      }
    }

    //-------------------------------------------------------------------------
    RTPSender::ChannelHolderPtr RTPSender::getDTMFChannelHolder() const
    {
      if (mParametersGroupedIntoChannels.size() < 1) return ChannelHolderPtr();

      auto params = mParametersGroupedIntoChannels.front();

      ZS_THROW_BAD_STATE_IF(!params);
      ZS_THROW_BAD_STATE_IF(!mChannels);

      auto found = mChannels->find(params);
      if (found == mChannels->end()) return ChannelHolderPtr();

      return (*found).second;
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
  #pragma mark IRTPSenderTypes
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPSenderTypes::IncompatibleMediaStreamTrackErrorPtr IRTPSenderTypes::IncompatibleMediaStreamTrackError::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(IncompatibleMediaStreamTrackError, any);
  }

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
