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
#include <ortc/adapter/internal/ortc_adapter_MediaStream.h>

#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_RTPTypes.h>

#include <ortc/IRTPSender.h>
#include <ortc/IRTPReceiver.h>
#include <ortc/IMediaStreamTrack.h>

#include <ortc/adapter/IMediaStream.h>
#include <ortc/adapter/IHelper.h>

#include <ortc/services/IHelper.h>
#include <ortc/services/IHTTP.h>

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
    ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
    ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);
    ZS_DECLARE_TYPEDEF_PTR(adapter::IHelper, UseAdapterHelper);
    ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPTypesHelper, UseRTPTypesHelper);

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
        UseServicesHelper::debugAppend(resultEl, "end of candidates", mRTPEndOfCandidates);
        UseServicesHelper::debugAppend(resultEl, "dtls transport", mDTLSTransport ? mDTLSTransport->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "srtp/sdes transport", mSRTPSDESTransport ? mSRTPSDESTransport->getID() : 0);

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

        UseServicesHelper::debugAppend(resultEl, "certificates", mCertificates.size());

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
        UseServicesHelper::debugAppend(resultEl, "bundled transport id", mBundledTransportID);
        UseServicesHelper::debugAppend(resultEl, "private transport id", mPrivateTransportID);
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

        UseServicesHelper::debugAppend(resultEl, "media type", mMediaType);
        UseServicesHelper::debugAppend(resultEl, "id preference", UseAdapterHelper::toString(mIDPreference));
        UseServicesHelper::debugAppend(resultEl, "local sender capabilities", mLocalSenderCapabilities ? mLocalSenderCapabilities->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(resultEl, "local receiver capabilities", mLocalReceiverCapabilities ? mLocalReceiverCapabilities->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(resultEl, "remote sender capabilities", mRemoteSenderCapabilities ? mRemoteSenderCapabilities->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(resultEl, "remote receiver capabilities", mRemoteReceiverCapabilities ? mRemoteReceiverCapabilities->toDebug() : ElementPtr());

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

        UseServicesHelper::debugAppend(resultEl, "sctp transport", mSCTPTransport ? mSCTPTransport->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "data channels", mDataChannels.size());

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
        UseServicesHelper::debugAppend(resultEl, mConfiguration ? mConfiguration->toDebug() : ElementPtr());
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
        UseServicesHelper::debugAppend(resultEl, "media stream track id", mMediaStreamTrackID);
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
      #pragma mark PeerConnection::PendingAddTrack
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::PendingAddTrack::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::PendingAddTrack");

        UseServicesHelper::debugAppend(resultEl, "promise", (bool)mPromise);

        UseServicesHelper::debugAppend(resultEl, "track id", mTrack ? mTrack->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "media streams", mMediaStreams.size());
        UseServicesHelper::debugAppend(resultEl, mConfiguration ? mConfiguration->toDebug() : ElementPtr());

        return resultEl;
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection::PendingAddDataChannel
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr PeerConnection::PendingAddDataChannel::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::PeerConnection::PendingAddDataChannel");

        UseServicesHelper::debugAppend(resultEl, "promise", (bool)mPromise);
        UseServicesHelper::debugAppend(resultEl, mParameters ? mParameters->toDebug() : ElementPtr());

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
        if (configuration.hasValue()) {
          mConfiguration = configuration.value();
        }

        mDefaultSubscription = mSubscriptions.subscribe(delegate, UseORTC::queueDelegate());

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
        AutoRecursiveLock lock(*this);

        for (size_t index = 0; index < mConfiguration.mICECandidatePoolSize; ++index) {
          addToTransportPool();
        }

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

        switch (mConfiguration.mSignalingMode) {
          case IPeerConnectionTypes::SignalingMode_JSON: {

            Optional<CapabilityOptions> options;
            if (configuration.hasValue()) {
              options = CapabilityOptions();
              options.value().mVoiceActivityDetection = configuration.value().mVoiceActivityDetection;
            }

            return createCapabilities(options);
          }
          case IPeerConnectionTypes::SignalingMode_SDP:   break;
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_CreateOffer, promise);

        pending->mOfferOptions = configuration;

        moveAddedTracksToPending();
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

        switch (mConfiguration.mSignalingMode) {
          case IPeerConnectionTypes::SignalingMode_JSON: {

            Optional<CapabilityOptions> options;
            if (configuration.hasValue()) {
              options = CapabilityOptions();
              options.value().mVoiceActivityDetection = configuration.value().mVoiceActivityDetection;
            }

            return createCapabilities(options);
          }
          case IPeerConnectionTypes::SignalingMode_SDP:   break;
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_CreateAnswer, promise);

        pending->mAnswerOptions = configuration;

        moveAddedTracksToPending();
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

        moveAddedTracksToPending();
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

        switch (mConfiguration.mSignalingMode) {
          case IPeerConnectionTypes::SignalingMode_JSON: {
            if (ISessionDescriptionTypes::SignalingType_JSON != description->type()) {
              ZS_LOG_WARNING(Debug, log("rejecting local description (sent in SDP but expecting JSON)"));
              return Promise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "expecting JSON but received SDP (mixing JSON/SDP signaling in same connection)"), UseORTC::queueDelegate());
            }
            break;
          }
          case IPeerConnectionTypes::SignalingMode_SDP: {
            switch (description->type()) {
              case ISessionDescriptionTypes::SignalingType_JSON: {
                ZS_LOG_WARNING(Debug, log("rejecting local description (as signaling is mixing JSON/SDP)"));
                return Promise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "expecting SDP but received JSON (mixing JSON/SDP signaling in same connection)"), UseORTC::queueDelegate());
              }
              case ISessionDescriptionTypes::SignalingType_SDPOffer: 
              case ISessionDescriptionTypes::SignalingType_SDPPranswer: 
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:
              case ISessionDescriptionTypes::SignalingType_SDPRollback:
              {
                break;
              }
            }
          }
        }

        auto promise = PromiseWithDescription::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_SetLocalDescription, promise);

        pending->mSessionDescription = description;
        mPendingLocalDescription = description;

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
        ORTC_THROW_INVALID_PARAMETERS_IF(!description);

        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("rejecting remote description (since already closed)"));
          return Promise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        switch (mConfiguration.mSignalingMode) {
          case IPeerConnectionTypes::SignalingMode_JSON: {
            if (ISessionDescriptionTypes::SignalingType_JSON != description->type()) {
              ZS_LOG_WARNING(Debug, log("rejecting remote description (sent in SDP but expecting JSON)"));
              return Promise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "expecting JSON but received SDP (mixing JSON/SDP signaling in same connection)"), UseORTC::queueDelegate());
            }
            break;
          }
          case IPeerConnectionTypes::SignalingMode_SDP: {
            switch (description->type()) {
              case ISessionDescriptionTypes::SignalingType_JSON: {
                ZS_LOG_WARNING(Debug, log("rejecting remote description (as signaling is mixing JSON/SDP)"));
                return Promise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "expecting SDP but received JSON (mixing JSON/SDP signaling in same connection)"), UseORTC::queueDelegate());
              }
              case ISessionDescriptionTypes::SignalingType_SDPOffer: 
              case ISessionDescriptionTypes::SignalingType_SDPPranswer: 
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:
              case ISessionDescriptionTypes::SignalingType_SDPRollback:
              {
                break;
              }
            }
          }
        }

        auto promise = Promise::create(UseORTC::queueDelegate());

        auto pending = make_shared<PendingMethod>(PendingMethod_SetRemoteDescription, promise);

        pending->mSessionDescription = description;
        mPendingRemoteDescription = description;

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
        AutoRecursiveLock lock(*this);

        Configuration oldConfiguration(mConfiguration);

        mConfiguration = configuration;
        if (mConfiguration.mCertificates.size() < 1) {
          mConfiguration.mCertificates = oldConfiguration.mCertificates;
        }

#define TODO_SET_CONFIGURATION 1
#define TODO_SET_CONFIGURATION 2
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
        auto result = make_shared<SenderList>();

        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("attempting to get senders while shutdown / shutting down"));
          return result;
        }

        for (auto iter = mSenders.begin(); iter != mSenders.end(); ++iter) {
          auto &senderInfo = ((*iter).second);

          auto sender = senderInfo->mSender;
          if (!sender) continue;

          result->push_back(sender);
        }

        ZS_LOG_TRACE(log("total senders returned") + ZS_PARAM("total", result->size()));

        return result;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::ReceiverListPtr PeerConnection::getReceivers() const
      {
        auto result = make_shared<ReceiverList>();

        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("attempting to get receivers while shutdown / shutting down"));
          return result;
        }

        for (auto iter = mReceivers.begin(); iter != mReceivers.end(); ++iter) {
          auto &receiverInfo = ((*iter).second);

          auto receiver = receiverInfo->mReceiver;
          if (!receiver) continue;

          result->push_back(receiver);
        }

        ZS_LOG_TRACE(log("total receivers returned") + ZS_PARAM("total", result->size()));

        return result;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PromiseWithSenderPtr PeerConnection::addTrack(
                                                                          IMediaStreamTrackPtr track,
                                                                          const MediaStreamTrackConfiguration &inConfiguration
                                                                          )
      {
        MediaStreamList mediaStreams;
        return addTrack(track, mediaStreams, inConfiguration);
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PromiseWithSenderPtr PeerConnection::addTrack(
                                                                          IMediaStreamTrackPtr track,
                                                                          const MediaStreamList &mediaStreams,
                                                                          const MediaStreamTrackConfiguration &inConfiguration
                                                                          )
      {
        ORTC_THROW_INVALID_PARAMETERS_IF(!track);

        auto configuration = make_shared<MediaStreamTrackConfiguration>(inConfiguration);

        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("cannot add track when peer connection is closed"));
          return PromiseWithSender::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        if (IMediaStreamTrackTypes::State_Ended == track->readyState()) {
          ZS_LOG_WARNING(Debug, log("cannot add track that has already ended"));
          return PromiseWithSender::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_PreconditionFailed, "media stream track has already ended"), UseORTC::queueDelegate());
        }

        auto pending = make_shared<PendingAddTrack>();

        pending->mPromise = PromiseWithSender::create(UseORTC::queueDelegate());
        pending->mConfiguration = configuration;
        pending->mTrack = track;

        pending->mMediaStreams = *convertToMap(mediaStreams);

        mAddedPendingAddTracks.push_back(pending);
        notifyNegotiationNeeded();

        ZS_LOG_DEBUG(log("will attempt to add track") + pending->toDebug());

        wake();

        return pending->mPromise;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::removeTrack(IRTPSenderPtr sender)
      {
        ORTC_THROW_INVALID_PARAMETERS_IF(!sender);

        AutoRecursiveLock lock(*this);
        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("attempting to remove track during shutdown (thus ignoring request)"));
          return;
        }

        ZS_LOG_DEBUG(log("will remove track") + ZS_PARAM("sender id", sender->getID()));

        mPendingRemoveTracks.push_back(sender);
        wake();
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::PromiseWithDataChannelPtr PeerConnection::createDataChannel(const IDataChannelTypes::Parameters &inParameters)
      {
        auto parameters = make_shared<IDataChannelTypes::Parameters>(inParameters);

        AutoRecursiveLock lock(*this);

        if (isStopped()) {
          ZS_LOG_WARNING(Debug, log("cannot add track when peer connection is closed"));
          return PromiseWithDataChannel::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "peer connection is already closed"), UseORTC::queueDelegate());
        }

        auto pending = make_shared<PendingAddDataChannel>();

        pending->mPromise = PromiseWithDataChannel::create(UseORTC::queueDelegate());
        pending->mParameters = parameters;

        mPendingAddDataChannels.push_back(pending);

        ZS_LOG_DEBUG(log("will attempt to add data cahnnel") + pending->toDebug());

        wake();

        return pending->mPromise;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IStatsProvider
      #pragma mark

      //-----------------------------------------------------------------------
      PeerConnection::PromiseWithStatsReportPtr PeerConnection::getStats(const StatsTypeSet &stats) const
      {
        auto promise = PromiseWithStatsReport::create(UseORTC::queueDelegate());

        IPeerConnectionAsyncDelegateProxy::create(mThisWeak.lock())->onProvideStats(promise, stats);

        return promise;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IPeerConnectionAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onProvideStats(PromiseWithStatsReportPtr promise, StatsTypeSet stats)
      {
        AutoRecursiveLock lock(*this);

        UseStatsReport::PromiseWithStatsReportList promises;

        bool hasTransportCandidateRelated = stats.hasStatType(IStatsReportTypes::StatsType_CandidatePair) ||
                                            stats.hasStatType(IStatsReportTypes::StatsType_LocalCandidate) ||
                                            stats.hasStatType(IStatsReportTypes::StatsType_RemoteCandidate);

        StatsTypeSet onlyStreamSet;
        onlyStreamSet.insert(IStatsReport::StatsType_Stream);

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &info = *((*iter).second);

          if ((stats.hasStatType(IStatsReportTypes::StatsType_ICEGatherer)) ||
              (stats.hasStatType(IStatsReportTypes::StatsType_Candidate))) {
            if (info.mRTP.mGatherer) {
              promises.push_back(info.mRTP.mGatherer->getStats(stats));
            }
            if (info.mRTCP.mGatherer) {
              promises.push_back(info.mRTCP.mGatherer->getStats(stats));
            }
          }
          if ((stats.hasStatType(IStatsReportTypes::StatsType_ICETransport)) ||
              (hasTransportCandidateRelated)) {
            if (info.mRTP.mTransport) {
              promises.push_back(info.mRTP.mTransport->getStats(stats));
            }
            if (info.mRTCP.mTransport) {
              promises.push_back(info.mRTCP.mTransport->getStats(stats));
            }
          }
          if ((stats.hasStatType(IStatsReportTypes::StatsType_DTLSTransport)) ||
              (stats.hasStatType(IStatsReportTypes::StatsType_Certificate))) {
            if (info.mRTP.mDTLSTransport) {
              promises.push_back(info.mRTP.mDTLSTransport->getStats(stats));
            }
            if (info.mRTCP.mDTLSTransport) {
              promises.push_back(info.mRTCP.mDTLSTransport->getStats(stats));
            }
          }
          if (stats.hasStatType(IStatsReportTypes::StatsType_SRTPTransport)) {
            if (info.mRTP.mSRTPSDESTransport) {
              promises.push_back(info.mRTP.mSRTPSDESTransport->getStats(stats));
            }
          }
        }

        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter) {
          auto &info = *((*iter).second);

          if (stats.hasStatType(IStatsReportTypes::StatsType_DataChannel)) {
            for (auto iter = info.mDataChannels.begin(); iter != info.mDataChannels.end(); ++iter)
            {
              auto dataChannelInfo = (*iter).second;
              promises.push_back(dataChannelInfo->mDataChannel->getStats(stats));
            }
          }

          if (stats.hasStatType(IStatsReportTypes::StatsType_SCTPTransport)) {
            if (info.mSCTPTransport) {
              promises.push_back(info.mSCTPTransport->getStats(stats));
            }
          }
        }

        for (auto iter = mSenders.begin(); iter != mSenders.end(); ++iter) {
          auto &info = *((*iter).second);

          if (stats.hasStatType(IStatsReportTypes::StatsType_Track)) {
            if (info.mTrack) {
              promises.push_back(info.mTrack->getStats(stats));
            }
          }
          if (stats.hasStatType(IStatsReportTypes::StatsType_OutboundRTP)) {
            if (info.mSender) {
              promises.push_back(info.mSender->getStats(stats));
            }
          }
          if (stats.hasStatType(IStatsReportTypes::StatsType_Stream)) {
            for (auto iterStream = info.mMediaStreams.begin(); iterStream != info.mMediaStreams.end(); ++iterStream) {
              IMediaStreamPtr stream = MediaStream::convert((*iterStream).second);
              promises.push_back(stream->getStats(onlyStreamSet));
            }
          }
        }

        for (auto iter = mReceivers.begin(); iter != mReceivers.end(); ++iter) {
          auto &info = *((*iter).second);

          if (stats.hasStatType(IStatsReportTypes::StatsType_Track)) {
            if (info.mReceiver) {
              auto track = info.mReceiver->track();
              if (track) {
                promises.push_back(track->getStats(stats));
              }
            }
          }
          if (stats.hasStatType(IStatsReportTypes::StatsType_InboundRTP)) {
            if (info.mReceiver) {
              promises.push_back(info.mReceiver->getStats(stats));
            }
          }
          if (stats.hasStatType(IStatsReportTypes::StatsType_Stream)) {
            for (auto iterStream = info.mMediaStreams.begin(); iterStream != info.mMediaStreams.end(); ++iterStream) {
              IMediaStreamPtr stream = MediaStream::convert((*iterStream).second);
              promises.push_back(stream->getStats(onlyStreamSet));
            }
          }
        }

        if (promises.size() < 1) {
          promise->resolve(UseStatsReport::create(UseStatsReport::StatMap()));
          return;
        }

        auto collecionPromise = UseStatsReport::collectReports(promises);

        mPendingStatPromises[collecionPromise->getID()] = CollectionPromisePair(collecionPromise, promise);

        collecionPromise->thenWeak(mThisWeak.lock());
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
        ZS_LOG_TRACE(log("on ice gatherer state change") + ZS_PARAM("gatherer id", gatherer->getID()) + ZS_PARAM("state", IICEGathererTypes::toString(state)));

        AutoRecursiveLock lock(*this);
        step();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererLocalCandidate(
                                                       IICEGathererPtr gatherer,
                                                       CandidatePtr candidate
                                                       )
      {
        ZS_LOG_TRACE(log("on ice gatherer candidate") + ZS_PARAM("gatherer id", gatherer->getID()) + candidate->toDebug());

        AutoRecursiveLock lock(*this);

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &transportInfo = (*iter).second;
          if ((transportInfo->mRTP.mGatherer) &&
              (transportInfo->mRTP.mGatherer->getID() == gatherer->getID())) {
            goto fire_candidate;
          }
          if ((transportInfo->mRTCP.mGatherer) &&
              (transportInfo->mRTCP.mGatherer->getID() == gatherer->getID())) {
            goto fire_candidate;
          }

          goto skip;

        skip:
          {
            continue;
          }

        fire_candidate:
          {
            auto midCandidate = make_shared<IPeerConnectionTypes::ICECandidate>();
            midCandidate->mMid = transportInfo->mID;

            switch (mConfiguration.mSignalingMode)
            {
              case IPeerConnectionTypes::SignalingMode_JSON:  break;
              case IPeerConnectionTypes::SignalingMode_SDP: {
                {
                  auto foundMedia = mRTPMedias.find(transportInfo->mID);
                  if (foundMedia != mRTPMedias.end()) {
                    auto &mediaLineInfo = *((*foundMedia).second);
                    midCandidate->mMLineIndex = mediaLineInfo.mLineIndex;
                    break;
                  }
                }
                {
                  auto foundMedia = mSCTPMedias.find(transportInfo->mID);
                  if (foundMedia != mSCTPMedias.end()) {
                    auto &mediaLineInfo = *((*foundMedia).second);
                    midCandidate->mMLineIndex = mediaLineInfo.mLineIndex;
                    break;
                  }
                }
                break;
              }
            }

            midCandidate->mCandidate = candidate;
            mSubscriptions.delegate()->onPeerConnectionIceCandidate(mThisWeak.lock(), midCandidate, "");
          }
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererLocalCandidateComplete(
                                                               IICEGathererPtr gatherer,
                                                               CandidateCompletePtr candidate
                                                               )
      {
        ZS_LOG_TRACE(log("on ice gatherer candidate complete") + ZS_PARAM("gatherer id", gatherer->getID()) + candidate->toDebug());

        AutoRecursiveLock lock(*this);

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &transportInfo = (*iter).second;
          if ((transportInfo->mRTP.mGatherer) &&
            (transportInfo->mRTP.mGatherer->getID() == gatherer->getID())) {
            goto fire_candidate;
          }
          if ((transportInfo->mRTCP.mGatherer) &&
            (transportInfo->mRTCP.mGatherer->getID() == gatherer->getID())) {
            goto fire_candidate;
          }

          goto skip;

        skip:
          {
            continue;
          }

        fire_candidate:
          {
            auto midCandidate = make_shared<IPeerConnectionTypes::ICECandidate>();
            midCandidate->mMid = transportInfo->mID;
            midCandidate->mCandidate = candidate;
            mSubscriptions.delegate()->onPeerConnectionIceCandidate(mThisWeak.lock(), midCandidate, "");
          }
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererLocalCandidateGone(
                                                           IICEGathererPtr gatherer,
                                                           CandidatePtr candidate
                                                           )
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onICEGathererError(
                                              IICEGathererPtr gatherer,
                                              ErrorEventPtr errorEvent
                                              )
      {
        // ignored
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
        ZS_LOG_TRACE(log("on ice transport state change") + ZS_PARAM("transport id", transport->getID()) + ZS_PARAM("state", IICETransportTypes::toString(state)));

        AutoRecursiveLock lock(*this);
        step();
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
        ZS_LOG_TRACE(log("on dtls transport state change") + ZS_PARAM("transport id", transport->getID()) + ZS_PARAM("state", IDTLSTransportTypes::toString(state)));

        AutoRecursiveLock lock(*this);
        step();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onDTLSTransportError(
                                                IDTLSTransportPtr transport,
                                                ErrorAnyPtr error
                                                )
      {
        // ignored
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
        // ignored
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onSRTPSDESTransportError(
                                                    ISRTPSDESTransportPtr transport,
                                                    ErrorAnyPtr errorCode
                                                    )
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IRTPListenerDelegate
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
        ZS_LOG_TRACE(log("on rtp listener unhandled") + ZS_PARAM("listener id", listener->getID()) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload", payloadType) + ZS_PARAM("mid", mid) + ZS_PARAM("rid", rid));
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IRTPSenderDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onRTPSenderSSRCConflict(
                                                   IRTPSenderPtr sender,
                                                   SSRCType ssrc
                                                   )
      {
        ZS_LOG_ERROR(Basic, log("SSRC conflict detected") + ZS_PARAM("sender", sender->getID()) + ZS_PARAM("ssrc", ssrc));
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
                                                      ISCTPTransportTypes::States state
                                                      )
      {
        ZS_LOG_TRACE(log("on sctp transport state change") + ZS_PARAM("transport id", transport->getID()) + ZS_PARAM("state", ISCTPTransportTypes::toString(state)));

        AutoRecursiveLock lock(*this);
        step();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onSCTPTransportDataChannel(
                                                      ISCTPTransportPtr transport,
                                                      IDataChannelPtr channel
                                                      )
      {
        ZS_LOG_TRACE(log("on sctp transport") + ZS_PARAM("transport id", transport->getID()) + ZS_PARAM("channel id", channel->getID()));

        AutoRecursiveLock lock(*this);

        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter)
        {
          auto &mediaLine = (*iter).second;
          if (!mediaLine->mSCTPTransport) continue;
          if (mediaLine->mSCTPTransport->getID() != transport->getID()) continue;

          auto dataChannelInfo = make_shared<DataChannelInfo>();
          dataChannelInfo->mDataChannel = channel;
          dataChannelInfo->mSubscription = channel->subscribe(mThisWeak.lock());
          mediaLine->mDataChannels[channel->getID()] = dataChannelInfo;
          break;
        }

        mSubscriptions.delegate()->onPeerConnectionDataChannel(mThisWeak.lock(), channel);
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
        ZS_LOG_TRACE(log("on sctp transport listener") + ZS_PARAM("transport id", transport->getID()));
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection => IDataChannelDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onDataChannelStateChange(
                                                    IDataChannelPtr channel,
                                                    IDataChannelTypes::States state
                                                    )
      {
        if (state != IDataChannelTypes::State_Closed) return;

        AutoRecursiveLock lock(*this);

        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter)
        {
          auto &mediaInfo = (*iter).second;
          
          auto found = mediaInfo->mDataChannels.find(channel->getID());
          if (found == mediaInfo->mDataChannels.end()) continue;

          mediaInfo->mDataChannels.erase(found);
          ZS_LOG_DEBUG(log("data channel was remove") + ZS_PARAM("data channel id", channel->getID()));
          return;
        }

        ZS_LOG_WARNING(Trace, log("data channel was close but not found in any media line (probably okay if during peer connection shutdown)") + ZS_PARAM("data channel id", channel->getID()));
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onDataChannelError(
                                              IDataChannelPtr channel,
                                              ErrorAnyPtr error
                                              )
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onDataChannelBufferedAmountLow(IDataChannelPtr channel)
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      void PeerConnection::onDataChannelMessage(
                                                IDataChannelPtr channel,
                                                MessageEventDataPtr data
                                                )
      {
        // ignored
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
      #pragma mark PeerConnection => IPromiseSettledDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onPromiseSettled(PromisePtr promise)
      {
        ZS_LOG_DEBUG("promise settled");

        PromiseWithStatsReportPtr resolvePromise;
        PromiseWithStatsReportPtr collectionPromise;

        // handle promise
        {
          AutoRecursiveLock lock(*this);

          // check if stats promise
          {
            auto found = mPendingStatPromises.find(promise->getID());
            if (found != mPendingStatPromises.end()) {
              collectionPromise = (*found).second.first;
              resolvePromise = (*found).second.second;
              mPendingStatPromises.erase(found);
              goto process_stats;
            }
          }

          step();
          return;
        }

      process_stats:
        {
          // intentionally process outside of lock
          processStats(collectionPromise, resolvePromise);
        }
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
        UseServicesHelper::debugAppend(resultEl, "error code", mErrorCode);
        UseServicesHelper::debugAppend(resultEl, "error reason", mErrorReason);

        UseServicesHelper::debugAppend(resultEl, "signaling state", IPeerConnectionTypes::toString(mLastSignalingState));
        UseServicesHelper::debugAppend(resultEl, "ice gathering state", IICEGathererTypes::toString(mLastICEGatheringStates));
        UseServicesHelper::debugAppend(resultEl, "ice connection state", IICETransport::toString(mLastICEConnectionState));
        UseServicesHelper::debugAppend(resultEl, "peer connection state", IPeerConnectionTypes::toString(mLastPeerConnectionState));

        UseServicesHelper::debugAppend(resultEl, "wake called", bool(mWakeCalled));
        UseServicesHelper::debugAppend(resultEl, "negotiation needed", mNegotiationNeeded);

        UseServicesHelper::debugAppend(resultEl, "local description", ISessionDescription::toDebug(mLocalDescription));
        UseServicesHelper::debugAppend(resultEl, "pending local description", ISessionDescription::toDebug(mPendingLocalDescription));

        UseServicesHelper::debugAppend(resultEl, "remote description", ISessionDescription::toDebug(mRemoteDescription));
        UseServicesHelper::debugAppend(resultEl, "pending remote description", ISessionDescription::toDebug(mPendingRemoteDescription));

        UseServicesHelper::debugAppend(resultEl, "pending methods", mPendingMethods.size());
        UseServicesHelper::debugAppend(resultEl, "added pending add tracks", mAddedPendingAddTracks.size());
        UseServicesHelper::debugAppend(resultEl, "pending add tracks", mPendingAddTracks.size());
        UseServicesHelper::debugAppend(resultEl, "pending remove tracks", mPendingRemoveTracks.size());
        UseServicesHelper::debugAppend(resultEl, "pending add data channels", mPendingAddDataChannels.size());

        UseServicesHelper::debugAppend(resultEl, "transports", mTransports.size());
        UseServicesHelper::debugAppend(resultEl, "rtp medias", mRTPMedias.size());
        UseServicesHelper::debugAppend(resultEl, "sctp medias", mSCTPMedias.size());
        UseServicesHelper::debugAppend(resultEl, "senders", mSenders.size());
        UseServicesHelper::debugAppend(resultEl, "receivers", mReceivers.size());

        UseServicesHelper::debugAppend(resultEl, "media streams", mMediaStreams.size());
        UseServicesHelper::debugAppend(resultEl, "pending remote candidates", mPendingRemoteCandidates.size());

        UseServicesHelper::debugAppend(resultEl, "pending transport pool", mTransportPool.size());

        UseServicesHelper::debugAppend(resultEl, "existing ids", mExistingIDs.size());

        UseServicesHelper::debugAppend(resultEl, "certificate promise", bool(mCertificatePromise));

        UseServicesHelper::debugAppend(resultEl, "pending stat promises", mPendingStatPromises.size());

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
        if (isStopped()) return;

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

        setState(IICEGathererTypes::State_Complete);
        setState(IICETransportTypes::State_Closed);
        setState(IPeerConnectionTypes::PeerConnectionState_Closed);

        if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

        for (auto iter = mPendingMethods.begin(); iter != mPendingMethods.end(); ++iter) {
          auto &pending = (*iter);

          close(*pending);
        }

        moveAddedTracksToPending();

        for (auto iter = mPendingAddTracks.begin(); iter != mPendingAddTracks.end(); ++iter) {
          auto &pending = (*iter);

          close(*pending);
        }

        for (auto iter = mPendingAddDataChannels.begin(); iter != mPendingAddDataChannels.end(); ++iter) {
          auto &pending = (*iter);

          close(*pending);
        }

        for (auto iter = mReceivers.begin(); iter != mReceivers.end(); ++iter) {
          auto &receiverInfo = (*iter).second;

          close(*receiverInfo);
        }

        for (auto iter = mSenders.begin(); iter != mSenders.end(); ++iter) {
          auto &senderInfo = (*iter).second;

          close(*senderInfo);
        }

        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter) {
          auto &mediaInfo = (*iter).second;

          close(*mediaInfo);
        }

        for (auto iter = mRTPMedias.begin(); iter != mRTPMedias.end(); ++iter) {
          auto &mediaInfo = (*iter).second;

          close(*mediaInfo);
        }

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &transportInfo = (*iter).second;

          close(*transportInfo);
        }

        for (auto iter = mTransportPool.begin(); iter != mTransportPool.end(); ++iter) {
          auto &transportInfo = (*iter);

          close(*transportInfo);
        }

        mLocalDescription.reset();
        mPendingLocalDescription.reset();

        mRemoteDescription.reset();
        mPendingRemoteDescription.reset();

        mPendingMethods.clear();
        mAddedPendingAddTracks.clear();
        mPendingAddTracks.clear();
        mPendingRemoveTracks.clear();
        mPendingAddDataChannels.clear();

        mTransports.clear();
        mRTPMedias.clear();
        mSCTPMedias.clear();
        mSenders.clear();
        mReceivers.clear();

        mMediaStreams.clear();
        mPendingRemoteCandidates.clear();

        mTransportPool.clear();

        mExistingIDs.clear();

        mCertificatePromise.reset();

        setState(InternalState_Shutdown);


#define TODO_CANCEL 1
#define TODO_CANCEL 2

        mGracefulShutdownReference.reset();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::setError(WORD errorCode, const char *errorReason)
      {
        String errorReasonStr(errorReason);
        if (isStopped()) {
          ZS_LOG_WARNING(Trace, log("cannot set error while already stopped") + ZS_PARAM("error", errorCode) + ZS_PARAM("reason", errorReasonStr));
          return;
        }

        if (0 != errorCode) {
          ZS_LOG_WARNING(Trace, log("cannot set error as error was already set") + ZS_PARAM("error", errorCode) + ZS_PARAM("reason", errorReasonStr));
          return;
        }

        mErrorCode = errorCode;
        if (errorReasonStr.isEmpty()) {
          errorReasonStr = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
        }
        mErrorReason = errorReasonStr;

        ZS_LOG_ERROR(Debug, log("error set") + ZS_PARAM("error", mErrorCode) + ZS_PARAM("reason", mErrorReason));
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
        if (!stepCertificates()) return;
        if (!stepProcessRemote()) return;
        if (!stepProcessLocal()) return;
        if (!stepAddTracks()) return;
        if (!stepAddSCTPTransport()) return;
        if (!stepCreateOfferOrAnswer()) return;
        if (!stepProcessPendingRemoteCandidates()) return;
        if (!stepFinalizeSenders()) return;
        if (!stepFinalizeDataChannels()) return;
        if (!stepFixGathererState()) return;
        if (!stepFixTransportState()) return;

        if (IPeerConnectionTypes::PeerConnectionState_Connected != mLastPeerConnectionState) return;

        goto ready;

      ready:
        {
          setState(InternalState_Ready);
        }
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepCertificates()
      {
        ZS_LOG_TRACE(log("step - certificates"));

        if (mConfiguration.mCertificates.size() > 0) {
          ZS_LOG_TRACE(log("already have certificates"));
          return true;
        }

        if (!mCertificatePromise) {
          mCertificatePromise = ICertificate::generateCertificate();
          if (!mCertificatePromise) {
            setError(UseHTTP::HTTPStatusCode_CertError, "unable to generate a certificate");
            cancel();
            return false;
          }

          mCertificatePromise->thenWeak(mThisWeak.lock());
        }

        if (!mCertificatePromise->isSettled()) {
          ZS_LOG_TRACE(log("certificate promise is not settled yet"));
          return false;
        }

        if (mCertificatePromise->isRejected()) {
          setError(UseHTTP::HTTPStatusCode_CertError, "certificate promise was rejected");
          cancel();
          return false;
        }

        auto certificate = mCertificatePromise->value();
        if (!certificate) {
          setError(UseHTTP::HTTPStatusCode_CertError, "certificate promise was resolved but no promise was returned");
          cancel();
          return false;
        }

        mConfiguration.mCertificates.push_back(certificate);

        mCertificatePromise.reset();
        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessRemote()
      {
        ZS_LOG_TRACE(log("step - process remote"));

        if (mPendingMethods.size() < 1) {
          ZS_LOG_TRACE(log("skipping step to proces remote (no pending methods)"));
          return true;
        }

        PendingMethodPtr pending = mPendingMethods.front();
        if (PendingMethod_SetRemoteDescription != pending->mMethod) {
          ZS_LOG_TRACE(log("skipping process remote as pending method is not setting a remote description"));
          return true;
        }

        mPendingMethods.pop_front();

        auto description = pending->mSessionDescription->description();
        if (!description) {
          flushRemotePending(pending->mSessionDescription);
          ZS_LOG_WARNING(Debug, log("cannot accept remote description (as remote description is not valid)") + ZS_PARAM("signaling state", IPeerConnectionTypes::toString(mLastSignalingState)) + pending->toDebug());
          pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "received SetRemoteDescription but it does not appear to be valid"));
          wake();
          return false;
        }

        auto nextSignalingState = mLastSignalingState;

        switch (mLastSignalingState) {
          case IPeerConnectionTypes::SignalingState_HaveLocalPranswer:
          case IPeerConnectionTypes::SignalingState_HaveRemoteOffer:      {
            flushRemotePending(pending->mSessionDescription);
            ZS_LOG_WARNING(Debug, log("cannot accept remote description (as in wrong signaling state)") + ZS_PARAM("signaling state", IPeerConnectionTypes::toString(mLastSignalingState)) + pending->toDebug());
            pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "received SetRemoteDescription (but signaling state does not match)"));
            wake();
            return false;
          }
          case IPeerConnectionTypes::SignalingState_Stable:
          {
            // always in stable state with JSON
            switch (pending->mSessionDescription->type()) {
              case ISessionDescriptionTypes::SignalingType_JSON:        nextSignalingState = SignalingState_Stable; break;
              case ISessionDescriptionTypes::SignalingType_SDPOffer:    nextSignalingState = SignalingState_HaveRemoteOffer; break;
              case ISessionDescriptionTypes::SignalingType_SDPPranswer:
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:
              {
                flushRemotePending(pending->mSessionDescription);
                ZS_LOG_WARNING(Debug, log("cannot accept remote description (not legal signaling type in Stable state)") + pending->toDebug());
                pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "received SetRemoteDescription (but signaling type not legal)"));
                wake();
                return false;
              }
              case ISessionDescriptionTypes::SignalingType_SDPRollback:
              {
                ZS_THROW_NOT_IMPLEMENTED("rollback not implemented at this time");
              }
            }
            break;
          }
          case IPeerConnectionTypes::SignalingState_HaveLocalOffer:   
          case IPeerConnectionTypes::SignalingState_HaveRemotePranswer:
          {
            switch (pending->mSessionDescription->type()) {
              case ISessionDescriptionTypes::SignalingType_SDPPranswer: nextSignalingState = SignalingState_HaveRemotePranswer; break;
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:   nextSignalingState = SignalingState_Stable; break;
              case ISessionDescriptionTypes::SignalingType_SDPOffer:
              case ISessionDescriptionTypes::SignalingType_JSON:
              {
                flushRemotePending(pending->mSessionDescription);
                ZS_LOG_WARNING(Debug, log("cannot accept remote description (not legal signaling type in HaveLocalOffer state)") + pending->toDebug());
                pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "received SetRemoteDescription (but signaling type not legal)"));
                wake();
                return false;
              }
              case ISessionDescriptionTypes::SignalingType_SDPRollback:
              {
                ZS_THROW_NOT_IMPLEMENTED("rollback not implemented at this time");
              }
            }
            break;
          }
          case IPeerConnectionTypes::SignalingState_Closed:               break;  // will not happen
        }

        bool result = true;

        result = stepProcessRemoteTransport(description) && (result);
        result = stepProcessRemoteRTPMediaLines(description) && (result);
        result = stepProcessRemoteRTPSenders(description) && (result);
        result = stepProcessRemoteSCTPTransport(description) && (result);

        ZS_LOG_DETAIL(log("processed remote description") + pending->toDebug());

        mRemoteDescription = pending->mSessionDescription;
        flushRemotePending(pending->mSessionDescription);
        setState(nextSignalingState);

        if (pending->mPromise) {
          pending->mPromise->resolve();
          pending->mPromise.reset();
        }
        close(*pending);

        onWake(); // redo step
        return false;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessRemoteTransport(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mTransports.begin(); iter != description->mTransports.end(); ++iter) {
          auto &transport = *(*iter);

          TransportInfoPtr transportInfo;

          {
            // find the transport
            {
              auto found = mTransports.find(transport.mID);
              if (found == mTransports.end()) {
                // no matching local transport, create one
                transportInfo = getTransportFromPool(transport.mID);
                transportInfo->mNegotiationState = NegotiationState_RemoteOffered;
              } else {
                transportInfo = (*found).second;
              }
            }

            ZS_THROW_INVALID_ASSUMPTION_IF(!transportInfo);

            if (NegotiationState_Rejected == transportInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("transport was already rejected") + transportInfo->toDebug());
              goto reject_transport;
            }

            if (!transport.mRTP) {
              ZS_LOG_WARNING(Debug, log("transport is missing rtp parameters") + transport.toDebug());
              goto reject_transport;
            }

            if (!transport.mRTP->mICEParameters) {
              ZS_LOG_WARNING(Debug, log("transport is missing ice parameters") + transport.toDebug());
              goto reject_transport;
            }

            if (!transportInfo->mRTP.mGatherer) {
              ZS_LOG_WARNING(Debug, log("transport info is missing ice gatherer") + transportInfo->toDebug());
              goto reject_transport;
            }
            if (!transportInfo->mRTP.mTransport) {
              ZS_LOG_WARNING(Debug, log("transport info is missing ice transport") + transportInfo->toDebug());
              goto reject_transport;
            }

            IICETransportTypes::Options options;
            options.mAggressiveICE = true;

            bool useRTCPMux = (IPeerConnectionTypes::RTCPMuxPolicy_Require == mConfiguration.mRTCPMuxPolicy);
            if (useRTCPMux) {
              useRTCPMux = useRTCPMux || transport.mUseMux;
            }

            bool hasRTPICE = true;
            bool hasRTCPICE = hasRTPICE && transport.mRTCP;
            auto useRTCPICEParams = hasRTCPICE ? (bool(transport.mRTCP->mICEParameters) ? transport.mRTCP->mICEParameters : transport.mRTP->mICEParameters) : IICETransportTypes::ParametersPtr();

            bool hasRTPDTLS = bool(transport.mRTP->mDTLSParameters);
            bool hasRTCPDTLS = hasRTCPICE && hasRTPDTLS && bool(transport.mRTCP);
            auto useRTCPDTLSParams = hasRTCPDTLS ? (transport.mRTCP->mDTLSParameters ? transport.mRTCP->mDTLSParameters : transport.mRTP->mDTLSParameters) : IDTLSTransportTypes::ParametersPtr();

            bool hasRTPSRTP = (!hasRTPDTLS) && bool(transport.mRTP->mSRTPSDESParameters) && (transport.mRTP->mSRTPSDESParameters->mCryptoParams.size() > 0);

            if ((!hasRTPDTLS) && (!hasRTPSRTP)) {
              ZS_LOG_WARNING(Debug, log("transport / info is missing secure transport") + transport.toDebug() + transportInfo->toDebug());
              goto reject_transport;
            }

            if (useRTCPMux) {
              hasRTCPICE = false;
              hasRTCPDTLS = false;
            }

            if (hasRTPDTLS) {
              switch (transport.mRTP->mDTLSParameters->mRole) {
                case IDTLSTransportTypes::Role_Auto: {
                  if (IPeerConnectionTypes::SignalingMode_SDP == mConfiguration.mSignalingMode) {
                    transportInfo->mLocalDTLSRole = IDTLSTransportTypes::Role_Client;
                    transport.mRTP->mDTLSParameters->mRole = IDTLSTransportTypes::Role_Server;
                    if (hasRTCPDTLS) {
                      transport.mRTP->mDTLSParameters->mRole = IDTLSTransportTypes::Role_Server;
                    }
                  }
                  break;
                }
                case IDTLSTransportTypes::Role_Client: {
                  transportInfo->mLocalDTLSRole = IDTLSTransportTypes::Role_Server;
                  break;
                }
                case IDTLSTransportTypes::Role_Server: {
                  transportInfo->mLocalDTLSRole = IDTLSTransportTypes::Role_Client;
                  break;
                }
              }
            }

            switch (transportInfo->mNegotiationState)
            {
              case NegotiationState_PendingOffer:
              case NegotiationState_LocalOffered:     options.mRole = IICETypes::Role_Controlling; break;
              case NegotiationState_RemoteOffered:    options.mRole = IICETypes::Role_Controlled;  break;
              case NegotiationState_Agreed:           break;
              case NegotiationState_Rejected:         break;  // will not happen
            }

            try {
              // scope: setup ICE
              {
                transportInfo->mRTP.mTransport->start(transportInfo->mRTP.mGatherer, *(transport.mRTP->mICEParameters), options);
                if (hasRTCPICE) {
                  transportInfo->mRTCP.mTransport->start(transportInfo->mRTCP.mGatherer, *useRTCPICEParams, options);
                } else {
                  if (transportInfo->mRTCP.mTransport) {
                    transportInfo->mRTCP.mTransport->stop();
                    transportInfo->mRTCP.mTransport.reset();
                  }
                  if (transportInfo->mRTCP.mGatherer) {
                    transportInfo->mRTCP.mGatherer->close();
                    transportInfo->mRTCP.mGatherer.reset();
                  }
                }
              }

              // scope: setup DTLS
              {
                if (hasRTPDTLS) {
                  if (!(transportInfo->mRTP.mDTLSTransport)) {
                    transportInfo->mRTP.mDTLSTransport = IDTLSTransport::create(mThisWeak.lock(), transportInfo->mRTP.mTransport, transportInfo->mCertificates);
                  }
                  transportInfo->mRTP.mDTLSTransport->start(*(transport.mRTP->mDTLSParameters));
                } else {
                  if (transportInfo->mRTP.mDTLSTransport) {
                    transportInfo->mRTP.mDTLSTransport->stop();
                    transportInfo->mRTP.mDTLSTransport.reset();
                  }
                }
                if (hasRTCPDTLS) {
                  if (!(transportInfo->mRTCP.mDTLSTransport)) {
                    transportInfo->mRTCP.mDTLSTransport = IDTLSTransport::create(mThisWeak.lock(), transportInfo->mRTCP.mTransport, transportInfo->mCertificates);
                  }
                  transportInfo->mRTCP.mDTLSTransport->start(*useRTCPDTLSParams);
                } else {
                  if (transportInfo->mRTCP.mDTLSTransport) {
                    transportInfo->mRTCP.mDTLSTransport->stop();
                    transportInfo->mRTCP.mDTLSTransport.reset();
                  }
                }
              }

              // scope: setup SRTP/SDES
              {
                if ((hasRTPSRTP) &&
                    (mConfiguration.mNegotiateSRTPSDES)) {
                  if (!(transportInfo->mRTP.mSRTPSDESTransport)) {
                    ZS_THROW_INVALID_ASSUMPTION_IF(!transportInfo->mRTP.mSRTPSDESParameters);
                    ZS_THROW_INVALID_ASSUMPTION_IF(transportInfo->mRTP.mSRTPSDESParameters->mCryptoParams.size() < 1);

                    auto &encodeCryptoParams = *(transportInfo->mRTP.mSRTPSDESParameters->mCryptoParams.begin());
                    auto &decodeCryptoParams = *(transport.mRTP->mSRTPSDESParameters->mCryptoParams.begin());
                    transportInfo->mRTP.mSRTPSDESTransport = ISRTPSDESTransport::create(mThisWeak.lock(), transportInfo->mRTP.mTransport, encodeCryptoParams, decodeCryptoParams);
                  }
                } else {
                  if (transportInfo->mRTP.mSRTPSDESTransport) {
                    transportInfo->mRTP.mSRTPSDESTransport->stop();
                    transportInfo->mRTP.mSRTPSDESTransport.reset();
                  }
                }
              }

              for (auto iterCan = transport.mRTP->mICECandidates.begin(); iterCan != transport.mRTP->mICECandidates.end(); ++iterCan) {
                auto &candidate = *(*iterCan);
                transportInfo->mRTP.mTransport->addRemoteCandidate(candidate);
              }

              if (transport.mRTP->mEndOfCandidates) {
                auto candidateComplete = make_shared<IICETypes::CandidateComplete>();
                transportInfo->mRTP.mTransport->addRemoteCandidate(*candidateComplete);
              }

              if ((transport.mRTCP) &&
                  (transportInfo->mRTCP.mTransport)) {
                for (auto iterCan = transport.mRTCP->mICECandidates.begin(); iterCan != transport.mRTCP->mICECandidates.end(); ++iterCan) {
                  auto &candidate = *(*iterCan);
                  transportInfo->mRTCP.mTransport->addRemoteCandidate(candidate);
                }
                if (transport.mRTCP->mEndOfCandidates) {
                  auto candidateComplete = make_shared<IICETypes::CandidateComplete>();
                  candidateComplete->mComponent = IICETypes::Component_RTCP;
                  transportInfo->mRTCP.mTransport->addRemoteCandidate(*candidateComplete);
                }
              }

            } catch (const InvalidParameters &) {
              ZS_LOG_WARNING(Debug, log("invalid parameters exception caught"));
              goto reject_transport;
            } catch (const InvalidStateError &) {
              ZS_LOG_WARNING(Debug, log("invalid state exception caught"));
              goto reject_transport;
            }

            goto accept_transport;
          }

        reject_transport:
          {
            if (!transportInfo) continue;
            close(*transportInfo);
            continue;
          }

        accept_transport:
          {
            switch (transportInfo->mNegotiationState)
            {
              case NegotiationState_PendingOffer:
              case NegotiationState_LocalOffered:     transportInfo->mNegotiationState = NegotiationState_Agreed; break;

              case NegotiationState_RemoteOffered:    notifyNegotiationNeeded(); break;
              case NegotiationState_Agreed:           break;
              case NegotiationState_Rejected:         break;  // will not happen
            }

          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessRemoteRTPMediaLines(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mRTPMediaLines.begin(); iter != description->mRTPMediaLines.end(); ++iter)
        {
          auto &mediaLine = *(*iter);

          RTPMediaLineInfoPtr mediaLineInfo;

          // scope prepare media line
          {
            // scope: find the media line
            {
              if (mediaLine.mID.hasData()) {
                auto found = mRTPMedias.find(mediaLine.mID);
                if (found != mRTPMedias.end()) {
                  mediaLineInfo = (*found).second;
                }
              }

              if (!mediaLineInfo) {
                mediaLineInfo = make_shared<RTPMediaLineInfo>();
                mediaLineInfo->mID = mediaLine.mID.hasData() ? registerIDUsage(mediaLine.mID) : registerNewID();
                mediaLineInfo->mLineIndex = mediaLine.mDetails ? mediaLine.mDetails->mInternalIndex : Optional<size_t>();
                mediaLineInfo->mNegotiationState = NegotiationState_RemoteOffered;
                mediaLineInfo->mIDPreference = UseAdapterHelper::IDPreference_Remote;
                mRTPMedias[mediaLineInfo->mID] = mediaLineInfo;
              }
            }

            if (NegotiationState_Rejected == mediaLineInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("media line previously rejected") + mediaLineInfo->toDebug());
              goto reject_media_line;
            }

            if (mediaLineInfo->mMediaType.hasData()) {
              if (0 != mediaLineInfo->mMediaType.compareNoCase(mediaLine.mMediaType)) {
                ZS_LOG_WARNING(Detail, log("media line cannot change type"));
                goto reject_media_line;
              }
            }

            mediaLineInfo->mMediaType = mediaLine.mMediaType;
            mediaLineInfo->mBundledTransportID = mediaLine.mTransportID;
            mediaLineInfo->mPrivateTransportID = mediaLine.mDetails ? mediaLine.mDetails->mPrivateTransportID : String();
            mediaLineInfo->mRemoteSenderCapabilities = make_shared<IRTPTypes::Capabilities>(*mediaLine.mSenderCapabilities);
            mediaLineInfo->mRemoteReceiverCapabilities = make_shared<IRTPTypes::Capabilities>(*mediaLine.mReceiverCapabilities);
            mediaLineInfo->mBundledTransportID = mediaLine.mTransportID;
            mediaLineInfo->mPrivateTransportID = mediaLine.mDetails ? mediaLine.mDetails->mPrivateTransportID : String();

            if ((!mediaLineInfo->mRemoteSenderCapabilities) ||
                (!mediaLineInfo->mRemoteReceiverCapabilities)) {
              ZS_LOG_WARNING(Detail, log("media line is missing remote capabilities") + mediaLine.toDebug());
              goto reject_media_line;
            }

            bool foundTransport = false;
            if (mediaLineInfo->mBundledTransportID.hasData()) {
              auto found = mTransports.find(mediaLineInfo->mBundledTransportID);
              if (found != mTransports.end()) {
                auto &transport = *((*found).second);
                if (NegotiationState_Rejected != transport.mNegotiationState) foundTransport = true;
              }
            }
            if (mediaLineInfo->mPrivateTransportID.hasData()) {
              auto found = mTransports.find(mediaLineInfo->mPrivateTransportID);
              if (found != mTransports.end()) {
                auto &transport = *((*found).second);
                if (NegotiationState_Rejected != transport.mNegotiationState) foundTransport = true;
              }
            }

            if (!foundTransport) {
              ZS_LOG_WARNING(Detail, log("did not found any transport for media line"));
              goto reject_media_line;
            }

            mediaLineInfo->mLocalReceiverCapabilities = UseAdapterHelper::createUnion(mediaLineInfo->mLocalReceiverCapabilities ? (*mediaLineInfo->mLocalReceiverCapabilities) : (*IRTPReceiver::getCapabilities(IMediaStreamTrack::toKind(mediaLine.mMediaType))), *mediaLineInfo->mRemoteSenderCapabilities, mediaLineInfo->mIDPreference);
            mediaLineInfo->mLocalSenderCapabilities = UseAdapterHelper::createUnion(mediaLineInfo->mLocalSenderCapabilities ? (*mediaLineInfo->mLocalSenderCapabilities) : (*IRTPSender::getCapabilities(IMediaStreamTrack::toKind(mediaLine.mMediaType))), *mediaLineInfo->mRemoteSenderCapabilities, mediaLineInfo->mIDPreference);

            if (!UseAdapterHelper::hasSupportedMediaCodec(*mediaLineInfo->mLocalReceiverCapabilities)) {
              ZS_LOG_WARNING(Detail, log("no supported media codec found") + mediaLineInfo->toDebug());
              goto reject_media_line;
            }

            goto accept_media_line;
          }

        reject_media_line:
          {
            if (!mediaLineInfo) continue;
            close(*mediaLineInfo);
            continue;
          }

        accept_media_line:
          {
            switch (mediaLineInfo->mNegotiationState) {
              case NegotiationState_PendingOffer:                                         // offered, now agreed
              case NegotiationState_LocalOffered:     {                                   // offered, now agreed
                mediaLineInfo->mNegotiationState = NegotiationState_Agreed;
                break;
              }
              case NegotiationState_Agreed:           break;                              // no change needed
              case NegotiationState_RemoteOffered:    notifyNegotiationNeeded(); break;   // requires an offer/answer be done
              case NegotiationState_Rejected:         break;                              // not possible
            }
            continue;
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessRemoteRTPSenders(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mRTPSenders.begin(); iter != description->mRTPSenders.end(); ++iter) {
          auto &sender = *(*iter);

          ReceiverInfoPtr receiverInfo;
          bool eventReceiver = false;

          // prepare receivers
          {
            // scope: find receiver
            {
              if (!sender.mID.hasData()) goto reject_sender;

              auto found = mReceivers.find(sender.mID);
              if (found != mReceivers.end()) {
                receiverInfo = (*found).second;
              }
            }

            if (!receiverInfo) {
              receiverInfo = make_shared<ReceiverInfo>();
              receiverInfo->mID = registerIDUsage(sender.mID);
              receiverInfo->mMediaLineID = sender.mRTPMediaLineID;
              receiverInfo->mMediaStreamTrackID = sender.mMediaStreamTrackID;
              receiverInfo->mNegotiationState = NegotiationState_RemoteOffered;
              mReceivers[receiverInfo->mID] = receiverInfo;
            }

            if (NegotiationState_Rejected == receiverInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("already rejected remote sender") + sender.toDebug());
              goto reject_sender;
            }

            RTPMediaLineInfoPtr mediaLine;

            {
              auto found = mRTPMedias.find(receiverInfo->mMediaLineID);
              if (found != mRTPMedias.end()) mediaLine = (*found).second;
            }

            if (!mediaLine) {
              ZS_LOG_WARNING(Detail, log("did not find associated media line"));
              goto reject_sender;
            }

            if (NegotiationState_Rejected == mediaLine->mNegotiationState) {
              ZS_LOG_WARNING(Detail, log("media line was rejected thus remote sender must be rejected") + sender.toDebug() + mediaLine->toDebug());
              goto reject_sender;
            }

            TransportInfoPtr transportInfo;

            {
              auto found = mTransports.find(mediaLine->mBundledTransportID);
              if (found != mTransports.end()) transportInfo = (*found).second;

              if ((!transportInfo) &&
                  (mediaLine->mPrivateTransportID.hasData())) {
                found = mTransports.find(mediaLine->mPrivateTransportID);
                if (found != mTransports.end()) transportInfo = (*found).second;
              }
            }

            if (!transportInfo) {
              ZS_LOG_WARNING(Detail, log("did not find associated transport"));
              goto reject_sender;
            }

            if (NegotiationState_Rejected == transportInfo->mNegotiationState) {
              ZS_LOG_WARNING(Detail, log("transport was rejected thus remote sender must be rejected") + sender.toDebug() + transportInfo->toDebug());
              goto reject_sender;
            }

            if ((!mediaLine->mLocalReceiverCapabilities) ||
                (!mediaLine->mRemoteSenderCapabilities)) {
              ZS_LOG_WARNING(Detail, log("media line is missing capabilities") + mediaLine->toDebug());
              goto reject_sender;
            }

            if (!sender.mParameters) {
              ZS_LOG_WARNING(Detail, log("sender is missing parameters") + sender.toDebug());
              goto reject_sender;
            }

            auto unionCaps = UseAdapterHelper::createUnion(*(mediaLine->mLocalReceiverCapabilities), *(mediaLine->mRemoteSenderCapabilities), mediaLine->mIDPreference);
            if (!UseAdapterHelper::hasSupportedMediaCodec(*unionCaps)) {
              ZS_LOG_WARNING(Detail, log("union of remote sender / local receiver capabilities does not produce via codec match") + unionCaps->toDebug() + mediaLine->toDebug());
              goto reject_sender;
            }

            auto filteredParams = UseAdapterHelper::filterParameters(*sender.mParameters, *unionCaps);
            if ((!UseAdapterHelper::isCompatible(*unionCaps, *filteredParams)) ||
                (!UseAdapterHelper::hasSupportedMediaCodec(*filteredParams))) {
              ZS_LOG_WARNING(Detail, log("sender parameters or capabililties are not compatible or has no supported codec") + filteredParams->toDebug() + unionCaps->toDebug() + mediaLine->toDebug());
              goto reject_sender;
            }

            if (!receiverInfo->mReceiver) {
              receiverInfo->mReceiver = IRTPReceiver::create(
                                                             mThisWeak.lock(),
                                                             IMediaStreamTrackTypes::toKind(mediaLine->mMediaType),
                                                             transportInfo->mRTP.mDTLSTransport ? IRTPTransportPtr(transportInfo->mRTP.mDTLSTransport) : IRTPTransportPtr(transportInfo->mRTP.mSRTPSDESTransport),
                                                             transportInfo->mRTP.mDTLSTransport ? IRTCPTransportPtr(transportInfo->mRTCP.mDTLSTransport) : IRTCPTransportPtr(transportInfo->mRTCP.mTransport)
                                                             );
              eventReceiver = true;
            }

            try {
              fillRTCPSSRC(*filteredParams);
              receiverInfo->mReceiver->receive(*filteredParams);
            } catch (const InvalidParameters &) {
              ZS_LOG_WARNING(Debug, log("invalid parameters when calling receiver.receive()"));
              goto reject_sender;
            } catch (const InvalidStateError &) {
              ZS_LOG_WARNING(Debug, log("invalid state when calling receiver.receive()"));
              goto reject_sender;
            }


            auto existingSet = convertToSet(receiverInfo->mMediaStreams);

            MediaStreamSet added;
            MediaStreamSet removed;

            calculateDelta(*existingSet, sender.mMediaStreamIDs, added, removed);

            for (auto iter = added.begin(); iter != added.end(); ++iter) {
              auto &id = (*iter);

              UseMediaStreamPtr stream;

              auto found = mMediaStreams.find(id);
              if (found == mMediaStreams.end()) {
                stream = UseMediaStream::create(id);
                mMediaStreams[id] = stream;
              } else {
                stream = (*found).second;
              }

              receiverInfo->mMediaStreams[id] = stream;

              stream->notifyAddTrack(receiverInfo->mReceiver->track());
            }

            for (auto iter = removed.begin(); iter != removed.end(); ++iter) {
              auto &id = (*iter);

              UseMediaStreamPtr stream;

              {
                auto found = mMediaStreams.find(id);
                if (found != mMediaStreams.end()) {
                  stream = (*found).second;
                }
              }
              {
                auto found = receiverInfo->mMediaStreams.find(id);
                if (found != receiverInfo->mMediaStreams.end()) {
                  stream = (*found).second;
                  receiverInfo->mMediaStreams.erase(found);
                }
              }
              if (stream) {
                stream->notifyRemoveTrack(receiverInfo->mReceiver->track());
              }
            }

            purgeNonReferencedAndEmptyStreams();
            goto accept_sender;
          }

        reject_sender:
          {
            if (!receiverInfo) continue;
            close(*receiverInfo);
            continue;
          }

        accept_sender:
          {
            switch (receiverInfo->mNegotiationState) {
              case NegotiationState_PendingOffer:     break;                              // not possible
              case NegotiationState_LocalOffered:     break;                              // not possible
              case NegotiationState_RemoteOffered:    {                                   // offered, now agreed
                receiverInfo->mNegotiationState = NegotiationState_Agreed;
                break;
              }
              case NegotiationState_Agreed:           break;                              // no change needed
              case NegotiationState_Rejected:         break;                              // not possible
            }
            if (eventReceiver) {
              ZS_LOG_DEBUG(log("new receiver created"));
              MediaStreamTrackEventPtr evt(make_shared<MediaStreamTrackEvent>());
              evt->mReceiver = receiverInfo->mReceiver;
              evt->mTrack = evt->mReceiver->track();
              evt->mMediaStreams = *convertToList(receiverInfo->mMediaStreams);
              mSubscriptions.delegate()->onPeerConnectionTrack(mThisWeak.lock(), evt);
            }
            continue;
          }

        }
        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessRemoteSCTPTransport(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mSCTPMediaLines.begin(); iter != description->mSCTPMediaLines.end(); ++iter)
        {
          auto &mediaLine = *(*iter);

          SCTPMediaLineInfoPtr mediaLineInfo;

          // scope prepare media line
          {
            // scope: find the media line
            {
              if (mediaLine.mID.hasData()) {
                auto found = mSCTPMedias.find(mediaLine.mID);
                if (found != mSCTPMedias.end()) {
                  mediaLineInfo = (*found).second;
                }
              }

              if (!mediaLineInfo) {
                mediaLineInfo = make_shared<SCTPMediaLineInfo>();
                mediaLineInfo->mID = mediaLine.mID.hasData() ? registerIDUsage(mediaLine.mID) : registerNewID();
                mediaLineInfo->mLineIndex = mediaLine.mDetails ? mediaLine.mDetails->mInternalIndex : Optional<size_t>();
                mediaLineInfo->mNegotiationState = NegotiationState_RemoteOffered;
                mSCTPMedias[mediaLineInfo->mID] = mediaLineInfo;
              }
            }

            if (NegotiationState_Rejected == mediaLineInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("media line previously rejected") + mediaLineInfo->toDebug());
              goto reject_media_line;
            }

            if (!mediaLine.mCapabilities) {
              ZS_LOG_WARNING(Debug, log("media line is missing remote capabilities") + mediaLineInfo->toDebug());
              goto reject_media_line;
            }

            if (0 == mediaLine.mPort) {
              ZS_LOG_WARNING(Detail, log("SCTP port is not specified thus sctp transport must be rejected") + mediaLineInfo->toDebug() + mediaLine.toDebug());
              goto reject_media_line;
            }

            mediaLineInfo->mBundledTransportID = mediaLine.mTransportID;
            mediaLineInfo->mPrivateTransportID = mediaLine.mDetails ? mediaLine.mDetails->mPrivateTransportID : String();
            mediaLineInfo->mRemoteCapabilities = make_shared<ISCTPTransportTypes::Capabilities>(*mediaLine.mCapabilities);

            TransportInfoPtr transportInfo;

            {
              auto found = mTransports.find(mediaLineInfo->mBundledTransportID);
              if (found != mTransports.end()) transportInfo = (*found).second;

              if ((!transportInfo) &&
                (mediaLineInfo->mPrivateTransportID.hasData())) {
                found = mTransports.find(mediaLineInfo->mPrivateTransportID);
                if (found != mTransports.end()) transportInfo = (*found).second;
              }
            }

            if (!transportInfo) {
              ZS_LOG_WARNING(Detail, log("did not find associated transport"));
              goto reject_media_line;
            }

            if (NegotiationState_Rejected == transportInfo->mNegotiationState) {
              ZS_LOG_WARNING(Detail, log("transport was rejected thus remote sctp must be rejected") + mediaLineInfo->toDebug() + transportInfo->toDebug());
              goto reject_media_line;
            }

            if (!transportInfo->mRTP.mDTLSTransport) {
              ZS_LOG_WARNING(Detail, log("transport is not DTLS based and thus must be rejected") + mediaLineInfo->toDebug() + transportInfo->toDebug());
              goto reject_media_line;
            }

            try {
              if (!mediaLineInfo->mSCTPTransport) {
                if (!mediaLineInfo->mLocalPort.hasValue()) {
                  mediaLineInfo->mLocalPort = mediaLine.mPort;
                  mExistingLocalPorts.insert(mediaLine.mPort);
                }
                mediaLineInfo->mSCTPTransport = ISCTPTransport::create(mThisWeak.lock(), transportInfo->mRTP.mDTLSTransport, mediaLine.mPort);
              }
              if (!mediaLineInfo->mRemotePort.hasValue()) {
                mediaLineInfo->mRemotePort = mediaLine.mPort;
                mediaLineInfo->mSCTPTransport->start(*mediaLine.mCapabilities, mediaLine.mPort);
              }
            } catch (const InvalidParameters &e) {
              ZS_LOG_WARNING(Detail, log("sctp transport cannot be constructed and started") + ZS_PARAM("invalid parameters exception", e.what()));
              goto reject_media_line;
            } catch (const InvalidStateError &e) {
              ZS_LOG_WARNING(Detail, log("sctp transport cannot be constructed and started") + ZS_PARAM("invalid state exception", e.what()));
              goto reject_media_line;
            }

            goto accept_media_line;
          }

        reject_media_line:
          {
            if (!mediaLineInfo) continue;
            close(*mediaLineInfo);
            continue;
          }

        accept_media_line:
          {
            switch (mediaLineInfo->mNegotiationState) {
              case NegotiationState_PendingOffer:                                         // offered, now agreed
              case NegotiationState_LocalOffered: {                                       // offered, now agreed
                mediaLineInfo->mNegotiationState = NegotiationState_Agreed;
                break;
              }
              case NegotiationState_Agreed:           break;                              // no change needed
              case NegotiationState_RemoteOffered:    notifyNegotiationNeeded(); break;   // requires an offer/answer be done
              case NegotiationState_Rejected:         break;                              // not possible
            }
            continue;
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessLocal()
      {
        ZS_LOG_TRACE(log("step - process local"));

        if (mPendingMethods.size() < 1) {
          ZS_LOG_TRACE(log("skipping step to proces local (no pending methods)"));
          return true;
        }

        PendingMethodPtr pending = mPendingMethods.front();
        if (PendingMethod_SetLocalDescription != pending->mMethod) {
          ZS_LOG_TRACE(log("skipping process local as pending method is not setting a remote description"));
          return true;
        }

        mPendingMethods.pop_front();

        auto description = pending->mSessionDescription->description();
        if (!description) {
          flushLocalPending(pending->mSessionDescription);
          ZS_LOG_WARNING(Debug, log("cannot accept remote description (as remote description is not valid)") + ZS_PARAM("signaling state", IPeerConnectionTypes::toString(mLastSignalingState)) + pending->toDebug());
          pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "received SetRemoteDescription but it does not appear to be valid"));
          wake();
          return false;
        }

        auto nextSignalingState = mLastSignalingState;

        switch (mLastSignalingState) {
          case IPeerConnectionTypes::SignalingState_HaveRemotePranswer:
          case IPeerConnectionTypes::SignalingState_HaveLocalOffer:      {
            flushLocalPending(pending->mSessionDescription);
            ZS_LOG_WARNING(Debug, log("cannot accept local description (as in wrong signaling state)") + ZS_PARAM("signaling state", IPeerConnectionTypes::toString(mLastSignalingState)) + pending->toDebug());
            pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "received SetRemoteDescription (but signaling state does not match)"));
            wake();
            return false;
          }
          case IPeerConnectionTypes::SignalingState_Stable:
          {
            // always in stable state with JSON
            switch (pending->mSessionDescription->type()) {
              case ISessionDescriptionTypes::SignalingType_JSON:          nextSignalingState = SignalingState_Stable; break;
              case ISessionDescriptionTypes::SignalingType_SDPOffer:      nextSignalingState = SignalingState_HaveLocalOffer; break;
              case ISessionDescriptionTypes::SignalingType_SDPPranswer:
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:
              {
                flushLocalPending(pending->mSessionDescription);
                ZS_LOG_WARNING(Debug, log("cannot accept local description (not legal signaling type in Stable state)") + pending->toDebug());
                pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "received SetRemoteDescription (but signaling type not legal)"));
                wake();
                return false;
              }
              case ISessionDescriptionTypes::SignalingType_SDPRollback:
              {
                ZS_THROW_NOT_IMPLEMENTED("rollback not implemented at this time");
              }
            }
            break;
          }
          case IPeerConnectionTypes::SignalingState_HaveRemoteOffer:
          case IPeerConnectionTypes::SignalingState_HaveLocalPranswer:
          {
            switch (pending->mSessionDescription->type()) {
              case ISessionDescriptionTypes::SignalingType_SDPPranswer: nextSignalingState = SignalingState_HaveLocalPranswer; break;
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:   nextSignalingState = SignalingState_Stable; break;
              case ISessionDescriptionTypes::SignalingType_SDPOffer:
              case ISessionDescriptionTypes::SignalingType_JSON:
              {
                flushLocalPending(pending->mSessionDescription);
                ZS_LOG_WARNING(Debug, log("cannot accept local description (not legal signaling type in HaveRemoteOffer/HaveLocalPreanswer state)") + pending->toDebug());
                pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "received SetRemoteDescription (but signaling type not legal)"));
                wake();
                return false;
              }
              case ISessionDescriptionTypes::SignalingType_SDPRollback:
              {
                ZS_THROW_NOT_IMPLEMENTED("rollback not implemented at this time");
              }
            }
            break;
          }
          case IPeerConnectionTypes::SignalingState_Closed:               break;  // will not happen
        }

        bool result = true;

        result = stepProcessLocalTransport(description) && (result);
        result = stepProcessLocalRTPMediaLines(description) && (result);
        result = stepProcessLocalRTPSenders(description) && (result);
        result = stepProcessLocalSCTPTransport(description) && (result);

        ZS_LOG_DETAIL(log("processed local description") + pending->toDebug());

        mLocalDescription = pending->mSessionDescription;
        flushLocalPending(pending->mSessionDescription);
        setState(nextSignalingState);

        if (pending->mPromise) {
          pending->mPromise->resolve();
          pending->mPromise.reset();
        }
        close(*pending);

        onWake(); // redo step
        return false;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessLocalTransport(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mTransports.begin(); iter != description->mTransports.end(); ++iter) {
          auto &transport = *(*iter);

          TransportInfoPtr transportInfo;

          {
            // find the transport
            {
              auto found = mTransports.find(transport.mID);
              if (found != mTransports.end()) {
                transportInfo = (*found).second;
              }
            }

            if (!transportInfo) {
              ZS_LOG_WARNING(Debug, log("transport was not created") + transportInfo->toDebug());
              goto reject_transport;
            }

            ZS_THROW_INVALID_ASSUMPTION_IF(!transportInfo);

            if (NegotiationState_Rejected == transportInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("transport was already rejected") + transportInfo->toDebug());
              goto reject_transport;
            }

            if (!transportInfo->mRTP.mGatherer) {
              ZS_LOG_WARNING(Debug, log("transport info is missing ice gatherer") + transportInfo->toDebug());
              goto reject_transport;
            }
            if (!transportInfo->mRTP.mTransport) {
              ZS_LOG_WARNING(Debug, log("transport info is missing ice transport") + transportInfo->toDebug());
              goto reject_transport;
            }

            if (NegotiationState_RemoteOffered == transportInfo->mNegotiationState) {
              if (transport.mUseMux) {
                if (transportInfo->mRTCP.mDTLSTransport) {
                  transportInfo->mRTCP.mDTLSTransport->stop();
                  transportInfo->mRTCP.mDTLSTransport.reset();
                }
                if (transportInfo->mRTCP.mTransport) {
                  transportInfo->mRTCP.mTransport->stop();
                  transportInfo->mRTCP.mTransport.reset();
                }
                if (transportInfo->mRTCP.mGatherer) {
                  transportInfo->mRTCP.mGatherer->close();
                  transportInfo->mRTCP.mGatherer.reset();
                }
              }
            }

            goto accept_transport;
          }

        reject_transport:
          {
            if (!transportInfo) continue;
            close(*transportInfo);
            continue;
          }

        accept_transport:
          {
            switch (transportInfo->mNegotiationState)
            {
              case NegotiationState_PendingOffer:     transportInfo->mNegotiationState = NegotiationState_LocalOffered; break;
              case NegotiationState_LocalOffered:     break;

              case NegotiationState_RemoteOffered:    transportInfo->mNegotiationState = NegotiationState_Agreed; break;
              case NegotiationState_Agreed:           break;
              case NegotiationState_Rejected:         break;  // will not happen
            }

          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessLocalRTPMediaLines(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mRTPMediaLines.begin(); iter != description->mRTPMediaLines.end(); ++iter)
        {
          auto &mediaLine = *(*iter);

          RTPMediaLineInfoPtr mediaLineInfo;

          // scope prepare media line
          {
            // scope: find the media line
            {
              if (mediaLine.mID.hasData()) {
                auto found = mRTPMedias.find(mediaLine.mID);
                if (found != mRTPMedias.end()) {
                  mediaLineInfo = (*found).second;
                }
              }

              if (!mediaLineInfo) {
                ZS_LOG_WARNING(Debug, log("media line was not found (thus reject it)") + mediaLineInfo->toDebug());
                goto reject_media_line;
              }
            }

            if (NegotiationState_Rejected == mediaLineInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("media line previously rejected") + mediaLineInfo->toDebug());
              goto reject_media_line;
            }

            if (mediaLineInfo->mMediaType.hasData()) {
              if (0 != mediaLineInfo->mMediaType.compareNoCase(mediaLine.mMediaType)) {
                ZS_LOG_WARNING(Detail, log("media line cannot change type"));
                goto reject_media_line;
              }
            }

            goto accept_media_line;
          }

        reject_media_line:
          {
            if (!mediaLineInfo) continue;
            close(*mediaLineInfo);
            continue;
          }

        accept_media_line:
          {
            switch (mediaLineInfo->mNegotiationState) {
              case NegotiationState_PendingOffer:     mediaLineInfo->mNegotiationState = NegotiationState_LocalOffered; break;
              case NegotiationState_LocalOffered:     break;
              case NegotiationState_Agreed:           break;                              
              case NegotiationState_RemoteOffered:    mediaLineInfo->mNegotiationState = NegotiationState_Agreed; break;
              case NegotiationState_Rejected:         break;                              
            }
            continue;
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessLocalRTPSenders(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mRTPSenders.begin(); iter != description->mRTPSenders.end(); ++iter) {
          auto &sender = *(*iter);

          SenderInfoPtr senderInfo;

          // prepare receivers
          {
            // scope: find receiver
            {
              if (!sender.mID.hasData()) goto reject_sender;

              auto found = mSenders.find(sender.mID);
              if (found != mSenders.end()) {
                senderInfo = (*found).second;
              }
            }

            if (!senderInfo) {
              ZS_LOG_WARNING(Debug, log("local sender was not found") + sender.toDebug());
              goto reject_sender;
            }

            if (NegotiationState_Rejected == senderInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("already rejected local sender") + sender.toDebug());
              goto reject_sender;
            }

            RTPMediaLineInfoPtr mediaLine;

            {
              auto found = mRTPMedias.find(senderInfo->mMediaLineID);
              if (found != mRTPMedias.end()) mediaLine = (*found).second;
            }

            if (!mediaLine) {
              ZS_LOG_WARNING(Detail, log("did not find associated media line"));
              goto reject_sender;
            }

            if (NegotiationState_Rejected == mediaLine->mNegotiationState) {
              ZS_LOG_WARNING(Detail, log("media line was rejected thus local sender must be rejected") + sender.toDebug() + mediaLine->toDebug());
              goto reject_sender;
            }

            TransportInfoPtr transportInfo;

            {
              auto found = mTransports.find(mediaLine->mBundledTransportID);
              if (found != mTransports.end()) transportInfo = (*found).second;

              if ((!transportInfo) &&
                  (mediaLine->mPrivateTransportID.hasData())) {
                found = mTransports.find(mediaLine->mPrivateTransportID);
                if (found != mTransports.end()) transportInfo = (*found).second;
              }
            }

            if (!transportInfo) {
              ZS_LOG_WARNING(Detail, log("did not find associated transport"));
              goto reject_sender;
            }

            if (NegotiationState_Rejected == transportInfo->mNegotiationState) {
              ZS_LOG_WARNING(Detail, log("transport was rejected thus local sender must be rejected") + sender.toDebug() + transportInfo->toDebug());
              goto reject_sender;
            }

            if (!mediaLine->mRemoteReceiverCapabilities) {
              ZS_LOG_DEBUG(log("waiting for remote party to accept media line to be able to start sending"));
              goto accept_sender;
            }

            if ((!mediaLine->mRemoteReceiverCapabilities) ||
                (!mediaLine->mLocalSenderCapabilities)) {
              ZS_LOG_WARNING(Detail, log("media line is missing capabilities") + mediaLine->toDebug());
              goto reject_sender;
            }

            auto unionCaps = UseAdapterHelper::createUnion(*(mediaLine->mLocalSenderCapabilities), *(mediaLine->mRemoteReceiverCapabilities), mediaLine->mIDPreference);
            if (!UseAdapterHelper::hasSupportedMediaCodec(*unionCaps)) {
              ZS_LOG_WARNING(Detail, log("union of remote sender / local receiver capabilities does not produce via codec match") + unionCaps->toDebug() + mediaLine->toDebug());
              goto reject_sender;
            }

            if (!sender.mParameters) {
              ZS_LOG_WARNING(Detail, log("sender parameters are missing") + sender.toDebug());
              goto reject_sender;
            }

            if (!senderInfo->mSender) {
              ZS_LOG_TRACE(log("sender is not ready to start sending yet") + senderInfo->toDebug());
              goto accept_sender;
            }

            auto filteredParams = UseAdapterHelper::filterParameters(*sender.mParameters, *unionCaps);
            if ((!UseAdapterHelper::isCompatible(*unionCaps, *filteredParams)) ||
                (!UseAdapterHelper::hasSupportedMediaCodec(*filteredParams))) {
              ZS_LOG_WARNING(Detail, log("sender parameters or capabililties are not compatible or has no supported codec") + filteredParams->toDebug() + unionCaps->toDebug() + mediaLine->toDebug() + sender.toDebug());
              goto reject_sender;
            }

            try {
              senderInfo->mSender->send(*filteredParams);
              insertSSRCs(*senderInfo);
            } catch (const InvalidParameters &) {
              ZS_LOG_WARNING(Debug, log("invalid parameters when calling sender.send()"));
              goto reject_sender;
            } catch (const InvalidStateError &) {
              ZS_LOG_WARNING(Debug, log("invalid state when calling sender.send()"));
              goto reject_sender;
            }
            if (senderInfo->mPromise) {
              senderInfo->mPromise->resolve();
              senderInfo->mPromise.reset();
            }
            goto accept_sender;
          }

        reject_sender:
          {
            if (!senderInfo) continue;
            close(*senderInfo);
            continue;
          }

        accept_sender:
          {
            switch (senderInfo->mNegotiationState) {
              case NegotiationState_PendingOffer:     senderInfo->mNegotiationState = NegotiationState_LocalOffered;  break;
              case NegotiationState_LocalOffered:     break;
              case NegotiationState_RemoteOffered:    senderInfo->mNegotiationState = NegotiationState_Agreed; break;
              case NegotiationState_Agreed:           break;
              case NegotiationState_Rejected:         break;
            }
            continue;
          }
        }
        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepProcessLocalSCTPTransport(ISessionDescriptionTypes::DescriptionPtr description)
      {
        for (auto iter = description->mSCTPMediaLines.begin(); iter != description->mSCTPMediaLines.end(); ++iter)
        {
          auto &mediaLine = *(*iter);

          SCTPMediaLineInfoPtr mediaLineInfo;

          // scope prepare media line
          {
            // scope: find the media line
            {
              if (mediaLine.mID.hasData()) {
                auto found = mSCTPMedias.find(mediaLine.mID);
                if (found != mSCTPMedias.end()) {
                  mediaLineInfo = (*found).second;
                }
              }

              if (!mediaLineInfo) {
                ZS_LOG_WARNING(Debug, log("media line was not found (thus reject it)") + mediaLineInfo->toDebug());
                goto reject_media_line;
              }
            }

            if (NegotiationState_Rejected == mediaLineInfo->mNegotiationState) {
              ZS_LOG_WARNING(Debug, log("media line previously rejected") + mediaLineInfo->toDebug());
              goto reject_media_line;
            }

            goto accept_media_line;
          }

        reject_media_line:
          {
            if (!mediaLineInfo) continue;
            close(*mediaLineInfo);
            continue;
          }

        accept_media_line:
          {
            switch (mediaLineInfo->mNegotiationState) {
            case NegotiationState_PendingOffer:     mediaLineInfo->mNegotiationState = NegotiationState_LocalOffered; break;
            case NegotiationState_LocalOffered:     break;
            case NegotiationState_Agreed:           break;
            case NegotiationState_RemoteOffered:    mediaLineInfo->mNegotiationState = NegotiationState_Agreed; break;
            case NegotiationState_Rejected:         break;
            }
            continue;
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepAddTracks()
      {
        typedef std::set<TransportID> TransportIDSet;

        ZS_LOG_TRACE(log("step - process add track"));

        while (mPendingAddTracks.size() > 0)
        {
          PendingAddTrackPtr pending = mPendingAddTracks.front();
          mPendingAddTracks.pop_front();

          bool alreadyPrivateTransport {false};
          TransportIDSet compatibleTransports;
          TransportIDSet disallowedTransports;

          String useMediaLineID;

          // find a compatible media line
          for (auto iter = mRTPMedias.begin(); iter != mRTPMedias.end(); ++iter)
          {
            auto &mediaLine = *((*iter).second);
            useMediaLineID = mediaLine.mID;

            // scope: check to see if media line is compatible with pending track
            {
              // track of same kind, see if it's compatible
              switch (mConfiguration.mBundlePolicy) {
                case IPeerConnectionTypes::BundlePolicy_MaxCompat:
                case IPeerConnectionTypes::BundlePolicy_MaxBundle: {
                  if (0 != mediaLine.mMediaType.compareNoCase(IMediaStreamTrackTypes::toString(pending->mTrack->kind()))) goto possible_bundle_but_not_a_match;
                  break;
                }
                
                case IPeerConnectionTypes::BundlePolicy_Balanced: {
                  if (0 != mediaLine.mMediaType.compareNoCase(IMediaStreamTrackTypes::toString(pending->mTrack->kind()))) goto not_compatible;
                  break;
                }
              }

              if (pending->mConfiguration->mCapabilities) {
                switch (mediaLine.mIDPreference) {
                  case UseAdapterHelper::IDPreference_Local: {
                    if (!UseAdapterHelper::isCompatible(*(mediaLine.mLocalSenderCapabilities), *(pending->mConfiguration->mCapabilities))) goto not_compatible;
                    break;
                  }
                  case UseAdapterHelper::IDPreference_Remote: {
                    ZS_THROW_INVALID_ASSUMPTION_IF(!mediaLine.mRemoteReceiverCapabilities);

                    // filter to the remote capabilities and ensure it's compatible
                    auto senderUnion = UseAdapterHelper::createUnion(*(pending->mConfiguration->mCapabilities), (*(mediaLine.mRemoteReceiverCapabilities)), UseAdapterHelper::IDPreference_Remote);
                    ZS_THROW_INVALID_ASSUMPTION_IF(!senderUnion);

                    if (!UseAdapterHelper::isCompatible(*(mediaLine.mRemoteReceiverCapabilities), *senderUnion)) goto not_compatible;
                    if (!UseAdapterHelper::hasSupportedMediaCodec(*senderUnion)) goto not_compatible;

                    break;
                  }
                }
              }

              if (pending->mConfiguration->mParameters) {
                switch (mediaLine.mIDPreference) {
                  case UseAdapterHelper::IDPreference_Local: {
                    if (!UseAdapterHelper::isCompatible(*(mediaLine.mLocalSenderCapabilities), *(pending->mConfiguration->mParameters))) goto not_compatible;
                    break;
                  }
                  case UseAdapterHelper::IDPreference_Remote: {
                    ZS_THROW_INVALID_ASSUMPTION_IF(!mediaLine.mRemoteReceiverCapabilities);

                    // filter to the remote capabilities and ensure it's compatible
                    auto filteredParameters = UseAdapterHelper::filterParameters(*(pending->mConfiguration->mParameters), (*(mediaLine.mRemoteReceiverCapabilities)));
                    ZS_THROW_INVALID_ASSUMPTION_IF(!filteredParameters);

                    if (!UseAdapterHelper::isCompatible(*(mediaLine.mRemoteReceiverCapabilities), *filteredParameters)) goto not_compatible;
                    if (!UseAdapterHelper::hasSupportedMediaCodec(*filteredParameters)) goto not_compatible;

                    break;
                  }
                }
              }
              switch (mConfiguration.mSignalingMode)
              {
                case IPeerConnectionTypes::SignalingMode_JSON:  break;
                case IPeerConnectionTypes::SignalingMode_SDP:   goto check_if_no_others_senders_using;
              }
              goto match;
            }

          check_if_no_others_senders_using:
            {
              for (auto iterSender = mSenders.begin(); iterSender != mSenders.end(); ++iterSender) {
                auto &sender = *((*iterSender).second);
                if (sender.mMediaLineID == mediaLine.mID) goto possible_bundle_but_not_a_match;
              }

              ZS_LOG_DEBUG(log("found no other senders using this media line (thus re-using)") + mediaLine.toDebug());
              goto match;
            }

          possible_bundle_but_not_a_match:
            {
              auto found = disallowedTransports.find(mediaLine.mBundledTransportID);
              if (found != disallowedTransports.end()) continue;
              compatibleTransports.insert(mediaLine.mBundledTransportID);
              continue;
            }

          not_compatible:
            {
              disallowedTransports.insert(mediaLine.mBundledTransportID);
              auto found = compatibleTransports.find(mediaLine.mBundledTransportID);
              if (found != compatibleTransports.end()) {
                // not allowed to bundle on this transport 
                compatibleTransports.erase(found);
              }
              continue;
            }
          }

          if (compatibleTransports.size() > 0) goto found_compatible;
          goto none_compatible;

        none_compatible:
          {
            alreadyPrivateTransport = true;

            auto transportInfo = getTransportFromPool();
            String transportID = transportInfo->mID;
            compatibleTransports.insert(transportID);

            goto found_compatible;
          }

        found_compatible:
          {
            auto first = compatibleTransports.begin();
            ZS_THROW_INVALID_ASSUMPTION_IF(first == compatibleTransports.end());

            auto bundledTransportID = (*first);
            auto mediaLine = make_shared<RTPMediaLineInfo>();
            mediaLine->mMediaType = IMediaStreamTrackTypes::toString(pending->mTrack->kind());
            mediaLine->mBundledTransportID = bundledTransportID;
            mediaLine->mLineIndex = getNextHighestMLineIndex();
            mediaLine->mIDPreference = UseAdapterHelper::IDPreference_Local;
            mediaLine->mLocalSenderCapabilities = pending->mConfiguration->mCapabilities ? pending->mConfiguration->mCapabilities : IRTPSender::getCapabilities(pending->mTrack->kind());
            mediaLine->mLocalReceiverCapabilities = pending->mConfiguration->mCapabilities ? pending->mConfiguration->mCapabilities : IRTPReceiver::getCapabilities(pending->mTrack->kind());

            if (!alreadyPrivateTransport) {
              if (IPeerConnectionTypes::BundlePolicy_MaxCompat == mConfiguration.mBundlePolicy) {
                auto privateTransport = getTransportFromPool();
                mediaLine->mPrivateTransportID = privateTransport->mID;
              }
            }

            switch (mConfiguration.mSignalingMode) {
              case IPeerConnectionTypes::SignalingMode_JSON:  {
                mediaLine->mID = registerNewID();
                break;
              }
              case IPeerConnectionTypes::SignalingMode_SDP:   {
                // with SDP media ID and transport ID must share a common ID (unless it's bundled)
                if (alreadyPrivateTransport) {
                  mediaLine->mID = registerIDUsage(bundledTransportID);
                } else if (mediaLine->mPrivateTransportID.hasData()) {
                  mediaLine->mID = registerIDUsage(mediaLine->mPrivateTransportID);
                } else {
                  mediaLine->mID = registerNewID();
                }
                break;
              }
            }

            mRTPMedias[mediaLine->mID] = mediaLine;
            useMediaLineID = mediaLine->mID;
            goto match;
          }

        match:
          {
            auto senderInfo = make_shared<SenderInfo>();
            senderInfo->mMediaLineID = useMediaLineID;
            senderInfo->mNegotiationState = NegotiationState_LocalOffered;

            RTPMediaLineInfoPtr mediaLine;

            // find media line
            {
              auto found = mRTPMedias.find(useMediaLineID);
              if (found != mRTPMedias.end()) {
                mediaLine = (*found).second;
              }
            }

            ZS_THROW_INVALID_ASSUMPTION_IF(!mediaLine);

            // With SDP, the sender and the media line must share the same ID,
            // but JSON signaling uses a unique ID for the sender.
            switch (mConfiguration.mSignalingMode) {
              case IPeerConnectionTypes::SignalingMode_JSON:  senderInfo->mID = registerNewID(); break;
              case IPeerConnectionTypes::SignalingMode_SDP:   senderInfo->mID = registerIDUsage(useMediaLineID); break;
            }

            senderInfo->mConfiguration = pending->mConfiguration;
            senderInfo->mTrack = pending->mTrack;
            senderInfo->mMediaStreams = pending->mMediaStreams;
            senderInfo->mPromise = pending->mPromise;
            pending->mPromise.reset();

            IRTPTypes::CapabilitiesPtr useCaps;

            if (mediaLine->mRemoteReceiverCapabilities) {
              useCaps = UseAdapterHelper::createUnion(*(mediaLine->mLocalSenderCapabilities), *(mediaLine->mRemoteReceiverCapabilities), mediaLine->mIDPreference);
              senderInfo->mParameters = UseAdapterHelper::capabilitiesToParameters(*useCaps);
            } else {
              useCaps = mediaLine->mLocalSenderCapabilities;
            }
            if (senderInfo->mConfiguration->mParameters) {
              senderInfo->mParameters = UseAdapterHelper::filterParameters(*senderInfo->mConfiguration->mParameters, *useCaps);
            } else {
              senderInfo->mParameters = UseAdapterHelper::capabilitiesToParameters(*(mediaLine->mLocalSenderCapabilities));
            }
            UseAdapterHelper::FillParametersOptions fillOptions(mFutureAudioSenderSSRCs, mFutureVideoSenderSSRCs);
            UseAdapterHelper::fillParameters(*(senderInfo->mParameters), *useCaps, &fillOptions);
            senderInfo->mParameters->mMuxID = senderInfo->mID;
            insertSSRCs(*senderInfo);

            mSenders[senderInfo->mID] = senderInfo;

            notifyNegotiationNeeded();  // need to signal this to the remote party before it will be started
            close(*pending);
            continue;
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepAddSCTPTransport()
      {
        if (mPendingAddDataChannels.size() < 1) {
          ZS_LOG_TRACE(log("step - add sctp transport - no pending data channels"));
          return true;
        }

        if (mConfiguration.mNegotiateSRTPSDES)
        {
          ZS_LOG_WARNING(Debug, log("step - add sctp transport - cannot add data cahnnel on SDES based transports"));
          return true;
        }

        // find a compatible media line
        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter)
        {
          auto &mediaLine = *((*iter).second);
          if (NegotiationState_Rejected == mediaLine.mNegotiationState) continue;

          ZS_LOG_TRACE(log("step - add sctp transport - already have transport available"));
          return true;
        }

        TransportInfoPtr transportInfo;

        if (IPeerConnectionTypes::BundlePolicy_MaxBundle == mConfiguration.mBundlePolicy) {
          for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter)
          {
            auto checkTransport = (*iter).second;
            if (NegotiationState_Rejected == checkTransport->mNegotiationState) continue;
          }
        }

        auto mediaLine = make_shared<SCTPMediaLineInfo>();
        mediaLine->mNegotiationState = NegotiationState_LocalOffered;

        if (!transportInfo) {
          transportInfo = getTransportFromPool();
          mediaLine->mPrivateTransportID = transportInfo->mID;
        }

        mediaLine->mBundledTransportID = transportInfo->mID;
        mediaLine->mLineIndex = getNextHighestMLineIndex();

        switch (mConfiguration.mSignalingMode) {
          case IPeerConnectionTypes::SignalingMode_JSON: {
            mediaLine->mID = registerNewID();
            break;
          }
          case IPeerConnectionTypes::SignalingMode_SDP: {
            // with SDP media ID and transport ID must share a common ID (unless it's bundled)
            if (mediaLine->mPrivateTransportID.hasData()) {
              mediaLine->mID = registerIDUsage(mediaLine->mPrivateTransportID);
            } else {
              mediaLine->mID = registerNewID();
            }
            break;
          }
        }

        mediaLine->mLocalPort = registerNewLocalPort();
        if (0 == mediaLine->mLocalPort.value()) {
          close(*mediaLine);
        }

        mSCTPMedias[mediaLine->mID] = mediaLine;
        notifyNegotiationNeeded();  // need to signal this to the remote party before it will be started
        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepCreateOfferOrAnswer()
      {
        ZS_LOG_TRACE(log("step - create offer"));

        if (mPendingMethods.size() < 1) {
          ZS_LOG_TRACE(log("skipping step to creeate offer (no pending methods)"));
          return true;
        }

        PendingMethodPtr pending = mPendingMethods.front();

        ISessionDescriptionTypes::SignalingTypes type {ISessionDescriptionTypes::SignalingType_First};

        bool isSDP = true;
        ULONGLONG version = 0;

        switch (pending->mMethod) {
          case PendingMethod_CreateOffer:         type = ISessionDescriptionTypes::SignalingType_SDPOffer; version = 1; break;
          case PendingMethod_CreateAnswer:        type = ISessionDescriptionTypes::SignalingType_SDPAnswer; break;
          case PendingMethod_CreateCapabilities:  type = ISessionDescriptionTypes::SignalingType_JSON; isSDP = false; break;
          case PendingMethod_SetLocalDescription:
          case PendingMethod_SetRemoteDescription:  
          {
            ZS_LOG_TRACE(log("pending is not related to offer/answer creation"));
            return true;
          }
        }

        mPendingMethods.pop_front();

        auto description = make_shared<ISessionDescription::Description>();
        auto remoteDescription = mRemoteDescription ? mRemoteDescription->description() : ISessionDescription::DescriptionPtr();
        description->mDetails = make_shared<ISessionDescription::Description::Details>();
        if ((remoteDescription) && (remoteDescription->mDetails)) {
          description->mDetails->mSessionID = remoteDescription->mDetails->mSessionID;
          description->mDetails->mSessionVersion = remoteDescription->mDetails->mSessionVersion + version;
          description->mDetails->mSessionName = remoteDescription->mDetails->mSessionName;
          description->mDetails->mStartTime = remoteDescription->mDetails->mStartTime;
          description->mDetails->mEndTime = remoteDescription->mDetails->mEndTime;
        } else {
          auto buffer = UseServicesHelper::random(sizeof(description->mDetails->mSessionID));
          memcpy(&(description->mDetails->mSessionID), buffer->BytePtr(), buffer->SizeInBytes());
          description->mDetails->mSessionVersion = 1;
          description->mDetails->mSessionName = "-";
        }
        if (isSDP) {
          description->mDetails->mUnicaseAddress = make_shared<ISessionDescription::ConnectionData::Details>();
          description->mDetails->mUnicaseAddress->mNetType = "IN";
          description->mDetails->mUnicaseAddress->mAddrType = "IP4";
          description->mDetails->mUnicaseAddress->mConnectionAddress = "127.0.0.1";
        }

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &transportInfo = ((*iter).second);
          if (NegotiationState_Rejected == transportInfo->mNegotiationState) continue;

          auto transport = make_shared<ISessionDescriptionTypes::Transport>();
          transport->mRTP = make_shared<ISessionDescriptionTypes::Transport::Parameters>();
          if (transportInfo->mRTCP.mGatherer) {
            transport->mRTCP = make_shared<ISessionDescriptionTypes::Transport::Parameters>();
          }

          transport->mID = transportInfo->mID;
          transport->mRTP->mICEParameters = transportInfo->mRTP.mGatherer->getLocalParameters();
          transport->mRTP->mDTLSParameters = getDTLSParameters(*transportInfo, IICETypes::Component_RTP);
          if (mConfiguration.mNegotiateSRTPSDES) {
            transport->mRTP->mSRTPSDESParameters = transportInfo->mRTP.mSRTPSDESParameters ? transportInfo->mRTP.mSRTPSDESParameters : ISessionDescriptionTypes::SRTPSDESParametersPtr();
          }

          // always get end of candidates state before candidates
          transport->mRTP->mEndOfCandidates = (IICEGathererTypes::State_Complete == transportInfo->mRTP.mGatherer->state());
          transport->mRTP->mICECandidates = *convertCandidateList(*(transportInfo->mRTP.mGatherer->getLocalCandidates()));

          if ((transportInfo->mRTCP.mGatherer) &&
              (transportInfo->mRTCP.mTransport)) {
            transport->mRTCP = make_shared<ISessionDescription::Transport::Parameters>();

            // always get end of candidates state before candidates
            transport->mRTP->mEndOfCandidates = (IICEGathererTypes::State_Complete == transportInfo->mRTCP.mGatherer->state());
            transport->mRTP->mICECandidates = *convertCandidateList(*(transportInfo->mRTCP.mGatherer->getLocalCandidates()));
          }
          transport->mUseMux = true;

          description->mTransports.push_back(transport);
        }

        for (auto iter = mRTPMedias.begin(); iter != mRTPMedias.end(); ++iter) {
          auto &mediaInfo = ((*iter).second);
          if (NegotiationState_Rejected == mediaInfo->mNegotiationState) continue;

          auto mediaLine = make_shared<ISessionDescriptionTypes::RTPMediaLine>();
          mediaLine->mID = mediaInfo->mID;
          mediaLine->mTransportID = mediaInfo->mBundledTransportID;
          mediaLine->mMediaType = mediaInfo->mMediaType;
          if ((isSDP) ||
              (mediaInfo->mPrivateTransportID.hasData())) {
            mediaLine->mDetails = make_shared<ISessionDescriptionTypes::RTPMediaLine::Details>();
            mediaLine->mDetails->mPrivateTransportID = mediaInfo->mPrivateTransportID;
            if (isSDP) {
              mediaLine->mDetails->mInternalIndex = mediaInfo->mLineIndex;
              mediaLine->mDetails->mProtocol = "UDP/TLS/RTP/SAVPF";
              mediaLine->mDetails->mConnectionData = make_shared<ISessionDescriptionTypes::ConnectionData>();
              mediaLine->mDetails->mConnectionData->mRTP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
              mediaLine->mDetails->mConnectionData->mRTCP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
              mediaLine->mDetails->mConnectionData->mRTP->mNetType = mediaLine->mDetails->mConnectionData->mRTCP->mNetType = "IN";
              mediaLine->mDetails->mConnectionData->mRTP->mAddrType = mediaLine->mDetails->mConnectionData->mRTCP->mAddrType = "IP4";
              mediaLine->mDetails->mConnectionData->mRTP->mConnectionAddress = mediaLine->mDetails->mConnectionData->mRTCP->mConnectionAddress = "0.0.0.0";
#define TODO_FILL_WITH_CANDIDATE_INFO_LATER 1
#define TODO_FILL_WITH_CANDIDATE_INFO_LATER 2
            }
          }
          mediaLine->mSenderCapabilities = make_shared<ISessionDescriptionTypes::RTPCapabilities>(*mediaInfo->mLocalSenderCapabilities);
          mediaLine->mReceiverCapabilities = make_shared<ISessionDescriptionTypes::RTPCapabilities>(*mediaInfo->mLocalReceiverCapabilities);

          description->mRTPMediaLines.push_back(mediaLine);
        }

        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter) {
          auto &mediaInfo = ((*iter).second);
          if (NegotiationState_Rejected == mediaInfo->mNegotiationState) continue;

          auto mediaLine = make_shared<ISessionDescriptionTypes::SCTPMediaLine>();
          mediaLine->mID = mediaInfo->mID;
          mediaLine->mTransportID = mediaInfo->mBundledTransportID;
          mediaLine->mMediaType = "application";
          if ((isSDP) ||
            (mediaInfo->mPrivateTransportID.hasData())) {
            mediaLine->mDetails = make_shared<ISessionDescriptionTypes::RTPMediaLine::Details>();
            mediaLine->mDetails->mPrivateTransportID = mediaInfo->mPrivateTransportID;
            if (isSDP) {
              mediaLine->mDetails->mInternalIndex = mediaInfo->mLineIndex;
              mediaLine->mDetails->mProtocol = "UDP/DTLS/SCTP";
              mediaLine->mDetails->mConnectionData = make_shared<ISessionDescriptionTypes::ConnectionData>();
              mediaLine->mDetails->mConnectionData->mRTP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
              mediaLine->mDetails->mConnectionData->mRTCP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
              mediaLine->mDetails->mConnectionData->mRTP->mNetType = mediaLine->mDetails->mConnectionData->mRTCP->mNetType = "IN";
              mediaLine->mDetails->mConnectionData->mRTP->mAddrType = mediaLine->mDetails->mConnectionData->mRTCP->mAddrType = "IP4";
              mediaLine->mDetails->mConnectionData->mRTP->mConnectionAddress = mediaLine->mDetails->mConnectionData->mRTCP->mConnectionAddress = "0.0.0.0";
#define TODO_FILL_WITH_CANDIDATE_INFO_LATER 1
#define TODO_FILL_WITH_CANDIDATE_INFO_LATER 2
            }
          }
          mediaLine->mCapabilities = ISCTPTransport::getCapabilities();
          mediaLine->mPort = mediaInfo->mLocalPort.value();

          description->mSCTPMediaLines.push_back(mediaLine);
        }

        for (auto iter = mSenders.begin(); iter != mSenders.end(); ++iter) {
          auto &senderInfo = ((*iter).second);
          if (NegotiationState_Rejected == senderInfo->mNegotiationState) continue;

          auto sender = make_shared<ISessionDescriptionTypes::RTPSender>();
          sender->mID = senderInfo->mID;
          if (isSDP) {
            sender->mDetails = make_shared<ISessionDescriptionTypes::RTPSender::Details>();
            //sender->mDetails->mInternalRTPMediaLineIndex
          }
          sender->mRTPMediaLineID = senderInfo->mMediaLineID;
          sender->mParameters = make_shared<IRTPTypes::Parameters>(*senderInfo->mParameters);
          sender->mMediaStreamTrackID = senderInfo->mTrack->id();
          
          for (auto iterStream = senderInfo->mMediaStreams.begin(); iterStream != senderInfo->mMediaStreams.end(); ++iterStream) {
            auto &stream = (*iterStream).second;
            sender->mMediaStreamIDs.insert(stream->id());
          }
          description->mRTPSenders.push_back(sender);
        }

        auto sessionDescription = ISessionDescription::create(type, *description);

        pending->mPromise->resolve(sessionDescription);
        pending->mPromise.reset();

        close(*pending);

        onWake();
        return false;
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
            if (!mediaLine.mLineIndex.hasValue()) continue;
            if (mediaLine.mLineIndex != candidate->mMLineIndex.value()) continue;

            String transportID(mediaLine.mPrivateTransportID.hasData() ? mediaLine.mPrivateTransportID : mediaLine.mBundledTransportID);

            auto found = mTransports.find(transportID);
            if (found == mTransports.end()) goto not_found;
            addCandidateToTransport(*((*found).second), candidate);
            goto found;
          }

          for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter) {
            auto &mediaLine = *((*iter).second);
            if (!mediaLine.mLineIndex.hasValue()) continue;
            if (mediaLine.mLineIndex != candidate->mMLineIndex.value()) continue;

            String transportID(mediaLine.mPrivateTransportID.hasData() ? mediaLine.mPrivateTransportID : mediaLine.mBundledTransportID);

            auto found = mTransports.find(transportID);
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
      bool PeerConnection::stepFinalizeSenders()
      {
        ZS_LOG_TRACE("step - finalize senders");

        for (auto iter_doNotUse = mSenders.begin(); iter_doNotUse != mSenders.end(); ) {

          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &senderInfo = ((*current).second);
          if (!senderInfo->mPromise) continue;

          WORD error = UseHTTP::HTTPStatusCode_Conflict;
          const char *reason = NULL;

          // scope: see if the sender can be resolved
          {
            auto foundMediaLine = mRTPMedias.find(senderInfo->mMediaLineID);
            if (foundMediaLine == mRTPMedias.end()) {
              reason = "media line associated with sender is not present (thus removing sender)";
              goto remove_sender;
            }
            
            auto &mediaLine = (*foundMediaLine).second;

            auto foundTransport = mTransports.find(mediaLine->mBundledTransportID);
            if (foundTransport == mTransports.end()) {
              reason = "transport associated with media line is not present (thus removing sender)";
              goto remove_sender;
            }

            auto &transport = (*foundTransport).second;
            if (NegotiationState_Agreed != transport->mNegotiationState) {
              ZS_LOG_TRACE(log("still waiting for negotiation of transport to complete") + transport->toDebug());
              continue;
            }

            ZS_THROW_INVALID_ASSUMPTION_IF(!mediaLine->mLocalSenderCapabilities);
            ZS_THROW_INVALID_ASSUMPTION_IF(!mediaLine->mRemoteReceiverCapabilities);

            auto capsUnion = UseAdapterHelper::createUnion(*mediaLine->mLocalSenderCapabilities, *mediaLine->mRemoteReceiverCapabilities, mediaLine->mIDPreference);
            if (!senderInfo->mParameters) {
              senderInfo->mParameters = senderInfo->mConfiguration->mParameters ? senderInfo->mConfiguration->mParameters : UseAdapterHelper::capabilitiesToParameters(*capsUnion);
            } else {
              senderInfo->mParameters = UseAdapterHelper::filterParameters(*(senderInfo->mParameters), *capsUnion);
            }

            if (!UseAdapterHelper::isCompatible(*capsUnion, *(senderInfo->mParameters))) {
              reason = "sender is not compatible with remote party (thus removing)";
              ZS_LOG_WARNING(Debug, log(reason) + capsUnion->toDebug() + senderInfo->mParameters->toDebug());
              goto remove_sender;
            }

            if (!UseAdapterHelper::hasSupportedMediaCodec(*(senderInfo->mParameters))) {
              reason = "sender is not compatible with remote party (thus removing)";
              ZS_LOG_WARNING(Debug, log(reason) + capsUnion->toDebug() + senderInfo->mParameters->toDebug());
              goto remove_sender;
            }

            UseAdapterHelper::FillParametersOptions fillOptions(mFutureAudioSenderSSRCs, mFutureVideoSenderSSRCs);
            UseAdapterHelper::fillParameters(*(senderInfo->mParameters), *capsUnion, &fillOptions);

            if (transport->mRTP.mDTLSTransport) {
              senderInfo->mSender = IRTPSender::create(mThisWeak.lock(), senderInfo->mTrack, transport->mRTP.mDTLSTransport, transport->mRTCP.mDTLSTransport);
            } else {
              senderInfo->mSender = IRTPSender::create(mThisWeak.lock(), senderInfo->mTrack, transport->mRTP.mSRTPSDESTransport, transport->mRTCP.mTransport);
            }

            if (!senderInfo->mSender) {
              reason = "sender was not created";
              error = UseHTTP::HTTPStatusCode_NoContent;
              goto remove_sender;
            }

            ZS_LOG_DEBUG(log("sender created") + ZS_PARAM("sender id", senderInfo->mSender->getID()) + senderInfo->mParameters->toDebug());

            try {
              senderInfo->mSender->send(*(senderInfo->mParameters));
              insertSSRCs(*senderInfo);
            } catch (const InvalidParameters &) {
              reason = "sender.send() caused InvalidParameters exception";
              goto remove_sender;
            } catch (const InvalidStateError &) {
              reason = "sender.send() caused InvalidState exception";
              goto remove_sender;
            }
            senderInfo->mPromise->resolve(senderInfo->mSender);
            senderInfo->mPromise.reset();
            goto next;
          }

        remove_sender:
          {
            ZS_LOG_WARNING(Detail, log(reason) + senderInfo->toDebug());
            senderInfo->mPromise->reject(ErrorAny::create(error, reason));
            senderInfo->mPromise.reset();
            close(*senderInfo);
            mSenders.erase(current);
            // ensure unlinked transports / media lines get removed
            wake();
          }

        next:
          {
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepFinalizeDataChannels()
      {
        SCTPMediaLineInfoPtr mediaLine;
        bool rejectAll = false;

        WORD error = UseHTTP::HTTPStatusCode_Conflict;
        const char *reason = NULL;

        while (mPendingAddDataChannels.size() > 0)
        {
          PendingAddDataChannelPtr pending = mPendingAddDataChannels.front();

          if ((mConfiguration.mNegotiateSRTPSDES) ||
              (rejectAll)) {
            if (!reason) {
              reason = "unable to add a datachannel unless the transport is DTLS not SRTP/SDES";
            }
            if (pending->mPromise) {
              pending->mPromise->reject(ErrorAny::create(error, reason));
              mPendingAddDataChannels.pop_front();
              continue;
            }
          }

          if (!mediaLine) {
            size_t totalRejected = 0;
            for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter)
            {
              auto checkMediaLine = (*iter).second;
              if (NegotiationState_Rejected == checkMediaLine->mNegotiationState) {
                ++totalRejected;
                continue;
              }

              if (!checkMediaLine->mSCTPTransport) continue;
              mediaLine = checkMediaLine;
              break;
            }

            if (totalRejected == mSCTPMedias.size()) {
              reason = "all sctp transport media lines have been rejected";
              rejectAll = true;
              continue;
            }

            if (!mediaLine) {
              ZS_LOG_TRACE("step - finalize datachannels - unable to create datachannel as no SCTP transport is available");
              return true;
            }
          }

          if ((!mediaLine->mSCTPTransport) ||
              (!mediaLine->mRemotePort.hasValue())) {
            ZS_LOG_TRACE("step - finalize datachannels - unable to create datachannel as SCTP transport is not ready");
            return true;
          }

          mPendingAddDataChannels.pop_front();

          if (!pending->mParameters) {
            pending->mParameters = make_shared<IDataChannelTypes::Parameters>();
          }

          auto dataChannel = IDataChannel::create(mThisWeak.lock(), mediaLine->mSCTPTransport, *pending->mParameters);

          if (pending->mPromise) {
            if (!dataChannel) {
              pending->mPromise->reject(ErrorAny::create(error, "unable to create a new datachannel on the sctp transport"));
            } else {
              pending->mPromise->resolve(dataChannel);
            }
            pending->mPromise.reset();
          }
        }
        return true;
      }

      //-----------------------------------------------------------------------
      static void figureOutState(
                                 Optional<IICEGathererTypes::States> &ioState,
                                 IICEGathererPtr gatherer
                                 )
      {
        if (!gatherer) return;

        auto state = gatherer->state();
        if (!ioState.hasValue()) {
          ioState = state;
          return;
        }

        switch (state) {
          case IICEGathererTypes::State_New:        break;
          case IICEGathererTypes::State_Gathering:  ioState = state; break;
          case IICEGathererTypes::State_Complete: 
          case IICEGathererTypes::State_Closed:     {
            switch (ioState.value()) {
              case IICEGathererTypes::State_New:        return;
              case IICEGathererTypes::State_Gathering:  return;
              case IICEGathererTypes::State_Complete:   
              case IICEGathererTypes::State_Closed:     {
                ioState = IICEGathererTypes::State_Complete;
                break;
              }
            }
          }
        }
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepFixGathererState()
      {
        ZS_LOG_TRACE("step - fix gatherer state");

        Optional<IICEGathererTypes::States> state;

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &transportInfo = (*iter).second;

          figureOutState(state, transportInfo->mRTP.mGatherer);
          figureOutState(state, transportInfo->mRTCP.mGatherer);
        }

        if (!state.hasValue()) {
          ZS_LOG_TRACE(log("state has not changed"));
          return true;
        }

        ZS_LOG_TRACE(log("ice gatherer state is now") + ZS_PARAM("state", IICEGathererTypes::toString(state.value())));
        setState(state.value());

        return true;
      }

      //-----------------------------------------------------------------------
      static void figureOutState(
                                 size_t *stateArray,
                                 IICETransportPtr transport
                                 )
      {
        if (!transport) return;

        auto state = transport->state();
        ++(stateArray[state]);
      }

      //-----------------------------------------------------------------------
      static void figureOutState(
                                 size_t *stateArray,
                                 IDTLSTransportPtr transport
                                 )
      {
        if (!transport) return;

        auto state = transport->state();
        ++(stateArray[state]);
      }

      //-----------------------------------------------------------------------
      static void figureOutState(
                                 size_t *stateArray,
                                 ISRTPSDESTransportPtr transport
                                 )
      {
        if (!transport) return;

        auto state = IDTLSTransportTypes::State_Connected;
        ++(stateArray[state]);
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepFixTransportState()
      {
        ZS_LOG_TRACE("step - fix gatherer state");

        size_t iceTransportStateCount[IICETransportTypes::State_Last + 1] = {};
        size_t dtlsTranportStateCount[IDTLSTransportTypes::State_Last + 1] = {};

        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto &transportInfo = (*iter).second;

          figureOutState(iceTransportStateCount, transportInfo->mRTP.mTransport);
          figureOutState(iceTransportStateCount, transportInfo->mRTCP.mTransport);
          figureOutState(dtlsTranportStateCount, transportInfo->mRTP.mDTLSTransport);
          figureOutState(dtlsTranportStateCount, transportInfo->mRTCP.mDTLSTransport);
          figureOutState(dtlsTranportStateCount, transportInfo->mRTP.mSRTPSDESTransport);
        }

        // scope: figure out ice transport state
        {
          Optional<IICETransportTypes::States> iceTransportState;

          if ((iceTransportStateCount[IICETransportTypes::State_New] > 0) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected])) {
            iceTransportState = IICETransportTypes::State_New;
          }

          if ((iceTransportStateCount[IICETransportTypes::State_Checking] > 0) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected])) {
            iceTransportState = IICETransportTypes::State_Checking;
          }

          if ((0 == iceTransportStateCount[IICETransportTypes::State_New] > 0) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (iceTransportStateCount[IICETransportTypes::State_Connected] > 0) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed])) {
            iceTransportState = IICETransportTypes::State_Connected;
          }

          if ((0 == iceTransportStateCount[IICETransportTypes::State_New]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Connected]) &&
              (iceTransportStateCount[IICETransportTypes::State_Completed] > 0) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed])) {
            iceTransportState = IICETransportTypes::State_Completed;
          }

          if (iceTransportStateCount[IICETransportTypes::State_Failed] > 0) {
            iceTransportState = IICETransportTypes::State_Failed;
          }

          if ((iceTransportStateCount[IICETransportTypes::State_Disconnected] > 0) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed])) {
            iceTransportState = IICETransportTypes::State_Disconnected;
          }

          if ((0 == iceTransportStateCount[IICETransportTypes::State_New]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Connected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Completed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (iceTransportStateCount[IICETransportTypes::State_Closed] > 0)) {
            iceTransportState = IICETransportTypes::State_Closed;
          }

          if (iceTransportState.hasValue()) {
            ZS_LOG_TRACE(log("ice connection state is now") + ZS_PARAM("state", IICETransportTypes::toString(iceTransportState.value())));
            setState(iceTransportState.value());
          }
        }

        // scope: figure out peer connection state
        {
          Optional<IPeerConnectionTypes::PeerConnectionStates> peerConnectionState;

          if (((iceTransportStateCount[IICETransportTypes::State_New] > 0) ||
               (dtlsTranportStateCount[IDTLSTransportTypes::State_New] > 0)) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Connecting]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Connected]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Connected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Completed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Failed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected])) {
            peerConnectionState = IPeerConnectionTypes::PeerConnectionState_New;
          }

          if ((0 == iceTransportStateCount[IICETransportTypes::State_New]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_New]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Connecting]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Connected]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Connected]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Completed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Failed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected]) &&
              ((0 > iceTransportStateCount[IICETransportTypes::State_Closed]) ||
               (0 > dtlsTranportStateCount[IDTLSTransportTypes::State_Closed]))) {
            peerConnectionState = IPeerConnectionTypes::PeerConnectionState_New;
          }

          if (((iceTransportStateCount[IICETransportTypes::State_Checking] > 0) ||
               (dtlsTranportStateCount[IDTLSTransportTypes::State_Connecting] > 0)) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Failed])) {
            peerConnectionState = IPeerConnectionTypes::PeerConnectionState_Connecting;
          }

          if ((0 == iceTransportStateCount[IICETransportTypes::State_New]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_New]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Connecting]) &&
              ((iceTransportStateCount[IICETransportTypes::State_Connected] > 0) ||
               (dtlsTranportStateCount[IDTLSTransportTypes::State_Connected] > 0) ||
               (iceTransportStateCount[IICETransportTypes::State_Completed] > 0)) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Failed]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Disconnected])) {
            peerConnectionState = IPeerConnectionTypes::PeerConnectionState_Connected;
          }

          if ((0 == iceTransportStateCount[IICETransportTypes::State_Checking]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Connecting]) &&
              (0 == iceTransportStateCount[IICETransportTypes::State_Failed]) &&
              (0 == dtlsTranportStateCount[IDTLSTransportTypes::State_Failed]) &&
              (iceTransportStateCount[IICETransportTypes::State_Disconnected] > 0)) {
            peerConnectionState = IPeerConnectionTypes::PeerConnectionState_Disconnected;
          }

          if ((iceTransportStateCount[IICETransportTypes::State_Failed] > 0) ||
              (dtlsTranportStateCount[IDTLSTransportTypes::State_Failed] > 0)) {
            peerConnectionState = IPeerConnectionTypes::PeerConnectionState_Failed;
          }

          if (peerConnectionState.hasValue()) {
            ZS_LOG_TRACE(log("peer connection state is now") + ZS_PARAM("state", IPeerConnectionTypes::toString(peerConnectionState.value())));
            setState(peerConnectionState.value());
          }
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

          if (IICETypes::Component_RTP == candidate->mCandidate->mComponent) {
            if (!transport.mRTP.mTransport) goto not_found;
            transport.mRTP.mTransport->addRemoteCandidate(*(candidate->mCandidate));
            return;
          }
          if (IICETypes::Component_RTCP == candidate->mCandidate->mComponent) {
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
      PeerConnection::TransportInfoPtr PeerConnection::getTransportFromPool(const char *useID)
      {
        String useIDStr(useID);

        if (mTransportPool.size() < 1) {
          addToTransportPool();
        }

        TransportInfoPtr transport = mTransportPool.front();
        mTransportPool.pop_front();

        transport->mID = useIDStr.hasData() ? registerIDUsage(useID) : registerNewID();
        transport->mCertificates = mConfiguration.mCertificates;

        mTransports[transport->mID] = transport;
        return transport;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::addToTransportPool()
      {
        TransportInfoPtr info(make_shared<TransportInfo>());

        IICEGatherer::Options emptyOptions;
        info->mRTP.mGatherer = IICEGatherer::create(mThisWeak.lock(), mConfiguration.mGatherOptions ? (*(mConfiguration.mGatherOptions)) : emptyOptions);
        info->mRTP.mGatherer->gather();
        info->mRTP.mTransport = IICETransport::create(mThisWeak.lock(), info->mRTP.mGatherer);

        if (IPeerConnectionTypes::RTCPMuxPolicy_Negotiated == mConfiguration.mRTCPMuxPolicy) {
          info->mRTCP.mGatherer = info->mRTP.mGatherer->createAssociatedGatherer(mThisWeak.lock());
          info->mRTCP.mTransport = info->mRTCP.mTransport->createAssociatedTransport(mThisWeak.lock());
        }

        if (mConfiguration.mNegotiateSRTPSDES) {
          info->mRTP.mSRTPSDESParameters = ISRTPSDESTransport::getLocalParameters();
        }

        mTransportPool.push_back(info);
      }

      //-----------------------------------------------------------------------
      String PeerConnection::registerNewID(size_t length)
      {
        size_t tries = 0;

        String result;
        
        while (true)
        {
          ++tries;
          if (tries % 10 == 0) ++length;

          result = UseServicesHelper::randomString(length);
          result.toLower();

          auto found = mExistingIDs.find(result);
          if (found != mExistingIDs.end()) continue;

          break;
        }

        mExistingIDs[result] = 1;
        return result;
      }

      //-----------------------------------------------------------------------
      String PeerConnection::registerIDUsage(const char *idStr)
      {
        String str(idStr);

        auto found = mExistingIDs.find(str);
        if (found == mExistingIDs.end()) {
          mExistingIDs[str] = 1;
          return str;
        }

        auto &total = (*found).second;
        ++total;
        return str;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::unregisterID(const char *idStr)
      {
        String str(idStr);

        auto found = mExistingIDs.find(str);
        if (found == mExistingIDs.end()) return;

        auto &total = (*found).second;
        if (total > 1) {
          --total;
          return;
        }

        mExistingIDs.erase(found);
      }

      //-----------------------------------------------------------------------
      WORD PeerConnection::registerNewLocalPort()
      {
        auto capabilities = ISCTPTransport::getCapabilities();
        WORD startingPort = capabilities->mMinPort;
        if (0 == startingPort) {
          startingPort = capabilities->mMaxPort;
          if (0 == startingPort) {
            startingPort = 5000;
          }
        }

        do
        {
          auto found = mExistingLocalPorts.find(startingPort);
          if (found != mExistingLocalPorts.end()) {
            if (startingPort == capabilities->mMaxPort) return 0;
            ++startingPort;
            continue;
          }
          break;
        } while (true);

        mExistingLocalPorts.insert(startingPort);
        return startingPort;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::flushLocalPending(ISessionDescriptionPtr description)
      {
        if (!mPendingLocalDescription) return;
        if (!description) return;

        if (mPendingLocalDescription->getID() != description->getID()) return;
        mPendingLocalDescription.reset();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::flushRemotePending(ISessionDescriptionPtr description)
      {
        if (!mPendingRemoteDescription) return;
        if (!description) return;

        if (mPendingRemoteDescription->getID() != description->getID()) return;
        mPendingRemoteDescription.reset();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(TransportInfo &transportInfo)
      {
        close(transportInfo.mRTCP);
        close(transportInfo.mRTP);
        transportInfo.mNegotiationState = NegotiationState_Rejected;
        unregisterID(transportInfo.mID);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(TransportInfo::Details &details)
      {
        if (details.mSRTPSDESTransport) {
          details.mSRTPSDESTransport->stop();
          details.mSRTPSDESTransport.reset();
        }
        if (details.mDTLSTransport) {
          details.mDTLSTransport->stop();
          details.mDTLSTransport.reset();
        }
        if (details.mTransport) {
          details.mTransport->stop();
          details.mTransport.reset();
        }
        if (details.mGatherer) {
          details.mGatherer->close();
          details.mGatherer.reset();
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(RTPMediaLineInfo &mediaLineInfo)
      {
        mediaLineInfo.mNegotiationState = NegotiationState_Rejected;
        unregisterID(mediaLineInfo.mID);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(SCTPMediaLineInfo &mediaLineInfo)
      {
        for (auto iter_doNotUse = mediaLineInfo.mDataChannels.begin(); iter_doNotUse != mediaLineInfo.mDataChannels.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto dataChannel = (*current).second;

          if (dataChannel->mSubscription) {
            dataChannel->mSubscription->cancel();
            dataChannel->mSubscription.reset();
          }
          if (dataChannel->mDataChannel) {
            dataChannel->mDataChannel->close();
            dataChannel->mDataChannel.reset();
          }
          mediaLineInfo.mDataChannels.erase(current);
        }

        if (mediaLineInfo.mSCTPTransport) {
          mediaLineInfo.mSCTPTransport->stop();
          mediaLineInfo.mSCTPTransport.reset();
        }
        mediaLineInfo.mNegotiationState = NegotiationState_Rejected;
        unregisterID(mediaLineInfo.mID);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(SenderInfo &senderInfo)
      {
        clearSSRCs(senderInfo);

        if (senderInfo.mParameters) {
          if (senderInfo.mParameters->mEncodings.size() > 0) {
            for (auto iter = senderInfo.mParameters->mEncodings.begin(); iter != senderInfo.mParameters->mEncodings.end(); ++iter)
            {
              auto &encoding = (*iter);

              if (encoding.mSSRC.hasValue()) {
                clearSSRC(encoding.mSSRC.value());
              }
            }
          }
        }

        if (senderInfo.mPromise) {
          senderInfo.mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "connection is closing"));
          senderInfo.mPromise.reset();
        }
        if (senderInfo.mSender) {
          senderInfo.mSender->stop();
          senderInfo.mSender.reset();
        }
        senderInfo.mNegotiationState = NegotiationState_Rejected;
        unregisterID(senderInfo.mID);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(ReceiverInfo &receiverInfo)
      {
        if (receiverInfo.mReceiver) {
          for (auto iter = receiverInfo.mMediaStreams.begin(); iter != receiverInfo.mMediaStreams.end(); ++iter) {
            auto &useStream = (*iter).second;
            useStream->notifyRemoveTrack(receiverInfo.mReceiver->track());
          }

          auto evt = make_shared<MediaStreamTrackEvent>();
          evt->mReceiver = receiverInfo.mReceiver;
          evt->mTrack = receiverInfo.mReceiver->track();

          mSubscriptions.delegate()->onPeerConnectionTrackGone(mThisWeak.lock(), evt);

          receiverInfo.mReceiver->stop();
          receiverInfo.mReceiver.reset();
        }
        receiverInfo.mNegotiationState = NegotiationState_Rejected;
        purgeNonReferencedAndEmptyStreams();
        unregisterID(receiverInfo.mID);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(PendingMethod &pending)
      {
        if (pending.mPromise) {
          pending.mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "connection is closing"));
          pending.mPromise.reset();
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(PendingAddTrack &pending)
      {
        if (pending.mPromise) {
          pending.mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "connection is closing"));
          pending.mPromise.reset();
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(PendingAddDataChannel &pending)
      {
        if (pending.mPromise) {
          pending.mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "connection is closing"));
          pending.mPromise.reset();
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::insertSSRCs(SenderInfo &senderInfo)
      {
        if (!senderInfo.mParameters) return;

        auto kind = UseRTPTypesHelper::getCodecsKind(*senderInfo.mParameters);
        if (!kind.hasValue()) return;

        for (auto iter = senderInfo.mParameters->mEncodings.begin(); iter != senderInfo.mParameters->mEncodings.end(); ++iter)
        {
          auto &encoding = (*iter);

          if (!encoding.mSSRC.hasValue()) continue;

          switch (kind.value())
          {
            case ortc::IMediaStreamTrackTypes::Kind_Audio:  mAudioSenderSSRCs.insert(encoding.mSSRC.value()); break;
            case ortc::IMediaStreamTrackTypes::Kind_Video:  mVideoSenderSSRCs.insert(encoding.mSSRC.value()); break;
          }
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::clearSSRCs(SenderInfo &senderInfo)
      {
        if (!senderInfo.mParameters) return;

        for (auto iter = senderInfo.mParameters->mEncodings.begin(); iter != senderInfo.mParameters->mEncodings.end(); ++iter)
        {
          auto &encoding = (*iter);

          if (encoding.mSSRC.hasValue()) {
            clearSSRC(encoding.mSSRC.value());
          }
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::clearSSRC(SSRCType ssrc)
      {
        {
          auto found = mAudioSenderSSRCs.find(ssrc);
          if (found != mAudioSenderSSRCs.end()) {
            mAudioSenderSSRCs.erase(found);
          }
        }
        {
          auto found = mVideoSenderSSRCs.find(ssrc);
          if (found != mVideoSenderSSRCs.end()) {
            mVideoSenderSSRCs.erase(found);
          }
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::fillRTCPSSRC(IRTPTypes::Parameters &receiverParameters)
      {
        auto kind = UseRTPTypesHelper::getCodecsKind(receiverParameters);
        if (!kind.hasValue()) return;

        {
          SSRCSet::iterator found {};

          switch (kind.value())
          {
            case IMediaStreamTrackTypes::Kind_Audio: {
              if (mAudioSenderSSRCs.size() < 1) goto not_found;
              found = mAudioSenderSSRCs.begin(); break;
            }
            case IMediaStreamTrackTypes::Kind_Video: {
              if (mVideoSenderSSRCs.size() < 1) goto not_found;
              found = mVideoSenderSSRCs.begin(); break;
            }
          }

          receiverParameters.mRTCP.mSSRC = (*found);
          return;
        }

      not_found:
        {
          receiverParameters.mRTCP.mSSRC = UseAdapterHelper::peekNextSSRC(kind.value(), mFutureAudioSenderSSRCs, mFutureVideoSenderSSRCs);
        }
      }

      //-----------------------------------------------------------------------
      void PeerConnection::purgeNonReferencedAndEmptyStreams()
      {
        for (auto iter_doNotUse = mMediaStreams.begin(); iter_doNotUse != mMediaStreams.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &stream = (*current).second;
          if (stream->size() > 0) continue;

          if (!stream.unique()) continue;

          mMediaStreams.erase(current);
        }
      }

      //-----------------------------------------------------------------------
      Optional<size_t> PeerConnection::getNextHighestMLineIndex() const
      {
        Optional<size_t> result;

        if (IPeerConnectionTypes::SignalingMode_JSON == mConfiguration.mSignalingMode) return result;

        for (auto iter = mRTPMedias.begin(); iter != mRTPMedias.end(); ++iter) {
          auto &mediaLine = (*iter).second;

          if (!result.hasValue()) {
            result = mediaLine->mLineIndex;
            continue;
          }

          if (!mediaLine->mLineIndex.hasValue()) continue;

          if (mediaLine->mLineIndex.value() <= result.value()) continue;

          result = mediaLine->mLineIndex;
        }

        for (auto iter = mSCTPMedias.begin(); iter != mSCTPMedias.end(); ++iter) {
          auto &mediaLine = (*iter).second;

          if (!result.hasValue()) {
            result = mediaLine->mLineIndex;
            continue;
          }

          if (!mediaLine->mLineIndex.hasValue()) continue;

          if (mediaLine->mLineIndex.value() <= result.value()) continue;

          result = mediaLine->mLineIndex;
        }

        if (!result.hasValue()) return static_cast<size_t>(0);
        return static_cast<size_t>(result.value() + 1);
      }

      //-----------------------------------------------------------------------
      void PeerConnection::moveAddedTracksToPending()
      {
        for (auto iter = mAddedPendingAddTracks.begin(); iter != mAddedPendingAddTracks.end(); ++iter) {
          auto &pending = (*iter);
          mPendingAddTracks.push_back(pending);
        }
        mAddedPendingAddTracks.clear();
      }

      //-----------------------------------------------------------------------
      void PeerConnection::processStats(
                                        PromiseWithStatsReportPtr collectionPromise,
                                        PromiseWithStatsReportPtr resolvePromise
                                        )
      {
        IStatsReportPtr report;

        // scope: go through the reports
        {
          report = collectionPromise->value();
          if (!report) goto done;

          auto ids = report->getStatesIDs();
          if (!ids) goto done;

          for (auto iter = ids->begin(); iter != ids->end(); ++iter) {
            auto &id = (*iter);

            auto stats = report->getStats(id);
            if (!stats) continue;

            stats->eventTrace();
          }
        }

      done:
        {
          if (report) {
            resolvePromise->resolve(report);
          } else {
            resolvePromise->reject();
          }
        }
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::MediaStreamListPtr PeerConnection::convertToList(const UseMediaStreamMap &useStreams)
      {
        auto result = make_shared<MediaStreamList>();
        for (auto iter = useStreams.begin(); iter != useStreams.end(); ++iter) {
          IMediaStreamPtr stream = MediaStream::convert((*iter).second);
          if (!stream) continue;
          result->push_back(stream);
        }
        return result;
      }

      //-----------------------------------------------------------------------
      PeerConnection::UseMediaStreamMapPtr PeerConnection::convertToMap(const MediaStreamList &mediaStreams)
      {
        auto result = make_shared<UseMediaStreamMap>();
        for (auto iter = mediaStreams.begin(); iter != mediaStreams.end(); ++iter) {
          UseMediaStreamPtr mediaStream = MediaStream::convert(*iter);
          if (!mediaStream) continue;
          (*result)[mediaStream->id()] = mediaStream;
        }
        return result;
      }

      //-----------------------------------------------------------------------
      IPeerConnectionTypes::MediaStreamSetPtr PeerConnection::convertToSet(const UseMediaStreamMap &useStreams)
      {
        auto result = make_shared<MediaStreamSet>();

        for (auto iter = useStreams.begin(); iter != useStreams.end(); ++iter) {
          UseMediaStreamPtr mediaStream = (*iter).second;
          if (!mediaStream) continue;
          (*result).insert(mediaStream->id());
        }

        return result;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionTypes::ICECandidateListPtr PeerConnection::convertCandidateList(IICETypes::CandidateList &source)
      {
        auto result = make_shared<ISessionDescriptionTypes::ICECandidateList>();

        for (auto iter = source.begin(); iter != source.end(); ++iter) {
          auto &candidate = (*iter);
          result->push_back(make_shared<IICETypes::Candidate>(candidate));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::calculateDelta(
                                          const MediaStreamSet &existingSet,
                                          const MediaStreamSet &newSet,
                                          MediaStreamSet &outAdded,
                                          MediaStreamSet &outRemoved
                                          )
      
      {
        outAdded.clear();
        outRemoved.clear();

        for (auto iter = existingSet.begin(); iter != existingSet.end(); ++iter) {
          auto &id = (*iter);

          auto found = newSet.find(id);
          if (found == newSet.end()) outRemoved.insert(id);
        }
        for (auto iter = newSet.begin(); iter != newSet.end(); ++iter) {
          auto &id = (*iter);

          auto found = existingSet.find(id);
          if (found == existingSet.end()) outAdded.insert(id);
        }
      }

      //-----------------------------------------------------------------------
      IDTLSTransportTypes::ParametersPtr PeerConnection::getDTLSParameters(
                                                                           const TransportInfo &transportInfo,
                                                                           IICETypes::Components component
                                                                           )
      {
        // scope: get parameters from active dtls transport
        {
          IDTLSTransportPtr transport;
          switch (component) {
            case IICETypes::Component_RTP:  transport = transportInfo.mRTP.mDTLSTransport; break;
            case IICETypes::Component_RTCP: transport = transportInfo.mRTCP.mDTLSTransport; break;
          }

          if (transport) {
            auto result = transport->getLocalParameters();
            if (IDTLSTransportTypes::Role_Auto == result->mRole) {
              if (transportInfo.mLocalDTLSRole.hasValue()) {
                result->mRole = transportInfo.mLocalDTLSRole.value();
              }
            }
            return result;
          }
        }

        auto result = make_shared<IDTLSTransportTypes::Parameters>();

        if (transportInfo.mLocalDTLSRole.hasValue()) {
          result->mRole = transportInfo.mLocalDTLSRole.value();
        }

        for (auto iter = transportInfo.mCertificates.begin(); iter != transportInfo.mCertificates.end(); ++iter) {
          auto &certificate = (*iter);
          result->mFingerprints.push_back(*certificate->fingerprint());
        }

        return result;
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
        case IPeerConnectionTypes::BundlePolicy_Balanced:   return "balanced";
        case IPeerConnectionTypes::BundlePolicy_MaxCompat:  return "max-compat";
        case IPeerConnectionTypes::BundlePolicy_MaxBundle:  return "max-bundle";
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
        case ISessionDescriptionTypes::SignalingType_SDPPranswer:
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
        case IPeerConnectionTypes::PeerConnectionState_Closed:        return "closed";
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
      UseServicesHelper::debugAppend(resultEl, "ice candidate pool size", mICECandidatePoolSize);

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
    IPeerConnectionTypes::MediaStreamTrackConfiguration::MediaStreamTrackConfiguration(const MediaStreamTrackConfiguration &op2) :
      mCapabilities(op2.mCapabilities ? make_shared<IRTPTypes::Capabilities>(*op2.mCapabilities) : IRTPTypes::CapabilitiesPtr()),
      mParameters(op2.mParameters ? make_shared<IRTPTypes::Parameters>(*op2.mParameters) : IRTPTypes::ParametersPtr())
    {
    }

    //-------------------------------------------------------------------------
    ElementPtr IPeerConnectionTypes::MediaStreamTrackConfiguration::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::adapter::IPeerConnectionTypes::MediaStreamTrackConfiguration");
      UseServicesHelper::debugAppend(resultEl, mCapabilities ? mCapabilities->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, mParameters ? mParameters->toDebug() : ElementPtr());
      return resultEl;
    }

    //-------------------------------------------------------------------------
    IPeerConnectionTypes::MediaStreamTrackConfiguration &IPeerConnectionTypes::MediaStreamTrackConfiguration::operator=(const MediaStreamTrackConfiguration &op2)
    {
      mCapabilities = op2.mCapabilities ? make_shared<IRTPTypes::Capabilities>(*op2.mCapabilities) : IRTPTypes::CapabilitiesPtr();
      mParameters = op2.mParameters ? make_shared<IRTPTypes::Parameters>(*op2.mParameters) : IRTPTypes::ParametersPtr();
      return *this;
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
