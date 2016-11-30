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

#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_RTPReceiverChannelMediaBase.h>
#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_DTLSTransport.h>
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
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPReceiverChannelSettingsDefaults);

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
    #pragma mark RTPReceiverChannelSettingsDefaults
    #pragma mark

    class RTPReceiverChannelSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPReceiverChannelSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPReceiverChannelSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPReceiverChannelSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPReceiverChannelSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPReceiverChannelSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        //      ISettings::setUInt(ORTC_SETTING_RTP_RECEIVER_CHANNEL_, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPReceiverChannelSettingsDefaults()
    {
      RTPReceiverChannelSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelForRTPReceiver::toDebug(ForRTPReceiverPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr IRTPReceiverChannelForRTPReceiver::create(
                                                                    RTPReceiverPtr receiver,
                                                                    MediaStreamTrackPtr track,
                                                                    const Parameters &params,
                                                                    const RTCPPacketList &packets
                                                                    )
    {
      return internal::IRTPReceiverChannelFactory::singleton().create(receiver, track, params, packets);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object)->toDebug();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiverChannel::toString(States state)
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
    RTPReceiverChannel::RTPReceiverChannel(
                                           const make_private &,
                                           IMessageQueuePtr queue,
                                           UseReceiverPtr receiver,
                                           UseMediaStreamTrackPtr track,
                                           const Parameters &params
                                           ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mReceiver(receiver),
      mTrack(track),
      mParameters(make_shared<Parameters>(params))
    {
      ZS_EVENTING_3(
                    x, i, Detail, RtpReceiverChannelCreate, ol, RtpReceiverChannel, Start,
                    puid, id, mID,
                    puid, receiverId, receiver->getID(),
                    puid, trackId, mTrack->getID()
                    );
      ZS_LOG_DETAIL(debug("created"));

      ORTC_THROW_INVALID_PARAMETERS_IF(!receiver);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::init(const RTCPPacketList &packets)
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
                    x, i, Detail, RtpReceiverChannelCreateMediaChannel, ol, RtpReceiverChannel, Info,
                    puid, id, mID,
                    puid, mediaBaseId, mMediaBase->getID(),
                    string, kind, IMediaStreamTrack::toString(kind.value())
                    );

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannel::~RTPReceiverChannel()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RtpReceiverChannelDestroy, ol, RtpReceiverChannel, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr RTPReceiverChannel::convert(ForRTPReceiverPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr RTPReceiverChannel::convert(ForRTPReceiverChannelMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr RTPReceiverChannel::convert(ForRTPReceiverChannelAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr RTPReceiverChannel::convert(ForRTPReceiverChannelVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr RTPReceiverChannel::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannel, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => ForRTPReceiver
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPReceiverChannel::toDebug(RTPReceiverChannelPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr RTPReceiverChannel::create(
                                                     RTPReceiverPtr receiver,
                                                     MediaStreamTrackPtr track,
                                                     const Parameters &params,
                                                     const RTCPPacketList &packets
                                                     )
    {
      RTPReceiverChannelPtr pThis(make_shared<RTPReceiverChannel>(make_private {}, IORTCForInternal::queueORTC(), receiver, track, params));
      pThis->mThisWeak = pThis;
      pThis->init(packets);
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::notifyTransportState(ISecureTransport::States state)
    {
      ZS_EVENTING_2(x, i, Detail, RtpReceiverChannelInternalSecureTransportStateChangedEvent, ol, RtpReceiverChannel, InternalEvent, puid, id, mID, string, state, ISecureTransportTypes::toString(state));

      // do NOT lock this object here, instead notify self asynchronously
      IRTPReceiverChannelAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::notifyPacket(RTPPacketPtr packet)
    {
      // do NOT lock this object here, instead notify self asynchronously
      IRTPReceiverChannelAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyPacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::notifyPackets(RTCPPacketListPtr packets)
    {
      // do NOT lock this object here, instead notify self asynchronously
      IRTPReceiverChannelAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyPackets(packets);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::notifyUpdate(const Parameters &params)
    {
      ZS_EVENTING_1(x, i, Detail, RtpReceiverChannelInternalUpdateEvent, ol, RtpReceiverChannel, InternalEvent, puid, id, mID);

      // do NOT lock this object here, instead notify self asynchronously
      IRTPReceiverChannelAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(make_shared<Parameters>(params));
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannel::handlePacket(RTPPacketPtr packet)
    {
      ZS_EVENTING_5(
                    x, i, Trace, RtpReceiverChannelDeliverIncomingPacketToMediaChannel, ol, RtpReceiverChannel, Deliver,
                    puid, id, mID,
                    puid, mediaBaseId, mMediaBase->getID(),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      return mMediaBase->handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannel::handlePacket(RTCPPacketPtr packet)
    {
      ZS_EVENTING_5(
                    x, i, Trace, RtpReceiverChannelDeliverIncomingPacketToMediaChannel, ol, RtpReceiverChannel, Deliver,
                    puid, id, mID,
                    puid, mediaBaseId, mMediaBase->getID(),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );
      return mMediaBase->handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      return mMediaBase->requestStats(promise, stats);
    }




    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => ForMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => ForRTPReceiverChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiverChannel::sendPacket(RTCPPacketPtr packet)
    {
      auto receiver = mReceiver.lock();
      if (!receiver) return false;

      ZS_EVENTING_5(
                    x, i, Trace, RtpReceiverChannelSendOutgoingPacket, ol, RtpReceiverChannel, Send,
                    puid, id, mID,
                    puid, receiverId, receiver->getID(),
                    enum, packetType, zsLib::to_underlying(IICETypes::Component_RTCP),
                    buffer, packet, packet->buffer()->BytePtr(),
                    size, size, packet->buffer()->SizeInBytes()
                    );

      return receiver->sendPacket(packet);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => ForRTPReceiverChannelAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => ForRTPReceiverChannelVideo
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    int32_t RTPReceiverChannel::getAudioSamples(
                                                const size_t numberOfSamples,
                                                const uint8_t numberOfChannels,
                                                void *audioSamples,
                                                size_t& numberOfSamplesOut
                                                )
    {
      numberOfSamplesOut = 0; // report no samples available

#define TODO_VERIFY_RETURN_RESULT 1
#define TODO_VERIFY_RETURN_RESULT 2
      if (!mAudio) return 0;
      
      return mAudio->getAudioSamples(numberOfSamples, numberOfChannels, audioSamples, numberOfSamplesOut);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::onWake()
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
    #pragma mark RTPReceiverChannel => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::onTimer(ITimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
      // NOTE: ADD IF NEEDED...
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => IRTPReceiverChannelAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::onSecureTransportState(ISecureTransport::States state)
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
    void RTPReceiverChannel::onNotifyPacket(RTPPacketPtr packet)
    {
      ZS_LOG_TRACE(log("notified rtcp packets") + ZS_PARAM("packet", packet->ssrc()))
      handlePacket(packet);
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::onNotifyPackets(RTCPPacketListPtr packets)
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
    void RTPReceiverChannel::onUpdate(ParametersPtr params)
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
      }

      mediaBase->notifyUpdate(params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannel::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiverChannel");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannel::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiverChannel::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPReceiverChannel");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      IHelper::debugAppend(resultEl, "secure transport state", ISecureTransport::toString(mSecureTransportState));

      auto receiver = mReceiver.lock();
      IHelper::debugAppend(resultEl, "receiver", receiver ? receiver->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannel::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannel::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::step()
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
    void RTPReceiverChannel::cancel()
    {
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

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPReceiverChannelPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPReceiverChannelStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannel::setError(WORD errorCode, const char *inReason)
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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPReceiverChannelFactory &IRTPReceiverChannelFactory::singleton()
    {
      return RTPReceiverChannelFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelPtr IRTPReceiverChannelFactory::create(
                                                             RTPReceiverPtr receiver,
                                                             MediaStreamTrackPtr track,
                                                             const Parameters &params,
                                                             const RTCPPacketList &packets
                                                             )
    {
      if (this) {}
      return internal::RTPReceiverChannel::create(receiver, track, params, packets);
    }

  } // internal namespace
}
