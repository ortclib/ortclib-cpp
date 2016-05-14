/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/adapter/internal/ortc_adapter_PeerConnection.h>

#include <ortc/internal/ortc_ORTC.h>
#include <ortc/IRTPSender.h>
#include <ortc/IRTPReceiver.h>
#include <ortc/IMediaStreamTrack.h>

#include <ortc/adapter/IMediaStream.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

#ifdef _MSC_VER
#pragma warning(3 : 4062)
#endif //_MSC_VER

namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter) } }

namespace ortc
{
  namespace adapter
  {
    ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper);
    ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP);

    namespace internal
    {
      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IORTCForInternal, UseORTC);

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::TransportInfo::Details
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::TransportInfo::Details::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::TransportInfo::Details");

        UseServicesHelper::debugAppend(resultEl, "ice gatherer", mGatherer ? mGatherer->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "ice transort", mTransport ? mTransport->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "candidates", mCandidates.size());
        UseServicesHelper::debugAppend(resultEl, "end of candidates", mRTPEndOfCandidates);
        UseServicesHelper::debugAppend(resultEl, "dtls transport", mDTLSTransport ? mDTLSTransport->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "srtp/sdes transport", mSRTPSDESTransport ? mSRTPSDESTransport->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "sctp transport", mSCTPTransport ? mSCTPTransport->getID() : 0);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::TransportInfo
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::TransportInfo::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::TransportInfo");

        {
          auto rtpEl = mRTP.toDebug();
          rtpEl->setValue("rtp");
          resultEl->adoptAsLastChild(rtpEl);
        }
        {
          auto rtcpEl = mRTP.toDebug();
          rtcpEl->setValue("rtcp");
          resultEl->adoptAsLastChild(rtcpEl);
        }

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::MediaLineInfo
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::MediaLineInfo::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::MediaLineInfo");

        UseServicesHelper::debugAppend(resultEl, "id", mID);
        UseServicesHelper::debugAppend(resultEl, "mline index", mLineIndex);
        UseServicesHelper::debugAppend(resultEl, "transport id", mTransportID);
        UseServicesHelper::debugAppend(resultEl, "negotiation state", PeerConnection::toString(mNegotiationState));

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::RTPMediaLineInfo
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::RTPMediaLineInfo::toDebug() const
      {
        ElementPtr resultEl = MediaLineInfo::toDebug();
        resultEl->setValue("ortc::adapter::PeerConnection::RTPMediaLineInfo");

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::SCTPMediaLineInfo
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::SCTPMediaLineInfo::toDebug() const
      {
        ElementPtr resultEl = MediaLineInfo::toDebug();
        resultEl->setValue("ortc::adapter::PeerConnection::SCTPMediaLineInfo");

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::SenderInfo
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::SenderInfo::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::SenderInfo");

        UseServicesHelper::debugAppend(resultEl, "id", mID);
        UseServicesHelper::debugAppend(resultEl, "media line id", mMediaLineID);
        UseServicesHelper::debugAppend(resultEl, "track id", mTrack ? mTrack->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "media streams", mMediaStreams.size());
        UseServicesHelper::debugAppend(resultEl, "negotiate state", PeerConnection::toString(mNegotiationState));
        UseServicesHelper::debugAppend(resultEl, "rtp sender id", mSender ? mSender->getID() : 0);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::SenderInfo
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::ReceiverInfo::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::ReceiverInfo");

        UseServicesHelper::debugAppend(resultEl, "id", mID);
        UseServicesHelper::debugAppend(resultEl, "media line id", mMediaLineID);
        UseServicesHelper::debugAppend(resultEl, "negotiate state", PeerConnection::toString(mNegotiationState));
        UseServicesHelper::debugAppend(resultEl, "rtp receiver id", mReceiver ? mReceiver->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "media streams", mMediaStreams.size());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::PendingMethod
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::PendingMethod::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::PendingMethod");

        UseServicesHelper::debugAppend(resultEl, "promise", (bool)mPromise);

        UseServicesHelper::debugAppend(resultEl, "method", PeerConnection::toString(mMethod));

        if (mOfferOptions.hasValue()) {
          UseServicesHelper::debugAppend(resultEl, "offer options", mOfferOptions.value().toDebug());
        }
        if (mAnswerOptions.hasValue()) {
          UseServicesHelper::debugAppend(resultEl, "answer options", mAnswerOptions.value().toDebug());
        }
        if (mCapabilityOptions.hasValue()) {
          UseServicesHelper::debugAppend(resultEl, "capability options", mCapabilityOptions.value().toDebug());
        }

        UseServicesHelper::debugAppend(resultEl, ISessionDescription::toDebug(mSessionDescription));

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection
      #pragma mark

      //-----------------------------------------------------------------------
      PeerConnection::PeerConnection(
                                     const make_private &,
                                     IMessageQueuePtr queue,
                                     IPeerConnectionDelegatePtr delegate,
                                     const Optional<Configuration> &configuration
                                     ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue)
      {
        ZS_LOG_DEBUG(log("created"));
      }

      //-----------------------------------------------------------------------
      PeerConnection::~PeerConnection()
      {
        mThisWeak.reset();
        ZS_LOG_DEBUG(log("destroyed"));
        cancel();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::init()
      {
        wake();
      }

      //-----------------------------------------------------------------------
      const char *PeerConnection::toString(InternalStates state)
      {
        switch (state)
        {
          case InternalState_Pending:       return "Pending";
          case InternalState_Ready:         return "Ready";
          case InternalState_ShuttingDown:  return "Shutting down";
          case InternalState_Shutdown:      return "Shutdown";
        }
        return "unknown";
      }

      //-----------------------------------------------------------------------
      const char *PeerConnection::toString(NegotiationStates state)
      {
        switch (state)
        {
          case NegotiationState_Agreed:         return "agreed";
          case NegotiationState_PendingOffer:   return "pending-offer";
          case NegotiationState_LocalOffered:   return "local-offered";
          case NegotiationState_RemoteOffered:  return "remote-offered";
          case NegotiationState_Rejected:       return "rejected";
        }
        return "unknown";
      }

      //-----------------------------------------------------------------------
      const char *PeerConnection::toString(PendingMethods method)
      {
        switch (method)
        {
          case PendingMethod_CreateOffer:           return "create-offer";
          case PendingMethod_CreateAnswer:          return "create-answer";
          case PendingMethod_CreateCapabilities:    return "create-capabilities";
          case PendingMethod_SetLocalDescription:   return "set-local-description";
          case PendingMethod_SetRemoteDescription:  return "set-remote-description";
        }
        return "unknown";
      }

      //-----------------------------------------------------------------------
      PeerConnectionPtr PeerConnection::convert(IPeerConnectionPtr object)
      {
        return ZS_DYNAMIC_PTR_CAST(PeerConnection, object);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IPeerConnection
      #pragma mark

      //-----------------------------------------------------------------------
      PeerConnectionPtr PeerConnection::create(
                                               IPeerConnectionDelegatePtr delegate,
                                               const Optional<Configuration> &configuration
                                               )
      {
        auto pThis = make_shared<PeerConnection>(make_private{}, UseORTC::queueORTC(), delegate, configuration);
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionSubscriptionPtr PeerConnection::subscribe(IPeerConnectionDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to peer connection"));

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        auto subscription = mSubscriptions.subscribe(originalDelegate, UseORTC::queueDelegate());

        auto delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          auto pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2

          if (SignalingState_First != mLastSignalingState) {
            delegate->onPeerConnectionSignalingStateChange(pThis, mLastSignalingState);
          }
          if (IICEGathererTypes::State_First != mLastICEGatheringStates) {
            delegate->onPeerConnectionICEGatheringStateChange(pThis, mLastICEGatheringStates);
          }
          if (IICETransport::State_First != mLastICEConnectionState) {
            delegate->onPeerConnectionICEConnectionStateChange(pThis, mLastICEConnectionState);
          }
          if (PeerConnectionState_First != mLastPeerConnectionState) {
            delegate->onPeerConnectionConnectionStateChange(pThis, mLastPeerConnectionState);
          }

          if (mNegotiationNeeded) {
            delegate->onPeerConnectionNegotiationNeeded(pThis);
          }
        }

        if (isShutdown()) {
          mSubscriptions.clear();
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PromiseWithDescriptionPtr PeerConnection::createOffer(const Optional<OfferOptions> &configuration)
      {
        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("rejecting create offer since already closed"));
          return PromiseWithDescription::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_CreateOffer, promise);

        pending->mOfferOptions = configuration;

        mPendingMethods.push_back(pending);

        ZS_LOG_DEBUG(log("create offer") + (configuration.hasValue() ? configuration.value().toDebug() : ElementPtr()));

        wake();
        return promise;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PromiseWithDescriptionPtr PeerConnection::createAnswer(const Optional<AnswerOptions> &configuration)
      {
        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("rejecting create answer since already closed"));
          return PromiseWithDescription::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_CreateAnswer, promise);

        pending->mAnswerOptions = configuration;

        mPendingMethods.push_back(pending);

        ZS_LOG_DEBUG(log("create answer") + (configuration.hasValue() ? configuration.value().toDebug() : ElementPtr()));

        wake();
        return promise;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PromiseWithDescriptionPtr PeerConnection::createCapabilities(const Optional<CapabilityOptions> &configuration)
      {
        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("rejecting create capabilities since already closed"));
          return PromiseWithDescription::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_CreateCapabilities, promise);

        pending->mCapabilityOptions = configuration;

        mPendingMethods.push_back(pending);

        ZS_LOG_DEBUG(log("create capabilities") + (configuration.hasValue() ? configuration.value().toDebug() : ElementPtr()));

        wake();
        return promise;
      }

      //-----------------------------------------------------------------------
      PromisePtr PeerConnection::setLocalDescription(ISessionDescriptionPtr description)
      {
        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("rejecting local description since already closed"));
          return PromiseWithDescription::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_SetLocalDescription, promise);

        pending->mSessionDescription = description;

        mPendingMethods.push_back(pending);

        ZS_LOG_DEBUG(log("set local description") + ISessionDescription::toDebug(description));

        wake();
        return promise;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionPtr PeerConnection::localDescription() const
      {
        AutoRecursiveLock lock(*this);
        if (mPendingLocalDescription) return mPendingLocalDescription;
        return mLocalDescription;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionPtr PeerConnection::currentDescription() const
      {
        AutoRecursiveLock lock(*this);
        return mLocalDescription;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionPtr PeerConnection::pendingDescription() const
      {
        AutoRecursiveLock lock(*this);
        return mPendingRemoteDescription;
      }

      //-----------------------------------------------------------------------
      PromisePtr PeerConnection::setRemoteDescription(ISessionDescriptionPtr description)
      {
        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("rejecting remote description since already closed"));
          return Promise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        auto promise = Promise::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_SetRemoteDescription, promise);

        pending->mSessionDescription = description;

        mPendingMethods.push_back(pending);

        ZS_LOG_DEBUG(log("set remote description") + ISessionDescription::toDebug(description));

        wake();
        return promise;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionPtr PeerConnection::remoteDescription() const
      {
        AutoRecursiveLock lock(*this);
        if (mPendingRemoteDescription) return mPendingRemoteDescription;
        return mRemoteDescription;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionPtr PeerConnection::currentRemoteDescription() const
      {
        AutoRecursiveLock lock(*this);
        return mRemoteDescription;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionPtr PeerConnection::pendingRemoteDescription() const
      {
        AutoRecursiveLock lock(*this);
        return mPendingRemoteDescription;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::addICECandidate(const ICECandidate &candidate)
      {
        AutoRecursiveLock lock(*this);
        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("adding candidate after peer connection was closed") + candidate.toDebug());
          return;
        }

        mPendingRemoteCandidates.push_back(make_shared<ICECandidate>(candidate));
        wake();
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::SignalingStates PeerConnection::signalingState() const
      {
        AutoRecursiveLock lock(*this);
        return mLastSignalingState;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::ICEGatheringStates PeerConnection::iceGatheringState() const
      {
        AutoRecursiveLock lock(*this);
        return mLastICEGatheringStates;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::ICEConnectionStates PeerConnection::iceConnectionState() const
      {
        AutoRecursiveLock lock(*this);
        return mLastICEConnectionState;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PeerConnectionStates PeerConnection::connectionState() const
      {
        AutoRecursiveLock lock(*this);
        return mLastPeerConnectionState;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::canTrickleCandidates() const
      {
        return true;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::ServerListPtr PeerConnection::getDefaultIceServers()
      {
        return make_shared<ServerList>();
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::ConfigurationPtr PeerConnection::getConfiguration() const
      {
        AutoRecursiveLock lock(*this);
        return make_shared<Configuration>(mConfiguration);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setConfiguration(const Configuration &configuration)
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close()
      {
        ZS_LOG_DEBUG(log("close called"));

        AutoRecursiveLock lock(*this);
        cancel();
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::SenderListPtr PeerConnection::getSenders() const
      {
        return SenderListPtr();
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::ReceiverListPtr PeerConnection::getReceivers() const
      {
        return ReceiverListPtr();
      }

      //-----------------------------------------------------------------------
      IRTPSenderPtr PeerConnection::addTrack(
                                             IMediaStreamTrackPtr track,
                                             const MediaStreamTrackConfiguration &configuration
                                             )
      {
        return IRTPSenderPtr();
      }

      //-----------------------------------------------------------------------
      IRTPSenderPtr PeerConnection::addTrack(
                                             IMediaStreamTrackPtr track,
                                             const MediaStreamList &mediaStreams,
                                             const MediaStreamTrackConfiguration &configuration
                                             )
      {
        return IRTPSenderPtr();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::removeTrack(IRTPSenderPtr sender)
      {
      }

      //-----------------------------------------------------------------------
      IDataChannelPtr PeerConnection::createDataChannel(const IDataChannelTypes::Parameters &parameters)
      {
        return IDataChannelPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IICEGathererDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererStateChange(
                                                    IICEGathererPtr gatherer,
                                                    IICEGatherer::States state
                                                    )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererLocalCandidate(
                                                       IICEGathererPtr gatherer,
                                                       CandidatePtr candidate
                                                       )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererLocalCandidateComplete(
                                                               IICEGathererPtr gatherer,
                                                               CandidateCompletePtr candidate
                                                               )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererLocalCandidateGone(
                                                           IICEGathererPtr gatherer,
                                                           CandidatePtr candidate
                                                           )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererError(
                                              IICEGathererPtr gatherer,
                                              ErrorEventPtr errorEvent
                                              )
      {
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IICETransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onICETransportStateChange(
                                                     IICETransportPtr transport,
                                                     IICETransport::States state
                                                     )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICETransportCandidatePairAvailable(
                                                                IICETransportPtr transport,
                                                                CandidatePairPtr candidatePair
                                                                )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICETransportCandidatePairGone(
                                                           IICETransportPtr transport,
                                                           CandidatePairPtr candidatePair
                                                           )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICETransportCandidatePairChanged(
                                                              IICETransportPtr transport,
                                                              CandidatePairPtr candidatePair
                                                              )
      {
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IDTLSTransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onDTLSTransportStateChange(
                                                      IDTLSTransportPtr transport,
                                                      IDTLSTransport::States state
                                                      )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onDTLSTransportError(
                                                IDTLSTransportPtr transport,
                                                ErrorAnyPtr error
                                                )
      {
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => ISRTPSDESTransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onSRTPSDESTransportLifetimeRemaining(
                                                                ISRTPSDESTransportPtr transport,
                                                                ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                                ULONG overallLifetimeRemainingPercentage
                                                                )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onSRTPSDESTransportError(
                                                    ISRTPSDESTransportPtr transport,
                                                    ErrorAnyPtr errorCode
                                                    )
      {
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IRTPListener
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onRTPListenerUnhandledRTP(
                                                     IRTPListenerPtr listener,
                                                     SSRCType ssrc,
                                                     PayloadType payloadType,
                                                     const char *mid,
                                                     const char *rid
                                                     )
      {
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => ISCTPTransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onSCTPTransportStateChange(
                                                      ISCTPTransportPtr transport,
                                                      States state
                                                      )
      {
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onSCTPTransportDataChannel(
                                                      ISCTPTransportPtr transport,
                                                      IDataChannelPtr channel
                                                      )
      {
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => ISCTPTransportListenerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onSCTPTransport(ISCTPTransportPtr transport)
      {
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => ISCTPTransportListenerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onWake()
      {
        ZS_LOG_DEBUG("on wake");

        mWakeCalled = false;

        AutoRecursiveLock lock(*this);
        step();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params PeerConnection::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::adapter::PeerConnection");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      Log::Params PeerConnection::debug(const char *message) const
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection");

        UseServicesHelper::debugAppend(resultEl, "id", mID);

        UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

        UseServicesHelper::debugAppend(resultEl, mConfiguration.toDebug());

        UseServicesHelper::debugAppend(resultEl, "state", toString(mState));

        UseServicesHelper::debugAppend(resultEl, "signaling state", IPeerConnectionTypes::toString(mLastSignalingState));
        UseServicesHelper::debugAppend(resultEl, "ice gathering state", IICEGathererTypes::toString(mLastICEGatheringStates));
        UseServicesHelper::debugAppend(resultEl, "ice connection state", IICETransport::toString(mLastICEConnectionState));
        UseServicesHelper::debugAppend(resultEl, "peer connection state", IPeerConnectionTypes::toString(mLastPeerConnectionState));

        UseServicesHelper::debugAppend(resultEl, "wake called", bool(mWakeCalled));

        UseServicesHelper::debugAppend(resultEl, "local description", ISessionDescription::toDebug(mLocalDescription));
        UseServicesHelper::debugAppend(resultEl, "pending local description", ISessionDescription::toDebug(mPendingLocalDescription));

        UseServicesHelper::debugAppend(resultEl, "remote description", ISessionDescription::toDebug(mRemoteDescription));
        UseServicesHelper::debugAppend(resultEl, "pending remote description", ISessionDescription::toDebug(mPendingRemoteDescription));

        UseServicesHelper::debugAppend(resultEl, "pending methods", mPendingMethods.size());

        UseServicesHelper::debugAppend(resultEl, "transports", mTransports.size());
        UseServicesHelper::debugAppend(resultEl, "rtp medias", mRTPMedias.size());
        UseServicesHelper::debugAppend(resultEl, "sctp medias", mSCTPMedias.size());
        UseServicesHelper::debugAppend(resultEl, "senders", mSenders.size());
        UseServicesHelper::debugAppend(resultEl, "receivers", mReceiver.size());

        UseServicesHelper::debugAppend(resultEl, "pending remote candidates", mPendingRemoteCandidates.size());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::wake()
      {
        if (mWakeCalled) return;
        mWakeCalled = true;
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::notifyNegotiationNeeded()
      {
        if (mNegotiationNeeded) return;
        mNegotiationNeeded = true;
        mSubscriptions.delegate()->onPeerConnectionNegotiationNeeded(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void PeerConnection::cancel()
      {
        if (isShutdown()) {
          ZS_LOG_TRACE(log("already shutdown"));
          return;
        }

        setState(InternalState_ShuttingDown);

        if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();



        setState(InternalState_Shutdown);

        mGracefulShutdownReference.reset();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::step()
      {
        if (isStopped()) {
          ZS_LOG_DEBUG(log("step calling cancel"));
          cancel();
          return;
        }

        ZS_LOG_DEBUG(debug("step"));

        // steps
        if (!stepProcessPendingRemoteCandidates()) return;

        goto ready;

      ready:
        {
          setState(InternalState_Ready);
        }
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessPendingRemoteCandidates()
      {
        ZS_LOG_TRACE(log("step - process pending remote candidates"));

        while (mPendingRemoteCandidates.size() > 0) {

          ICECandidatePtr candidate = mPendingRemoteCandidates.front();
          mPendingRemoteCandidates.pop_front();

          if (candidate->mMid.hasData()) {
            auto found = mTransports.find(candidate->mMid);
            if (found == mTransports.end()) goto not_found;
            addCandidateToTransport(*((*found).second), candidate);
            goto found;
          }

          if (!candidate->mMLineIndex.hasValue()) goto not_found;

          for (auto iter = mRTPMedias.begin(); iter != mRTPMedias.end(); ++iter) {
            auto &mediaLine = *((*iter).second);
            if (mediaLine.mLineIndex != candidate->mMLineIndex.value()) continue;

            auto found = mTransports.find(mediaLine.mTransportID);
            if (found == mTransports.end()) goto not_found;
            addCandidateToTransport(*((*found).second), candidate);
            goto found;
          }

          for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter) {
            auto &mediaLine = *((*iter).second);
            if (mediaLine.mLineIndex != candidate->mMLineIndex.value()) continue;

            auto found = mTransports.find(mediaLine.mTransportID);
            if (found == mTransports.end()) goto not_found;
            addCandidateToTransport(*((*found).second), candidate);
            goto found;
          }

        not_found:
          {
            ZS_LOG_WARNING(Debug, log("no transport found for candidate") + candidate->toDebug());
          }
        found: {}
        }
        return true;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setState(InternalStates state)
      {
        if (state == mState) return;

        ZS_LOG_DEBUG(log("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mState)));

        mState = state;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setState(SignalingStates state)
      {
        if (state == mState) return;

        ZS_LOG_DEBUG(log("signaling state changed") + ZS_PARAM("new state", IPeerConnectionTypes::toString(state)) + ZS_PARAM("old state", IPeerConnectionTypes::toString(mLastSignalingState)));

        mLastSignalingState = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onPeerConnectionSignalingStateChange(pThis, state);
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setState(ICEGatheringStates state)
      {
        if (state == mState) return;

        ZS_LOG_DEBUG(log("ice gatherer state changed") + ZS_PARAM("new state", IICEGathererTypes::toString(state)) + ZS_PARAM("old state", IICEGathererTypes::toString(mLastICEGatheringStates)));

        mLastICEGatheringStates = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onPeerConnectionICEGatheringStateChange(pThis, state);
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setState(ICEConnectionStates state)
      {
        if (state == mState) return;

        ZS_LOG_DEBUG(log("ice connection state changed") + ZS_PARAM("new state", IICETransportTypes::toString(state)) + ZS_PARAM("old state", IICETransportTypes::toString(mLastICEConnectionState)));

        mLastICEConnectionState = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onPeerConnectionICEConnectionStateChange(pThis, state);
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setState(PeerConnectionStates state)
      {
        if (state == mState) return;

        ZS_LOG_DEBUG(log("peer connection state changed") + ZS_PARAM("new state", IPeerConnectionTypes::toString(state)) + ZS_PARAM("old state", IPeerConnectionTypes::toString(mLastPeerConnectionState)));

        mLastPeerConnectionState = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onPeerConnectionConnectionStateChange(pThis, state);
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::addCandidateToTransport(
                                                   TransportInfo &transport,
                                                   ICECandidatePtr candidate
                                                   )
      {
        {
          if (!candidate->mCandidate) {
            ZS_LOG_WARNING(Debug, log("candidate to add was null (thus is being ignored)") + candidate->toDebug());
            return;
          }

          if (IICETypes::Component_RTP == candidate->mComponent) {
            if (!transport.mRTP.mTransport) goto not_found;
            transport.mRTP.mTransport->addRemoteCandidate(*(candidate->mCandidate));
            return;
          }
          if (IICETypes::Component_RTCP == candidate->mComponent) {
            if (!transport.mRTCP.mTransport) goto not_found;
            transport.mRTCP.mTransport->addRemoteCandidate(*(candidate->mCandidate));
            return;
          }
          goto not_found;
        }
      not_found:
        {
          ZS_LOG_WARNING(Debug, log("component transport for added candidate was not found") + candidate->toDebug() + transport.toDebug());
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IPeerConnectionFactory
      #pragma mark

      //-------------------------------------------------------------------------
      IPeerConnectionFactory &IPeerConnectionFactory::singleton()
      {
        return PeerConnectionFactory::singleton();
      }

      //-------------------------------------------------------------------------
      PeerConnectionPtr IPeerConnectionFactory::create(
                                                       IPeerConnectionDelegatePtr delegate,
                                                       const Optional<Configuration> &configuration
                                                       )
      {
        if (this) {}
        return internal::PeerConnection::create(delegate, configuration);
      }

      //-------------------------------------------------------------------------
      IPeerConnectionTypes::ServerListPtr IPeerConnectionFactory::getDefaultIceServers()
      {
        if (this) {}
        return internal::PeerConnection::getDefaultIceServers();
      }

    }  // namespace internal

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IPeerConnectionTypes::toString(BundlePolicies policy)
    {
      switch (policy)
      {
        case IPeerConnectionTypes::BundlePolicy_Balanced: return "balanced";
        case IPeerConnectionTypes::BundlePolity_MaxCompat: return "max-compat";
        case IPeerConnectionTypes::BundlePolicy_MaxBundle: return "max-bundle";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    const char *IPeerConnectionTypes::toString(RTCPMuxPolicies policy)
    {
      switch (policy)
      {
        case IPeerConnectionTypes::RTCPMuxPolicy_Negotiated:  return "negotiated";
        case IPeerConnectionTypes::RTCPMuxPolicy_Require:     return "required";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    const char *IPeerConnectionTypes::toString(SignalingModes mode)
    {
      switch (mode)
      {
      case IPeerConnectionTypes::SignalingMode_JSON:  return "json";
      case IPeerConnectionTypes::SignalingMode_SDP:   return "sdp";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    IPeerConnectionTypes::SignalingModes IPeerConnectionTypes::toSignalingMode(const char *mode)
    {
      String str(mode);
      for (SignalingModes index = SignalingMode_First; index <= SignalingMode_Last; index = static_cast<SignalingModes>(static_cast<std::underlying_type<SignalingModes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(toString(index))) return index;
      }

      ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    }

    //-------------------------------------------------------------------------
    bool IPeerConnectionTypes::isCompatible(
                                            SignalingModes mode,
                                            ISessionDescriptionTypes::SignalingTypes signalingType
                                            )
    {
      switch (signalingType)
      {
        case ISessionDescriptionTypes::SignalingType_JSON:            return IPeerConnectionTypes::SignalingMode_JSON == mode;
        case ISessionDescriptionTypes::SignalingType_SDPOffer:
        case ISessionDescriptionTypes::SignalingType_SDPPreanswer:
        case ISessionDescriptionTypes::SignalingType_SDPAnswer:
        case ISessionDescriptionTypes::SignalingType_SDPRollback:     return IPeerConnectionTypes::SignalingMode_SDP == mode;
      }
      return false;
    }

    //-------------------------------------------------------------------------
    const char *IPeerConnectionTypes::toString(SignalingStates state)
    {
      switch (state)
      {
        case IPeerConnectionTypes::SignalingState_Stable:             return "stable";
        case IPeerConnectionTypes::SignalingState_HaveLocalOffer:     return "have-local-offer";
        case IPeerConnectionTypes::SignalingState_HaveRemoteOffer:    return "have-remote-offer";
        case IPeerConnectionTypes::SignalingState_HaveLocalPranswer:  return "have-local-pranswer";
        case IPeerConnectionTypes::SignalingState_HaveRemotePranswer: return "have-remote-pranswer";
        case IPeerConnectionTypes::SignalingState_Closed:             return "closed";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    const char *IPeerConnectionTypes::toString(PeerConnectionStates state)
    {
      switch (state)
      {
        case IPeerConnectionTypes::PeerConnectionState_New:           return "new";
        case IPeerConnectionTypes::PeerConnectionState_Connecting:    return "connecting";
        case IPeerConnectionTypes::PeerConnectionState_Connected:     return "connected";
        case IPeerConnectionTypes::PeerConnectionState_Disconnected:  return "disconnected";
        case IPeerConnectionTypes::PeerConnectionState_Failed:        return "failed";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::Configuration
    #pragma mark

    //-------------------------------------------------------------------------
    IPeerConnection::Configuration::Configuration(const Configuration &op2) :
      mGatherOptions(op2.mGatherOptions ? make_shared<IICEGathererTypes::Options>(*op2.mGatherOptions) : IICEGathererTypes::OptionsPtr()),
      mSignalingMode(op2.mSignalingMode),
      mBundlePolicy(op2.mBundlePolicy),
      mRTCPMuxPolicy(op2.mRTCPMuxPolicy),
      mCertificates(op2.mCertificates)
    {
    }

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnection::Configuration::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::adapter::IPeerConnectionTypes::Configuration");

      UseServicesHelper::debugAppend(resultEl, mGatherOptions ? mGatherOptions->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "signaling mode", IPeerConnectionTypes::toString(mSignalingMode));
      UseServicesHelper::debugAppend(resultEl, "negotiate SRTP SDES", mNegotiateSRTPSDES);
      UseServicesHelper::debugAppend(resultEl, "bundle policy", IPeerConnectionTypes::toString(mBundlePolicy));
      UseServicesHelper::debugAppend(resultEl, "rtcp policy", IPeerConnectionTypes::toString(mRTCPMuxPolicy));
      UseServicesHelper::debugAppend(resultEl, "certificates", mCertificates.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::OfferAnswerOptions
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnection::OfferAnswerOptions::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::adapter::IPeerConnectionTypes::OfferAnswerOptions");
      UseServicesHelper::debugAppend(resultEl, "voice activity detection", mVoiceActivityDetection);
      return resultEl;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::OfferOptions
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::OfferOptions::toDebug() const
    {
      auto resultEl = OfferAnswerOptions::toDebug();
      resultEl->setValue("ortc::adapter::IPeerConnectionTypes::OfferAnswerOptions");
      UseServicesHelper::debugAppend(resultEl, "ice restart", mICERestart);
      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::OfferOptions
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::AnswerOptions::toDebug() const
    {
      auto resultEl = OfferAnswerOptions::toDebug();
      resultEl->setValue("ortc::adapter::IPeerConnectionTypes::AnswerOptions");
      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::CapabilityOptions
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::CapabilityOptions::toDebug() const
    {
      auto resultEl = OfferAnswerOptions::toDebug();
      resultEl->setValue("ortc::adapter::IPeerConnectionTypes::CapabilityOptions");
      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::MediaStreamTrackConfiguration
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::MediaStreamTrackConfiguration::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::adapter::IPeerConnectionTypes::MediaStreamTrackConfiguration");
      UseServicesHelper::debugAppend(resultEl, mParameters ? mParameters->toDebug() : ElementPtr());
      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes::ICECandidateErrorEvent
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::ICECandidateErrorEvent::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::adapter::IPeerConnectionTypes::ICECandidateErrorEvent");
      UseServicesHelper::debugAppend(resultEl, "mid", mMid);
      UseServicesHelper::debugAppend(resultEl, "m line index", mMLineIndex);
      UseServicesHelper::debugAppend(resultEl, "ice candidate", mHostCandidate ? mHostCandidate->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "error code", mErrorCode);
      UseServicesHelper::debugAppend(resultEl, "error text", mErrorText);
      return resultEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::MediaStreamTrackEvent::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::adapter::IPeerConnectionTypes::MediaStreamTrackEvent");
      UseServicesHelper::debugAppend(resultEl, "receiver id", mReceiver ? mReceiver->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "track id", mTrack ? mTrack->getID() : 0);

      ElementPtr mediaStreamsEl = Element::create("media streams");
      for (auto iter = mMediaStreams.begin(); iter != mMediaStreams.end(); ++iter) {
        auto &mediaStream = (*iter);
        UseServicesHelper::debugAppend(mediaStreamsEl, "media stream id", mediaStream ? mediaStream->getID() : 0);
      }
      if (mediaStreamsEl->hasChildren()) {
        resultEl->adoptAsFirstChild(mediaStreamsEl);
      }

      return resultEl;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark PeerConnection
    #pragma mark

    //-------------------------------------------------------------------------
    IPeerConnectionPtr IPeerConnection::create(
                                               IPeerConnectionDelegatePtr delegate,
                                               const Optional<Configuration> &configuration
                                               )
    {
      return internal::IPeerConnectionFactory::singleton().create(delegate, configuration);
    }

    //-------------------------------------------------------------------------
    IPeerConnection::ServerListPtr IPeerConnection::getDefaultIceServers()
    {
      return internal::IPeerConnectionFactory::singleton().getDefaultIceServers();
    }

  } // namespace adapter
} // namespace ortc
