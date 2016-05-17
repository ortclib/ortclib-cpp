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
#include <ortc/IRTPSender.h>
#include <ortc/IRTPReceiver.h>
#include <ortc/IMediaStreamTrack.h>

#include <ortc/adapter/IMediaStream.h>
#include <ortc/adapter/IHelper.h>

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
    ZS_DECLARE_TYPEDEF_PTR(adapter::IHelper, UseAdapterHelper);

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

        for (auto iter = mediaStreams.begin(); iter != mediaStreams.end(); ++iter) {
          auto &stream = (*iter);
          ORTC_THROW_INVALID_PARAMETERS_IF(!stream);
          pending->mMediaStreams.push_back(MediaStream::convert(stream));
        }

        mPendingAddTracks.push_back(pending);

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
      #pragma mark PeerConnection => IPromiseSettledDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void PeerConnection::onPromiseSettled(PromisePtr promise)
      {
        ZS_LOG_DEBUG("promise settled");

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
        UseServicesHelper::debugAppend(resultEl, "pending add tracks", mPendingAddTracks.size());
        UseServicesHelper::debugAppend(resultEl, "pending remove tracks", mPendingRemoveTracks.size());
        UseServicesHelper::debugAppend(resultEl, "pending add data channels", mPendingAddDataChannels.size());

        UseServicesHelper::debugAppend(resultEl, "transports", mTransports.size());
        UseServicesHelper::debugAppend(resultEl, "rtp medias", mRTPMedias.size());
        UseServicesHelper::debugAppend(resultEl, "sctp medias", mSCTPMedias.size());
        UseServicesHelper::debugAppend(resultEl, "senders", mSenders.size());
        UseServicesHelper::debugAppend(resultEl, "receivers", mReceivers.size());

        UseServicesHelper::debugAppend(resultEl, "pending remote candidates", mPendingRemoteCandidates.size());

        UseServicesHelper::debugAppend(resultEl, "pending transport pool", mTransportPool.size());

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
        if (!stepAddTracks()) return;
        if (!stepCreateOffer()) return;
        if (!stepProcessPendingRemoteCandidates()) return;
#define TODO_STEP 1
#define TODO_STEP 2
        if (!stepFinalizeSenders()) return;

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
          setError(UseHTTP::HTTPStatusCode_CertError, "certificate promise is not settled");
          cancel();
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
                case IPeerConnectionTypes::SignalingMode_SDP:   goto possible_bundle_but_not_a_match;
              }
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
                  mediaLine->mID = registerNewID(3);
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

            mSenders[senderInfo->mID] = senderInfo;

            notifyNegotiationNeeded();  // need to signal this to the remote party before it will be started
            continue;
          }
        }

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
              case ISessionDescriptionTypes::SignalingType_JSON:
              case ISessionDescriptionTypes::SignalingType_SDPOffer:    
              {
                break;
              }
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
              case ISessionDescriptionTypes::SignalingType_SDPPranswer:
              case ISessionDescriptionTypes::SignalingType_SDPAnswer:
              {
                break;
              }
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

        mPendingMethods.pop_front();

        auto description = pending->mSessionDescription->description();
        if (!description) {
          flushRemotePending(pending->mSessionDescription);
          ZS_LOG_WARNING(Debug, log("cannot accept remote description (as remote description is not valid)") + ZS_PARAM("signaling state", IPeerConnectionTypes::toString(mLastSignalingState)) + pending->toDebug());
          pending->mPromise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "received SetRemoteDescription but it does not appear to be valid"));
          wake();
          return false;
        }

        bool result = true;

        result = stepProcessRemoteTransport(description) && (result);
        result = stepProcessRemoteRTPMediaLines(description) && (result);
        result = stepProcessRemoteRTPSenders(description) && (result);

        return result;
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

            if (transportInfo->mNegotiationState) {
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

            bool hasRTPICE = true;
            bool hasRTCPICE = hasRTPICE && transport.mRTCP && (transportInfo->mRTCP.mTransport);
            auto useRTCPICEParams = hasRTCPICE ? (bool(transport.mRTCP->mICEParameters) ? transport.mRTCP->mICEParameters : transport.mRTP->mICEParameters) : IICETransportTypes::ParametersPtr();

            bool hasRTPDTLS = bool(transport.mRTP->mDTLSParameters);
            bool hasRTCPDTLS = hasRTCPICE && hasRTPDTLS && bool(transport.mRTCP);
            auto useRTCPDTLSParams = hasRTCPDTLS ? (transport.mRTCP->mDTLSParameters ? transport.mRTCP->mDTLSParameters : transport.mRTP->mDTLSParameters) : IDTLSTransportTypes::ParametersPtr();

            bool hasRTPSRTP = (!hasRTPDTLS) && bool(transport.mRTP->mSRTPSDESParameters) && (transport.mRTP->mSRTPSDESParameters->mCryptoParams.size() > 0);

            if ((!hasRTPDTLS) && (!hasRTPSRTP)) {
              ZS_LOG_WARNING(Debug, log("transport / info is missing secure transport") + transport.toDebug() + transportInfo->toDebug());
              goto reject_transport;
            }

            switch (transportInfo->mNegotiationState)
            {
              case NegotiationState_PendingOffer:
              case NegotiationState_LocalOffered:     options.mRole = IICETypes::Role_Controlling; break;
              case NegotiationState_RemoteOffered:    options.mRole = IICETypes::Role_Controlled;  break;
              case NegotiationState_Agreed:           break;
              case NegotiationState_Rejected:         break;  // will not happen
            }

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
                  transportInfo->mRTP.mDTLSTransport = IDTLSTransport::create(mThisWeak.lock(), transportInfo->mRTP.mTransport, mConfiguration.mCertificates);
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
                  transportInfo->mRTCP.mDTLSTransport = IDTLSTransport::create(mThisWeak.lock(), transportInfo->mRTCP.mTransport, mConfiguration.mCertificates);
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
              if (hasRTPSRTP) {
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
              } else {
                if (mediaLine.mDetails) {
                  if (mediaLine.mDetails->mInternalIndex.hasValue()) {
                    for (auto iterSearchByIndex = mRTPMedias.begin(); iterSearchByIndex != mRTPMedias.end(); ++iterSearchByIndex) {
                      auto &checkMediaLineInfo = (*iterSearchByIndex).second;
                      if (!checkMediaLineInfo->mLineIndex.hasValue()) continue;
                      if (checkMediaLineInfo->mLineIndex.value() != mediaLine.mDetails->mInternalIndex.value()) continue;

#define THIS_IS_LEGACY_APPROACH_TO_MATCH_MEDIA_LINES 1
#define THIS_IS_LEGACY_APPROACH_TO_MATCH_MEDIA_LINES 2

                      ZS_LOG_WARNING(Trace, log("found media line by index") + checkMediaLineInfo->toDebug());
                      mediaLineInfo = checkMediaLineInfo;
                      break;
                    }
                  }
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

            if ((!mediaLineInfo->mRemoteSenderCapabilities) ||
                (!mediaLineInfo->mRemoteReceiverCapabilities)) {
              ZS_LOG_WARNING(Detail, log("media line is missing remote capabilities") + mediaLine.toDebug());
              goto reject_media_line;
            }

            mediaLineInfo->mMediaType = mediaLine.mMediaType;
            mediaLineInfo->mBundledTransportID = mediaLine.mTransportID;
            mediaLineInfo->mPrivateTransportID = mediaLine.mDetails ? mediaLine.mDetails->mPrivateTransportID : String();
            mediaLineInfo->mRemoteSenderCapabilities = make_shared<IRTPTypes::Capabilities>(*mediaLine.mSenderCapabilities);
            mediaLineInfo->mRemoteReceiverCapabilities = make_shared<IRTPTypes::Capabilities>(*mediaLine.mReceiverCapabilities);
            mediaLineInfo->mBundledTransportID = mediaLine.mTransportID;
            mediaLineInfo->mPrivateTransportID = mediaLine.mDetails ? mediaLine.mDetails->mPrivateTransportID : String();

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

          // 
          {
            // scope: find receiver
            {
              auto found = mReceivers.find(sender.mID);
            }

          reject_receiver:
            {
              if (!receiverInfo) continue;
              close(*receiverInfo);
              continue;
            }

          accept_receiver:
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
              continue;
            }
          }

        }
        return true;
      }

      //-----------------------------------------------------------------------
      bool PeerConnection::stepCreateOffer()
      {
        ZS_LOG_TRACE(log("step - create offer"));

        switch (mLastSignalingState) {
          case IPeerConnectionTypes::SignalingState_Stable:                 break;
          case IPeerConnectionTypes::SignalingState_HaveRemoteOffer:        
          case IPeerConnectionTypes::SignalingState_HaveLocalOffer:
          case IPeerConnectionTypes::SignalingState_HaveLocalPranswer:
          case IPeerConnectionTypes::SignalingState_HaveRemotePranswer:
          case IPeerConnectionTypes::SignalingState_Closed: 
          {
            ZS_LOG_TRACE(log("skipping step to create an offer (in wrong signaling state)"));
            break;
          }
        }

        if (mPendingMethods.size() < 1) {
          ZS_LOG_TRACE(log("skipping step to creeate offer (no pending methods)"));
          return true;
        }

        PendingMethodPtr pending = mPendingMethods.front();
        mPendingMethods.pop_front();

        switch (pending->mMethod) {
          case PendingMethod_CreateOffer:
          case PendingMethod_CreateAnswer:
          case PendingMethod_CreateCapabilities:
          case PendingMethod_SetLocalDescription:
          case PendingMethod_SetRemoteDescription:  
          {
            break;
          }
        }

        switch (mConfiguration.mSignalingMode) {
          case IPeerConnectionTypes::SignalingMode_JSON:
          case IPeerConnectionTypes::SignalingMode_SDP:
          {
            break;
          }
        }


        return true;
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
            auto parameters = senderInfo->mConfiguration->mParameters ? senderInfo->mConfiguration->mParameters : UseAdapterHelper::capabilitiesToParameters(*capsUnion);
            if (!UseAdapterHelper::isCompatible(*capsUnion, *parameters)) {
              reason = "sender is not compatible with remote party (thus removing)";
              ZS_LOG_WARNING(Debug, log(reason) + capsUnion->toDebug() + parameters->toDebug());
              goto remove_sender;
            }

            if (!UseAdapterHelper::hasSupportedMediaCodec(*parameters)) {
              reason = "sender is not compatible with remote party (thus removing)";
              ZS_LOG_WARNING(Debug, log(reason) + capsUnion->toDebug() + parameters->toDebug());
              goto remove_sender;
            }

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

            ZS_LOG_DEBUG(log("sender created") + ZS_PARAM("sender id", senderInfo->mSender->getID()) + parameters->toDebug());

            try {
              senderInfo->mSender->send(*parameters);
            } catch (const InvalidParameters &) {
              reason = "sender.send() caused InvalidParameters exception";
              goto remove_sender;
            } catch (const InvalidStateError &) {
              reason = "sender.send() caused InvalidState exception";
              goto remove_sender;
            }
            senderInfo->mPromise->resolve(senderInfo->mSender);
          }

        remove_sender:
          {
            ZS_LOG_WARNING(Detail, log(reason) + senderInfo->toDebug());
            senderInfo->mPromise->reject(ErrorAny::create(error, reason));
            unregisterID(senderInfo->mID);
            mSenders.erase(current);
            // ensure unlinked transports / media lines get removed
            wake();
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
        mTransports[transport->mID] = transport;
        return transport;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::addToTransportPool()
      {
        TransportInfoPtr info(make_shared<TransportInfo>());

        IICEGatherer::Options emptyOptions;
        info->mRTP.mGatherer = IICEGatherer::create(mThisWeak.lock(), mConfiguration.mGatherOptions ? (*(mConfiguration.mGatherOptions)) : emptyOptions);
        info->mRTP.mTransport = IICETransport::create(mThisWeak.lock(), info->mRTP.mGatherer);

        if (IPeerConnectionTypes::RTCPMuxPolicy_Negotiated == mConfiguration.mRTCPMuxPolicy) {
          info->mRTCP.mGatherer = info->mRTP.mGatherer->createAssociatedGatherer(mThisWeak.lock());
          info->mRTCP.mTransport = info->mRTCP.mTransport->createAssociatedTransport(mThisWeak.lock());
        }

        info->mRTP.mSRTPSDESParameters = ISRTPSDESTransport::getLocalParameters();

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
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(SCTPMediaLineInfo &mediaLineInfo)
      {
        if (mediaLineInfo.mSCTPTransport) {
          mediaLineInfo.mSCTPTransport->stop();
          mediaLineInfo.mSCTPTransport.reset();
        }
        mediaLineInfo.mNegotiationState = NegotiationState_Rejected;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(SenderInfo &senderInfo)
      {
        if (senderInfo.mSender) {
          senderInfo.mSender->stop();
          senderInfo.mSender.reset();
        }
        senderInfo.mNegotiationState = NegotiationState_Rejected;
      }

      //-----------------------------------------------------------------------
      void PeerConnection::close(ReceiverInfo &receiverInfo)
      {
        if (receiverInfo.mReceiver) {
          receiverInfo.mReceiver->stop();
          receiverInfo.mReceiver.reset();
        }
        receiverInfo.mNegotiationState = NegotiationState_Rejected;
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
