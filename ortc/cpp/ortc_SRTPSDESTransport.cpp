  /*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#include <ortc/internal/ortc_SRTPSDESTransport.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_SRTPTransport.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_srtp) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  using zsLib::Log;
  using zsLib::Numeric;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SRTPSDESTransportSettingsDefaults);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransportSettingsDefaults
    //

    class SRTPSDESTransportSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~SRTPSDESTransportSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static SRTPSDESTransportSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<SRTPSDESTransportSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static SRTPSDESTransportSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<SRTPSDESTransportSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installSRTPSDESTransportSettingsDefaults() noexcept
    {
      SRTPSDESTransportSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => IStatsProvider
    //

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr SRTPSDESTransport::getStats(ZS_MAYBE_USED() const StatsTypeSet &stats) const noexcept
    {
      ZS_MAYBE_USED(stats);
#pragma ZS_BUILD_NOTE("TODO","Implement getStats")
      return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport
    //
    
    //-------------------------------------------------------------------------
    SRTPSDESTransport::SRTPSDESTransport(
                                         const make_private &,
                                         IMessageQueuePtr queue,
                                         ISRTPSDESTransportDelegatePtr originalDelegate,
                                         IICETransportPtr iceTransport
                                         ) noexcept(false) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSubscriptions(decltype(mSubscriptions)::create()),
      mSecureTransportSubscriptions(decltype(mSecureTransportSubscriptions)::create()),
      mICETransportRTP(ICETransport::convert(iceTransport))
    {
      ZS_LOG_DETAIL(debug("created"));

      ORTC_THROW_INVALID_PARAMETERS_IF(!mICETransportRTP);

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::init(
                                 const CryptoParameters &encryptParameters,
                                 const CryptoParameters &decryptParameters
                                 ) noexcept
    {
      AutoRecursiveLock lock(*this);

      mICETransportRTP->notifyAttached(mID, mThisWeak.lock());
      mICETransportSubscription = mICETransportRTP->subscribe(mThisWeak.lock());

      fixRTCPTransport();

      mRTPListener = UseRTPListener::create(mThisWeak.lock());
      mSRTPTransport = UseSRTPTransport::create(mThisWeak.lock(), mThisWeak.lock(), encryptParameters, decryptParameters);
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransport::~SRTPSDESTransport() noexcept
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ISRTPSDESTransportPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForRTPSenderPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }
    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForRTPReceiverPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }


    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForICETransportPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForSRTPPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForRTPListenerPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISRTPSDESTransport
    //

    //-------------------------------------------------------------------------
    ElementPtr SRTPSDESTransport::toDebug(SRTPSDESTransportPtr transport) noexcept
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::create(
                                                   ISRTPSDESTransportDelegatePtr delegate,
                                                   IICETransportPtr iceTransport,
                                                   const CryptoParameters &encryptParameters,
                                                   const CryptoParameters &decryptParameters
                                                   ) noexcept (false)
    {
      SRTPSDESTransportPtr pThis(make_shared<SRTPSDESTransport>(make_private{}, IORTCForInternal::queueORTC(), delegate, iceTransport));
      pThis->mThisWeak = pThis;
      pThis->init(encryptParameters, decryptParameters);
      return pThis;
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(IRTPTransportPtr rtpTransport) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, rtpTransport);
    }

    //-------------------------------------------------------------------------
    ISRTPSDESTransportSubscriptionPtr SRTPSDESTransport::subscribe(ISRTPSDESTransportDelegatePtr originalDelegate) noexcept
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      ISRTPSDESTransportSubscriptionPtr subscription = mSubscriptions.subscribe(ISRTPSDESTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      ISRTPSDESTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        SRTPSDESTransportPtr pThis = mThisWeak.lock();

      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICETransportPtr SRTPSDESTransport::transport() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return ICETransport::convert(mICETransportRTP);
    }

    //-------------------------------------------------------------------------
    IICETransportPtr SRTPSDESTransport::rtcpTransport() const noexcept
    {
      AutoRecursiveLock lock(*this);
      return ICETransport::convert(fixRTCPTransport());
    }

    //-------------------------------------------------------------------------
    ISRTPSDESTransportTypes::ParametersPtr SRTPSDESTransport::getLocalParameters() noexcept
    {
      return UseSRTPTransport::getLocalParameters();
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::stop() noexcept
    {
      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISecureTransport
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISecureTransportForRTPSender
    //

    //-------------------------------------------------------------------------
    ISecureTransportSubscriptionPtr SRTPSDESTransport::subscribe(ISecureTransportDelegatePtr originalDelegate) noexcept
    {
      ZS_LOG_DETAIL(log("subscribing to secure transport state"))

      AutoRecursiveLock lock(*this);

      ISecureTransportSubscriptionPtr subscription = mSecureTransportSubscriptions.subscribe(ISecureTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      ISecureTransportDelegatePtr delegate = mSecureTransportSubscriptions.delegate(subscription, true);

      if (delegate) {
        auto pThis = mThisWeak.lock();

        if (ISecureTransport::State_Pending != mSecureTransportState) {
          delegate->onSecureTransportStateChanged(pThis, mSecureTransportState);
        }
      }

      if (isShutdown()) {
        mSecureTransportSubscriptions.clear();
      }

      return subscription;
    }
    
    //-------------------------------------------------------------------------
    ISecureTransportTypes::States SRTPSDESTransport::state(ZS_MAYBE_USED() ISecureTransportTypes::States ignored) const noexcept
    {
      ZS_MAYBE_USED(ignored);
      return mSecureTransportState; // no lock needed
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::sendPacket(
                                       IICETypes::Components sendOverICETransport,
                                       IICETypes::Components packetType,
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       ) noexcept
    {
      ZS_LOG_TRACE(log("sending packet") + ZS_PARAM("send over transport", IICETypes::toString(sendOverICETransport)) + ZS_PARAM("packet type", IICETypes::toString(packetType)) + ZS_PARAM("length", bufferLengthInBytes))

      return mSRTPTransport->sendPacket(sendOverICETransport, packetType, buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    IICETransportPtr SRTPSDESTransport::getICETransport() const noexcept
    {
      return ICETransport::convert(mICETransportRTP);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISRTPSDESTransportForICETransport
    //

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::notifyAssociateTransportCreated(
                                                            ZS_MAYBE_USED() IICETypes::Components associatedComponent,
                                                            ZS_MAYBE_USED() ICETransportPtr assoicated
                                                            ) noexcept
    {
      ZS_MAYBE_USED(associatedComponent);
      ZS_MAYBE_USED(assoicated);
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::handleReceivedPacket(
                                                 IICETypes::Components viaTransport,
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 ) noexcept
    {
      ZS_LOG_TRACE(log("handle receive packet") + ZS_PARAM("via component", IICETypes::toString(viaTransport)) + ZS_PARAM("length", bufferLengthInBytes))

      if (isShutdown()) {
        ZS_LOG_WARNING(Debug, log("cannot receive packet on shutdown transport") + ZS_PARAM("via component", IICETypes::toString(viaTransport)) + ZS_PARAM("length", bufferLengthInBytes))
        return false;
      }

      ZS_LOG_INSANE(log("forwarding packet to SRTP transport") + ZS_PARAM("srtp transport id", mSRTPTransport->getID()) + ZS_PARAM("via", IICETypes::toString(viaTransport)) + ZS_PARAM("buffer length", bufferLengthInBytes))

      return mSRTPTransport->handleReceivedPacket(viaTransport, buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::handleReceivedSTUNPacket(
                                                     IICETypes::Components viaComponent,
                                                     STUNPacketPtr packet
                                                     ) noexcept
    {
      ZS_LOG_TRACE(log("handle receive stun packet") + ZS_PARAM("via component", IICETypes::toString(viaComponent))  + packet->toDebug())

      if (isShutdown()) {
        ZS_LOG_WARNING(Debug, log("cannot receive STUN packet on shutdown transport"))
        return;
      }

      // scope: pre-validation check
      {
        AutoRecursiveLock lock(*this);
#pragma ZS_BUILD_NOTE("TODO","Any reason to forward on a STUN packet")
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISecureTransportForSRTPTransport
    //

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::sendEncryptedPacket(
                                                IICETypes::Components sendOverICETransport,
                                                IICETypes::Components packetType,
                                                const BYTE *buffer,
                                                size_t bufferLengthInBytes
                                                ) noexcept
    {
      if (isShutdown()) {
        ZS_LOG_WARNING(Debug, log("cannot send packet on shutdown transport"))
        return false;
      }

      UseICETransportPtr transport = (IICETypes::Component_RTP == sendOverICETransport ? mICETransportRTP : fixRTCPTransport());
      if (!transport) {
        ZS_LOG_WARNING(Debug, log("no ice transport is attached") + ZS_PARAM("send over transport", IICETypes::toString(sendOverICETransport)) + ZS_PARAM("packet type", IICETypes::toString(packetType)))
        return false;
      }

      return transport->sendPacket(buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::handleReceivedDecryptedPacket(
                                                          IICETypes::Components viaTransport,
                                                          IICETypes::Components packetType,
                                                          const BYTE *buffer,
                                                          size_t bufferLengthInBytes
                                                          ) noexcept
    {
      if (isShutdown()) {
        ZS_LOG_WARNING(Debug, log("cannot receive packet on shutdown transport"))
        return false;
      }

      ZS_LOG_INSANE(log("forwarding packet to RTP listener") + ZS_PARAM("rtp listener id", mRTPListener->getID()) + ZS_PARAM("via", IICETypes::toString(viaTransport)) + ZS_PARAM("packet type", IICETypes::toString(packetType)) + ZS_PARAM("buffer length", bufferLengthInBytes))

      return mRTPListener->handleRTPPacket(viaTransport, packetType, buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISecureTransportForRTPListener
    //

    //-------------------------------------------------------------------------
    RTPListenerPtr SRTPSDESTransport::getListener() const noexcept
    {
      return RTPListener::convert(mRTPListener);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISRTPSDESTransportAsyncDelegate
    //

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onAttachRTCP()
    {
      UseICETransportPtr iceTransport;

      {
        AutoRecursiveLock lock(*this);

        if (mAttachedRTCP) {
          ZS_LOG_TRACE(log("already attached to ICE RTCP"))
          return;
        }

        iceTransport = fixRTCPTransport();
        if (!iceTransport) {
          ZS_LOG_WARNING(Detail, log("no RTCP transport present on ICE transport"))
          return;
        }

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Detail, log("cannot attach to ICE transport while shutting down / shutdown"))
          return;
        }

        mAttachedRTCP = true;
      }

      iceTransport->notifyAttached(mID, mThisWeak.lock());
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISRTPTransportDelegate
    //

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onSRTPTransportLifetimeRemaining(
                                                             ISRTPTransportPtr transport,
                                                             ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                             ULONG overallLifetimeRemainingPercentage
                                                             )
    {
      ZS_LOG_TRACE(log("lifetime remaining") + ZS_PARAM("least % for keys", leastLifetimeRemainingPercentageForAllKeys) + ZS_PARAM("% overall", overallLifetimeRemainingPercentage))

      AutoRecursiveLock lock(*this);
      mSubscriptions.delegate()->onSRTPSDESTransportLifetimeRemaining(mThisWeak.lock(), leastLifetimeRemainingPercentageForAllKeys, overallLifetimeRemainingPercentage);
      if (0 == overallLifetimeRemainingPercentage) {
        setError(UseHTTP::HTTPStatusCode_UpgradeRequired, "keying material is exhausted");
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => ISRTPTransportDelegate
    //

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportStateChange(
                                                      IICETransportPtr transport,
                                                      IICETransport::States state
                                                      )
    {
      ZS_LOG_DEBUG(log("ice transport state changed") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("state", IICETransportTypes::toString(state)))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportCandidatePairAvailable(
                                                                 IICETransportPtr transport,
                                                                 CandidatePairPtr candidatePair
                                                                 )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportCandidatePairGone(
                                                            IICETransportPtr transport,
                                                            CandidatePairPtr candidatePair
                                                            )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportCandidatePairChanged(
                                                               IICETransportPtr transport,
                                                               CandidatePairPtr candidatePair
                                                               )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport => (internal)
    //

    //-------------------------------------------------------------------------
    Log::Params SRTPSDESTransport::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("ortc::SRTPSDESTransport");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SRTPSDESTransport::debug(const char *message) const noexcept
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr SRTPSDESTransport::toDebug() const noexcept
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::SRTPSDESTransport");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      IHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      IHelper::debugAppend(resultEl, "secure transport subscriptions", mSecureTransportSubscriptions.size());
      IHelper::debugAppend(resultEl, "secure transport state", ISecureTransportTypes::toString(mSecureTransportState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      IHelper::debugAppend(resultEl, "ice rtp transport", mICETransportRTP ? mICETransportRTP->getID() : 0);
      IHelper::debugAppend(resultEl, "ice attached rtcp", mAttachedRTCP);
      IHelper::debugAppend(resultEl, "ice rtcp transport", mICETransportRTCP ? mICETransportRTCP->getID() : 0);

      IHelper::debugAppend(resultEl, "srtp transport", mSRTPTransport ? mSRTPTransport->getID() : 0);

      IHelper::debugAppend(resultEl, "rtp listener", mRTPListener ? mRTPListener->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::isShuttingDown() const noexcept
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::isShutdown() const noexcept
    {
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::step() noexcept
    {
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step calling cancel (already shutting down / shutdown)"))
        cancel();
        return;
      }

      if (!stepIceState()) goto not_ready;

    not_ready:
      {
      }
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::stepIceState() noexcept
    {
      auto state = mICETransportRTP->state();

      switch (state) {
        case IICETransportTypes::State_Connected:
        case IICETransportTypes::State_Completed:
        {
          setState(ISecureTransportTypes::State_Connected);
          break;
        }
        case IICETransportTypes::State_New:
        case IICETransportTypes::State_Checking:
        case IICETransportTypes::State_Disconnected:
        case IICETransportTypes::State_Failed:
        case IICETransportTypes::State_Closed:
        {
          setState(ISecureTransportTypes::State_Disconnected);
          break;
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::cancel() noexcept
    {
      if (mShutdown) return;

      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
      }

      //.......................................................................
      // final cleanup

      if (mICETransportRTP) {
        mICETransportRTP->notifyDetached(mID);
      }
      if ((mICETransportRTCP) &&
          (mAttachedRTCP)) {
        mICETransportRTCP->notifyDetached(mID);
      }

      setState(ISecureTransportTypes::State_Closed);

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
      mShutdown = true;
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::setError(WORD errorCode, const char *inReason) noexcept
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

      auto pThis = mThisWeak.lock();
      if (pThis) {
        ErrorAnyPtr error(make_shared<ErrorAny>());
        error->mErrorCode = mLastError;
        error->mReason = mLastErrorReason;
        mSubscriptions.delegate()->onSRTPSDESTransportError(mThisWeak.lock(), error);
      }

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::setState(ISecureTransportTypes::States state) noexcept
    {
      if (state == mSecureTransportState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", ISecureTransportTypes::toString(state)) + ZS_PARAM("old state", ISecureTransportTypes::toString(mSecureTransportState)))

      mSecureTransportState = state;

      auto pThis = mThisWeak.lock();
      if (pThis) {
        mSecureTransportSubscriptions.delegate()->onSecureTransportStateChanged(pThis, mSecureTransportState);
      }
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransport::UseICETransportPtr SRTPSDESTransport::fixRTCPTransport() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (!mICETransportRTCP) {
        mICETransportRTCP = mICETransportRTP->getRTCPTransport();
        if (mICETransportRTCP) {
          // ensure SRTP is attached to RTCP transport
          ISRTPSDESTransportAsyncDelegateProxy::create(mThisWeak.lock())->onAttachRTCP();
        }
      }
      return mICETransportRTCP;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISRTPSDESTransportFactory
    //

    //-------------------------------------------------------------------------
    ISRTPSDESTransportFactory &ISRTPSDESTransportFactory::singleton() noexcept
    {
      return SRTPSDESTransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ISRTPSDESTransportFactory::ParametersPtr ISRTPSDESTransportFactory::getLocalParameters() noexcept
    {
      if (this) {}
      return internal::SRTPSDESTransport::getLocalParameters();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr ISRTPSDESTransportFactory::create(
      ISRTPSDESTransportDelegatePtr delegate,
      IICETransportPtr iceTransport,
      const CryptoParameters &encryptParameters,
      const CryptoParameters &decryptParameters
      ) noexcept
    {
      if (this) {}
      return internal::SRTPSDESTransport::create(delegate, iceTransport, encryptParameters, decryptParameters);
    }
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportTypes
  //

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportTypes::CryptoParameters
  //

  //---------------------------------------------------------------------------
  ISRTPSDESTransportTypes::CryptoParameters::CryptoParameters(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::CryptoParameters", "tag", mTag);
    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::CryptoParameters", "cryptoSuite", mCryptoSuite);

    {
      ElementPtr keyParamsEl = elem->findFirstChildElement("keyParams");
      if (keyParamsEl) {
        ElementPtr keyParamEl = keyParamsEl->findFirstChildElement("keyParam");
        while (keyParamEl) {
          KeyParameters param(keyParamEl);
          mKeyParams.push_back(param);
          keyParamEl = keyParamEl->findNextSiblingElement("keyParam");
        }
      }
    }

    {
      ElementPtr sessionParamsEl = elem->findFirstChildElement("sessionParams");
      if (sessionParamsEl) {
        ElementPtr sessionParamEl = sessionParamsEl->findFirstChildElement("sessionParam");
        while (sessionParamEl) {
          mSessionParams.push_back(IHelper::getElementTextAndDecode(sessionParamEl));
          sessionParamEl = sessionParamEl->findNextSiblingElement("sessionParam");
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::CryptoParameters::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "tag", mTag);
    IHelper::adoptElementValue(elem, "cryptoSuite", mCryptoSuite, false);

    if (mKeyParams.size() > 0) {
      ElementPtr keyParamsEl = Element::create("keyParams");

      for (auto iter = mKeyParams.begin(); iter != mKeyParams.end(); ++iter) {
        auto &value = (*iter);
        keyParamsEl->adoptAsLastChild(value.createElement("keyParam"));
      }
      elem->adoptAsLastChild(keyParamsEl);
    }

    if (mSessionParams.size() > 0) {
      ElementPtr sessionParamsEl = Element::create("sessionParams");
      for (auto iter = mSessionParams.begin(); iter != mSessionParams.end(); ++iter) {
        auto &value = (*iter);
        sessionParamsEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("sessionParam", value));
      }
      elem->adoptAsLastChild(sessionParamsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::CryptoParameters::toDebug() const noexcept
  {
    return createElement("ortc::ISRTPSDESTransportTypes::CryptoParameters");
  }

  //---------------------------------------------------------------------------
  String ISRTPSDESTransportTypes::CryptoParameters::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ISRTPSDESTransportTypes:CryptoParameters:");

    hasher->update(mTag);
    hasher->update(":");
    hasher->update(mCryptoSuite);

    hasher->update(":keyParams");

    {
      for (auto iter = mKeyParams.begin(); iter != mKeyParams.end(); ++iter) {
        auto keyParam = (*iter);
        auto hash = keyParam.hash();
        hasher->update(":");
        hasher->update(hash);
      }
    }

    hasher->update(":sessionParams");

    {
      ElementPtr listEl = Element::create("sessionParams");

      for (auto iter = mSessionParams.begin(); iter != mSessionParams.end(); ++iter) {
        auto sessionParam = (*iter);
        hasher->update(":");
        hasher->update(sessionParam);
      }
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportTypes::Parameters
  //

  //---------------------------------------------------------------------------
  ISRTPSDESTransportTypes::Parameters::Parameters(ElementPtr elem) noexcept
  {
    if (!elem) return;

    {
      ElementPtr cryptoParamsEl = elem->findFirstChildElement("cryptoParams");
      if (cryptoParamsEl) {
        ElementPtr cryptoParamEl = cryptoParamsEl->findFirstChildElement("cryptoParam");
        while (cryptoParamEl) {
          CryptoParameters param(cryptoParamEl);
          mCryptoParams.push_back(param);
          cryptoParamEl = cryptoParamEl->findNextSiblingElement("cryptoParam");
        }
      }
    }

  }

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::Parameters::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    if (mCryptoParams.size() > 0) {
      ElementPtr cryptoParamsEl = Element::create("cryptoParams");

      for (auto iter = mCryptoParams.begin(); iter != mCryptoParams.end(); ++iter) {
        auto &value = (*iter);
        cryptoParamsEl->adoptAsLastChild(value.createElement("cryptoParam"));
      }
      elem->adoptAsLastChild(cryptoParamsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::Parameters::toDebug() const noexcept
  {
    return createElement("ortc::ISRTPSDESTransportTypes::Parameters");
  }

  //---------------------------------------------------------------------------
  String ISRTPSDESTransportTypes::Parameters::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ISRTPSDESTransportTypes:Parameters:");

    {
      for (auto iter = mCryptoParams.begin(); iter != mCryptoParams.end(); ++iter) {
        auto cryptoParam = (*iter);
        auto hash = cryptoParam.hash();
        hasher->update(":");
        hasher->update(hash);
      }
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportTypes::KeyParameters
  //

  //---------------------------------------------------------------------------
  ISRTPSDESTransportTypes::KeyParameters::KeyParameters(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::KeyParameters", "keyMethod", mKeyMethod);
    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::KeyParameters", "keySalt", mKeySalt);
    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::KeyParameters", "lifetime", mLifetime);
    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::KeyParameters", "mkiValue", mMKIValue);
    IHelper::getElementValue(elem, "ortc::ISRTPSDESTransportTypes::KeyParameters", "mkiLength", mMKILength);
  }

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::KeyParameters::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "keyMethod", mKeyMethod, false);
    IHelper::adoptElementValue(elem, "keySalt", mKeySalt, false);
    IHelper::adoptElementValue(elem, "lifetime", mLifetime, false);
    IHelper::adoptElementValue(elem, "mkiValue", mMKIValue, false);
    IHelper::adoptElementValue(elem, "mkiLength", mMKILength);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::KeyParameters::toDebug() const noexcept
  {
    return createElement("ortc::ISRTPSDESTransportTypes::KeyParameters");
  }

  //---------------------------------------------------------------------------
  String ISRTPSDESTransportTypes::KeyParameters::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ISRTPSDESTransportTypes:Parameters:");

    hasher->update(mKeyMethod);
    hasher->update(":");
    hasher->update(mKeySalt);
    hasher->update(":");
    hasher->update(mLifetime);
    hasher->update(":");
    hasher->update(mMKIValue);
    hasher->update(":");
    hasher->update(mMKILength);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransport
  //

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransport::toDebug(ISRTPSDESTransportPtr transport) noexcept
  {
    return internal::SRTPSDESTransport::toDebug(internal::SRTPSDESTransport::convert(transport));
  }

  //---------------------------------------------------------------------------
  ISRTPSDESTransportPtr ISRTPSDESTransport::convert(IRTPTransportPtr rtpTransport) noexcept
  {
    return internal::SRTPSDESTransport::convert(rtpTransport);
  }

  //---------------------------------------------------------------------------
  ISRTPSDESTransportPtr ISRTPSDESTransport::create(
                                                   ISRTPSDESTransportDelegatePtr delegate,
                                                   IICETransportPtr iceTransport,
                                                   const CryptoParameters &encryptParameters,
                                                   const CryptoParameters &decryptParameters
                                                   ) noexcept
  {
    return internal::ISRTPSDESTransportFactory::singleton().create(delegate, iceTransport, encryptParameters, decryptParameters);
  }

  //---------------------------------------------------------------------------
  ISRTPSDESTransport::ParametersPtr ISRTPSDESTransport::getLocalParameters() noexcept
  {
    return internal::ISRTPSDESTransportFactory::singleton().getLocalParameters();
  }
}
