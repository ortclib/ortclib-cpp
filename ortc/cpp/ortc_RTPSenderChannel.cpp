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

#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_RTPSenderChannelMediaBase.h>
#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_RTPTypes.h>
#include <ortc/internal/ortc_ORTC.h>
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


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpsender) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher)
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPSenderChannelSettingsDefaults);

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
    #pragma mark RTPSenderChannelSettingsDefaults
    #pragma mark

    class RTPSenderChannelSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPSenderChannelSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPSenderChannelSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPSenderChannelSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPSenderChannelSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPSenderChannelSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ISettings::setUInt(ORTC_SETTING_RTP_SENDER_CHANNEL_RETAG_RTP_PACKETS_AFTER_SSRC_NOT_SENT_IN_SECONDS, 5);
        ISettings::setBool(ORTC_SETTING_RTP_SENDER_CHANNEL_TAG_MID_RID_IN_RTCP_SDES, true);
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPSenderChannelSettingsDefaults()
    {
      RTPSenderChannelSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderChannelForRTPSender::toDebug(ForRTPSenderPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr IRTPSenderChannelForRTPSender::create(
                                                              RTPSenderPtr sender,
                                                              MediaStreamTrackPtr track,
                                                              const Parameters &params
                                                              )
    {
      return internal::IRTPSenderChannelFactory::singleton().create(sender, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderChannelForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    const char *RTPSenderChannel::toString(States state)
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
    RTPSenderChannel::RTPSenderChannel(
                                       const make_private &,
                                       IMessageQueuePtr queue,
                                       UseSenderPtr sender,
                                       UseMediaStreamTrackPtr track,
                                       const Parameters &params
                                       ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSender(sender),
      mTrack(track),
      mParameters(make_shared<Parameters>(params)),
      mRetagAfterInSeconds(Seconds(ISettings::getUInt(ORTC_SETTING_RTP_SENDER_CHANNEL_RETAG_RTP_PACKETS_AFTER_SSRC_NOT_SENT_IN_SECONDS))),
      mTagSDES(ISettings::getBool(ORTC_SETTING_RTP_SENDER_CHANNEL_TAG_MID_RID_IN_RTCP_SDES))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!sender)

      setupTagging();

      ZS_EVENTING_3(
                    x, i, Detail, RtpSenderChannelCreate, ol, RtpSender, Start,
                    puid, id, mID,
                    puid, senderId, sender->getID(),
                    puid, trackId, ((bool)track) ? track->getID() : 0
                    );
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::init()
    {
      AutoRecursiveLock lock(*this);

      Optional<IMediaStreamTrackTypes::Kinds> kind = RTPTypesHelper::getCodecsKind(*mParameters);
      
      bool found = false;
      
      if (kind.hasValue())
      {
        switch (kind.value()) {
          case IMediaStreamTrackTypes::Kind_Audio:
          {
            mAudio = UseAudio::create(mThisWeak.lock(), MediaStreamTrack::convert(mTrack), *mParameters);
            mMediaBase = mAudio;
            found = true;
            break;
          }
          case IMediaStreamTrackTypes::Kind_Video:
          {
            mVideo = UseVideo::create(mThisWeak.lock(), MediaStreamTrack::convert(mTrack), *mParameters);
            mMediaBase = mVideo;
            found = true;
            break;
          }
        }
      }

      ORTC_THROW_INVALID_PARAMETERS_IF(!found);

      ZS_EVENTING_3(
                    x, i, Detail, RtpSenderChannelCreateMediaChannel, ol, RtpSender, Info,
                    puid, id, mID,
                    puid, mediaBaseId, mMediaBase->getID(),
                    string, kind, IMediaStreamTrack::toString(kind.value())
                    );

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannel::~RTPSenderChannel()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RtpSenderChannelDestroy, ol, RtpSender, Stop, puid, id, mID);
    }


    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForRTPSenderChannelMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForRTPSenderChannelAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForRTPSenderChannelVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => ForRTPSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannel::toDebug(RTPSenderChannelPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::create(
                                                 RTPSenderPtr sender,
                                                 MediaStreamTrackPtr track,
                                                 const Parameters &params
                                                 )
    {
      RTPSenderChannelPtr pThis(make_shared<RTPSenderChannel>(make_private {}, IORTCForInternal::queueORTC(), sender, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyTrackChanged(MediaStreamTrackPtr inTrack)
    {
      UseMediaStreamTrackPtr track = inTrack;
      ZS_EVENTING_3(
                    x, i, Debug, RtpSenderChannelChangeTrack, ol, RtpSenderChannel, InternalEvent,
                    puid, id, mID,
                    puid, mediaBaseId, mMediaBase->getID(),
                    puid, trackId, ((bool)track) ? track->getID() : 0
                    );
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onTrackChanged(track);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyTransportState(ISecureTransportTypes::States state)
    {
      ZS_EVENTING_2(
                    x, i, Debug, RtpSenderChannelInternalSecureTransportStateChangedEvent, ol, RtpSenderChannel, InternalEvent,
                    puid, id, mID,
                    string, state, ISecureTransportTypes::toString(state)
                    );

      // do NOT lock this object here, instead notify self asynchronously
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyPackets(RTCPPacketListPtr packets)
    {
      // do NOT lock this object here, instead notify self asynchronously
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyPackets(packets);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyUpdate(const Parameters &params)
    {
      ZS_EVENTING_1(x, i, Debug, RtpSenderChannelInternalUpdateEvent, ol, RtpSenderChannel, InternalEvent, puid, id, mID);

      // do NOT lock this object here, instead notify self asynchronously
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(make_shared<Parameters>(params));
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      ZS_LOG_TRACE(log("on provide stats") + ZS_PARAM("promise", promise->getID()));

      UseMediaBasePtr mediaBase;

      {
        AutoRecursiveLock lock(*this);
        mediaBase = mMediaBase;
      }

      mediaBase->requestStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::handlePacket(RTCPPacketPtr packet)
    {
      ZS_EVENTING_5(
                    x, i, Trace, RtpSenderChannelDeliverIncomingPacketToMediaChannel, ol, RtpSenderChannel, Deliver,
                    puid, id, mID,
                    puid, mediaBaseId, mMediaBase->getID(),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      if (mIsTagging)
      {
        for (auto rr = packet->firstReceiverReport(); NULL != rr; rr = rr->nextReceiverReport())
        {
          AutoRecursiveLock lock(*this);

          for (auto rb = rr->firstReportBlock(); NULL != rb; rb = rb->next())
          {
            auto ssrc = rb->ssrc();

            auto found = mTaggings.find(ssrc);
            if (found == mTaggings.end()) continue;

            auto &entry = (*found).second;

            if (entry->mReceiverAck) continue;

            DWORD sequenceNumber = (0xFFFF & rb->extendedHighestSequenceNumberReceived());

            DWORD sequenceFirst = entry->mSequenceNumberFirst;
            DWORD sequenceLast = entry->mSequenceNumberLast;

            if (sequenceLast < sequenceFirst) {
              sequenceLast = (0x10000 | sequenceLast);
            }

            if (sequenceNumber < sequenceFirst) {
              sequenceNumber = (0x10000 | sequenceNumber);;
            }

            if ((sequenceNumber >= sequenceFirst) &&
                (sequenceNumber <= sequenceLast)) {
              entry->mReceiverAck = true;
            }
          }
        }
      }

      return mMediaBase->handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::insertDTMF(
                                      const char *tones,
                                      Milliseconds duration,
                                      Milliseconds interToneGap
                                      )
    {
      mMediaBase->insertDTMF(tones, duration, interToneGap);
    }

    //-------------------------------------------------------------------------
    String RTPSenderChannel::toneBuffer() const
    {
      return mMediaBase->toneBuffer();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSenderChannel::duration() const
    {
      return mMediaBase->duration();
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPSenderChannel::interToneGap() const
    {
      return mMediaBase->interToneGap();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => ForRTPSenderChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::sendPacket(RTPPacketPtr packet)
    {
      auto sender = mSender.lock();
      if (!sender) return false;

      if (mIsTagging)
      {
        Time tick = zsLib::now();

        AutoRecursiveLock lock(*this);

        TaggingInfoPtr tagInfo;

        auto found = mTaggings.find(packet->ssrc());
        if (found == mTaggings.end()) {
          tagInfo = make_shared<TaggingInfo>();
          mTaggings[packet->ssrc()] = tagInfo;
        } else {
          tagInfo = (*found).second;
        }

        if (tagInfo->mLastSentPacket + mRetagAfterInSeconds < tick) {
          tagInfo->mReceiverAck = false;
          tagInfo->mSequenceNumberFirst = packet->sequenceNumber();
        }
        tagInfo->mLastSentPacket = tick;
        if (!tagInfo->mReceiverAck) {
          tagInfo->mSequenceNumberLast = packet->sequenceNumber();

          auto oldHeaderExtensions = packet->mHeaderExtensions; // remember old pointer temporarily

          RTPPacket::StringHeaderExtension muxHeader(mMuxHeader ? mMuxHeader->mID : 0, mMuxID.c_str());
          RTPPacket::StringHeaderExtension ridHeader(mRIDHeader ? mRIDHeader->mID : 0, mRID.c_str());

          // Re-point extenion headers in RTP packet to first point to MuxID or
          // RID or both.
          if (mMuxID.hasData()) {
            if (mRID.hasData()) {
              muxHeader.mNext = &ridHeader;
              ridHeader.mNext = oldHeaderExtensions;
              packet->mHeaderExtensions = &muxHeader;
            } else {
              muxHeader.mNext = oldHeaderExtensions;
              packet->mHeaderExtensions = &muxHeader;
            }
          } else {
            ridHeader.mNext = oldHeaderExtensions;
            packet->mHeaderExtensions = &ridHeader;
          }

          RTPPacketPtr newPacket = RTPPacket::create(*packet);

          // Put back old pointer to prevent crash during free of old packet.
          packet->mHeaderExtensions = oldHeaderExtensions;

          // Replace old packet with new packet that will have additional
          // extension headers.
          packet = newPacket;
        }
      }

      ZS_EVENTING_5(
                    x, i, Trace, RtpSenderChannelSendOutgoingPacket, ol, RtpSenderChannel, Send,
                    puid, id, mID,
                    puid, senderId, sender->getID(),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      return sender->sendPacket(packet);
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::sendPacket(RTCPPacketPtr packet)
    {
      auto sender = mSender.lock();
      if (!sender) return false;

      ZS_EVENTING_5(
                    x, i, Trace, RtpSenderChannelSendOutgoingPacket, ol, RtpSenderChannel, Send,
                    puid, id, mID,
                    puid, senderId, sender->getID(),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      if ((mIsTagging) &&
          (mTagSDES))
      {
        String muxID;
        String rid;

        {
          AutoRecursiveLock lock(*this);
          muxID = mMuxID;
          mRID = rid;
        }

        //RTCPPacket::SDES::Chunk::Mid
        RTCPPacket::SDES::Chunk::Mid midItem;
        RTCPPacket::SDES::Chunk::Rid ridItem;

        midItem.mValue = muxID.c_str();
        ridItem.mValue = rid.c_str();

        // Insert the MID/RID SDES entries onto the RTCP packets.
        for (auto sdes = packet->firstSDES(); NULL != sdes; sdes = sdes->nextSDES())
        {
          for (auto chunk = sdes->firstChunk(); NULL != chunk; chunk = chunk->next())
          {
            ASSERT(NULL == chunk->firstMid())
            ASSERT(NULL == chunk->firstRid())
            if (muxID.hasData()) {
              chunk->mMidCount = 1;
              chunk->mFirstMid = &midItem;
            }
            if (rid.hasData()) {
              chunk->mRidCount = 1;
              chunk->mFirstRid = &ridItem;
            }
          }
        }

        RTCPPacketPtr newPacket(RTCPPacket::create(packet->first()));

        // Reset the MID/RID SDES entries to NULL on the RTCP packets to
        // prevent the packet destruction from attempting to free the faked
        // inserted entries.
        for (auto sdes = packet->firstSDES(); NULL != sdes; sdes = sdes->nextSDES())
        {
          for (auto chunk = sdes->firstChunk(); NULL != chunk; chunk = chunk->next())
          {
            chunk->mMidCount = 0;
            chunk->mFirstMid = NULL;
            chunk->mRidCount = 0;
            chunk->mFirstRid = NULL;
          }
        }

        // Replace the old packet with the new packet that contains the
        // additional SDES mid/rid entries.
        packet = newPacket;
      }

      return sender->sendPacket(packet);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => ForRTPSenderChannelAudio
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyDTMFSenderToneChanged(const char *tone)
    {
      auto sender = mSender.lock();
      if (!sender) {
        ZS_LOG_WARNING(Debug, log("cannot forward tone event (sender gone)"));
        return;
      }

      sender->notifyDTMFSenderToneChanged(tone);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => ForRTPSenderChannelVideo
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => IRTPSenderChannelForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    int32_t RTPSenderChannel::sendAudioSamples(
                                               const void* audioSamples,
                                               const size_t numberOfSamples,
                                               const uint8_t numberOfChannels
                                               )
    {
#define TODO_VERIFY_RESULT 1
#define TODO_VERIFY_RESULT 2
      if (!mAudio) return 0;
      return mAudio->sendAudioSamples(audioSamples, numberOfSamples, numberOfChannels);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::sendVideoFrame(VideoFramePtr videoFrame)
    {
      if (!mVideo) return;
      mVideo->sendVideoFrame(videoFrame);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onWake()
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
    #pragma mark RTPSenderChannel => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onTimer(ITimerPtr timer)
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
    #pragma mark RTPSenderChannel => IRTPSenderChannelAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onTrackChanged(UseMediaStreamTrackPtr track)
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase, UseBaseMediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelAudio, UseAudioMediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelVideo, UseVideoMediaStreamTrack)

      switch (mKind.value()) {
        case IMediaStreamTrackTypes::Kind_Audio: {
          mMediaBase->onTrackChanged(IMediaStreamTrackForRTPSenderChannelAudioPtr(MediaStreamTrack::convert(track)));
          break;
        }
        case IMediaStreamTrackTypes::Kind_Video: {
          mMediaBase->onTrackChanged(IMediaStreamTrackForRTPSenderChannelVideoPtr(MediaStreamTrack::convert(track)));
          break;
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onSecureTransportState(ISecureTransport::States state)
    {
      ZS_LOG_TRACE(log("notified secure transport state") + ZS_PARAM("state", ISecureTransport::toString(state)))

      {
        AutoRecursiveLock lock(*this);

        mSecureTransportState = state;

        if (ISecureTransport::State_Closed == state) {
          ZS_LOG_DEBUG(log("secure channel closed (thus shutting down)"))
          cancel();
        }
      }

      mMediaBase->notifyTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onNotifyPackets(RTCPPacketListPtr packets)
    {
      ZS_LOG_TRACE(log("notified rtcp packets") + ZS_PARAM("packets", packets->size()))

      // WARNING: Do NOT modify the contents of "packets" as the pointer to
      //          this list could have been sent to multiple receiver channels
      //          simultaneously. Use COW pattern if needing mutability.

      for (auto iter = packets->begin(); iter != packets->end(); ++iter) {
        auto packet = (*iter);
        handlePacket(packet);
      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onUpdate(ParametersPtr params)
    {
      ZS_LOG_TRACE(log("on update") + params->toDebug())

      UseMediaBasePtr mediaBase;
      
      {
        AutoRecursiveLock lock(*this);

        mParameters = params;
        mediaBase = mMediaBase;

        Optional<IMediaStreamTrackTypes::Kinds> kind = RTPTypesHelper::getCodecsKind(*mParameters);
        
        bool found = false;
        
        if (kind.hasValue())
        {
          switch (kind.value()) {
            case IMediaStreamTrackTypes::Kind_Audio:
            {
              found = (bool)mAudio;
              break;
            }
            case IMediaStreamTrackTypes::Kind_Video:
            {
              found = (bool)mVideo;
              break;
            }
          }
        }

        ORTC_THROW_INVALID_PARAMETERS_IF(!found)

        setupTagging();
      }
      
      mediaBase->notifyUpdate(params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannel::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSenderChannel");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannel::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannel::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSenderChannel");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      IHelper::debugAppend(resultEl, "secure transport state", ISecureTransport::toString(mSecureTransportState));

      auto sender = mSender.lock();
      IHelper::debugAppend(resultEl, "sender", sender ? sender->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::step()
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
    void RTPSenderChannel::cancel()
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

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderChannelPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderChannelStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::setError(WORD errorCode, const char *inReason)
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
    void RTPSenderChannel::setupTagging()
    {
      mMuxHeader.reset();
      mRIDHeader.reset();

      for (auto iter = mParameters->mHeaderExtensions.begin(); iter != mParameters->mHeaderExtensions.end(); ++iter) {
        auto &ext = *(iter);
        switch (IRTPTypes::toHeaderExtensionURI(ext.mURI))
        {
          case IRTPTypes::HeaderExtensionURI_MuxID:
          {
            mMuxHeader = make_shared<HeaderExtensionParameters>(ext);
            break;
          }
          case IRTPTypes::HeaderExtensionURI_RID:
          {
            mRIDHeader = make_shared<HeaderExtensionParameters>(ext);
            break;
          }
          default:
          {
            break;
          }
        }
      }

      // The MuxID / RID can only be set if the header extensiosn have been
      // defined in the parameters.
      mMuxID.clear();
      mRID.clear();

      if (mMuxHeader) {
        mMuxID = mParameters->mMuxID;
      }
      if (mRIDHeader) {
        if (mParameters->mEncodings.size() > 0) {
          auto &encoding = (mParameters->mEncodings.front());
          mRID = encoding.mEncodingID;
        }
      }

      // Check to see if the header options have changed since last update.
      // If any changes have occured then the streams that pass through
      // this mapping must be retagged with the MuxID / RID.
      {
        auto hasher = IHasher::sha1();
        if (mMuxHeader) {
          hasher->update(mMuxHeader->hash());
        }
        hasher->update(":");
        if (mRIDHeader) {
          hasher->update(mRIDHeader->hash());
        }
        hasher->update(":");
        hasher->update(mMuxID);
        hasher->update(":");
        hasher->update(mRID);

        String hashResult = hasher->finalizeAsString();
        if (hashResult != mHeaderHash) {
          mTaggings.clear();
        }
        mHeaderHash = hashResult;
      }

      // Set flag to do tagging if there is a mux id or a rid set.
      mIsTagging = mMuxID.hasData() || mRID.hasData();
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderChannelFactory &IRTPSenderChannelFactory::singleton()
    {
      return RTPSenderChannelFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr IRTPSenderChannelFactory::create(
                                                         RTPSenderPtr sender,
                                                         MediaStreamTrackPtr track,
                                                         const Parameters &params
                                                         )
    {
      if (this) {}
      return internal::RTPSenderChannel::create(sender, track, params);
    }

  } // internal namespace

}
