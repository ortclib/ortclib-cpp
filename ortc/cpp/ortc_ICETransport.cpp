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

#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ICEGatherer.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>


#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/XML.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForICEGatherer
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForICETransportContoller
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IICETransportForICETransportContoller::toDebug(ForTransportContollerPtr transport)
    {
      if (!transport) return ElementPtr();
      return ICETransport::convert(transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForRTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ICETransport::ICETransport(
                               IMessageQueuePtr queue,
                               IICETransportDelegatePtr originalDelegate,
                               UseICEGathererPtr gatherer
                               ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mGatherer(ICEGatherer::convert(gatherer))
    {
      ZS_LOG_BASIC(debug("created"))

      mDefaultSubscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
    }

    //-------------------------------------------------------------------------
    void ICETransport::init()
    {
      AutoRecursiveLock lock(*this);

      if (mGatherer) {
        mGatherer->installTransport(mThisWeak.lock(), String());
        mGathererSubscription = mGatherer->subscribe(mThisWeak.lock());

        auto localCandidates = mGatherer->getLocalCandidates();
        for (auto iter = localCandidates->begin(); iter != localCandidates->end(); ++iter) {
          auto localCandidate = (*iter);

          CandidatePtr candidate(new Candidate(localCandidate));
          mLocalCandidates[candidate->hash()] = candidate;
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    ICETransport::~ICETransport()
    {
      if (isNoop()) return;

      ZS_LOG_BASIC(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(IICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForICEGathererPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForTransportContollerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForRTPTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr ICETransport::getStats() const throw(InvalidStateError)
    {
      AutoRecursiveLock lock(*this);
      ORTC_THROW_INVALID_STATE_IF(isShutdown() || isShuttingDown())

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      ITransportAsyncDelegateProxy::create(mThisWeak.lock())->onResolveStatsPromise(promise);
      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr ICETransport::toDebug(ICETransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::create(
                                         IICETransportDelegatePtr delegate,
                                         IICEGathererPtr inGatherer
                                         )
    {
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);
      if (gatherer) {
        ORTC_THROW_INVALID_PARAMETERS_IF(Component_RTCP == gatherer->component())
        ORTC_THROW_INVALID_PARAMETERS_IF(IICEGatherer::State_Complete == gatherer->state())
      }

      ICETransportPtr pThis(new ICETransport(IORTCForInternal::queueORTC(), delegate, gatherer));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    PUID ICETransport::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    IICETransportSubscriptionPtr ICETransport::subscribe(IICETransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IICETransportSubscriptionPtr subscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      IICETransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        ICETransportPtr pThis = mThisWeak.lock();

        if (IICETransportTypes::State_New != mCurrentState) {
          delegate->onICETransportStateChanged(pThis, mCurrentState);
        }

        if (!isShutdown()) {
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICEGathererPtr ICETransport::iceGatherer() const
    {
      AutoRecursiveLock lock(*this);
      ZS_LOG_TRACE(log("get gatherer") + ZS_PARAM("gatherer", mGatherer ? mGatherer->getID() : 0))
      return ICEGatherer::convert(mGatherer);
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::Roles ICETransport::role() const
    {
      AutoRecursiveLock lock(*this);
      ZS_LOG_TRACE(log("get role") + ZS_PARAM("role", IICETypes::toString(mOptions.mRole)))
      return mOptions.mRole;
    }

    //-------------------------------------------------------------------------
    IICETypes::Components ICETransport::component() const
    {
      ZS_LOG_TRACE(log("get component") + ZS_PARAM("component", IICETypes::toString(mComponent)))
      return mComponent;
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::States ICETransport::state() const
    {
      AutoRecursiveLock lock(*this);
      ZS_LOG_TRACE(log("get state") + ZS_PARAM("state", IICETransport::toString(mCurrentState)))
      return mCurrentState;
    }

    //-------------------------------------------------------------------------
    IICETypes::CandidateListPtr ICETransport::getRemoteCandidates() const
    {
      AutoRecursiveLock lock(*this);

      CandidateListPtr result(new CandidateList);
      for (auto iter = mRemoteCandidates.begin(); iter != mRemoteCandidates.end(); ++iter) {
        auto tempCandidate = (*iter).second;
        result->push_back(*tempCandidate);
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::CandidatePairPtr ICETransport::getNominatedCandidatePair() const
    {
      AutoRecursiveLock lock(*this);

      auto found = mWarmRoutes.begin();
      if (found == mWarmRoutes.end()) return CandidatePairPtr();

      auto pairing = (*found).second;

      CandidatePairPtr result(new CandidatePair);

      result->mLocal = CandidatePtr(new Candidate(*(pairing->mCandidatePair->mLocal)));
      result->mRemote = CandidatePtr(new Candidate(*(pairing->mCandidatePair->mRemote)));

      return result;
    }

    //-------------------------------------------------------------------------
    void ICETransport::start(
                             IICEGathererPtr inGatherer,
                             Parameters remoteParameters,
                             Optional<Options> options
                             ) throw (InvalidParameters)
    {
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);
      ORTC_THROW_INVALID_PARAMETERS_IF(!gatherer)
      ORTC_THROW_INVALID_STATE_IF(IICEGatherer::State_Closed == gatherer->state())

      ORTC_THROW_INVALID_PARAMETERS_IF(remoteParameters.mUsernameFragment.isEmpty())
      ORTC_THROW_INVALID_PARAMETERS_IF(remoteParameters.mPassword.isEmpty())

      AutoRecursiveLock lock(*this);

      ORTC_THROW_INVALID_STATE_IF(IICETransport::State_Closed == mCurrentState)

      if (options.hasValue()) {
        mOptionsHash = options.mType.hash();
        mOptions = options.mType;
      }

      auto oldParamHash = mRemoteParametersHash;

      mRemoteParametersHash = remoteParameters.hash();
      mRemoteParameters = remoteParameters;

      if (oldParamHash.hasData()) {
        ZS_LOG_DETAIL(log("remote ufrag has changed thus must flush out all remote candidates"))
        mRemoteCandidatesHash.clear();
        mRemoteCandidates.clear();
      }

      bool removedOldGatherer = false;

      if (gatherer != mGatherer) {
        ZS_LOG_DETAIL(log("local gatherer has changed thus must flush out all local candidates"))
        if (mGathererSubscription) {
          mGathererSubscription->cancel();
          mGathererSubscription.reset();
        }
        if (mGatherer) {
          mGatherer->removeTransport(*this);
          removedOldGatherer = true;
        }

        mGatherer = gatherer;

        mGatherer->installTransport(mThisWeak.lock(), String());
        mGathererSubscription = mGatherer->subscribe(mThisWeak.lock());

        auto localCandidates = mGatherer->getLocalCandidates();
        for (auto iter = localCandidates->begin(); iter != localCandidates->end(); ++iter) {
          auto localCandidate = (*iter);

          CandidatePtr candidate(new Candidate(localCandidate));
          mLocalCandidates[candidate->hash()] = candidate;
        }
        mEndOfLocalCandidates = false;
        mLocalCandidatesHash.clear();

        // all routes had to be destroyed
        if (removedOldGatherer) {
          mRoutes.clear();
        }
      }

      mComputedPairsHash.clear();

      RoutePtr currentRoute;

      for (auto iter_doNotUse = mLegalRoutes.begin(); iter_doNotUse != mLegalRoutes.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (mActiveRoute == route) {
          currentRoute = route;
          ZS_LOG_DEBUG(log("remembering previous route") + route->toDebug())
        }

        if (removedOldGatherer) {
          route->mGathererRouteID = 0;  // reset the route since it's already gone
        }

        ZS_LOG_DEBUG(log("shutting down route due to ice restart") + route->toDebug())
        shutdown(route);
      }

      mActiveRoute = 0;

      if (currentRoute) {
        // reactivate this route
      }

#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::stop()
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::ParametersPtr ICETransport::getRemoteParameters() const
    {
      AutoRecursiveLock lock(*this);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_WARNING(Debug, log("remote parameters have not been set"))
        return ParametersPtr();
      }

      ParametersPtr parameters(new Parameters);
      (*parameters) = mRemoteParameters;

      ZS_LOG_TRACE(log("obtained remote parameters") + parameters->toDebug())
      return parameters;
    }

    //-------------------------------------------------------------------------
    IICETransportPtr ICETransport::createAssociatedTransport() throw (InvalidStateError)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return IICETransportPtr();
    }

    //-------------------------------------------------------------------------
    void ICETransport::addRemoteCandidate(const GatherCandidate &remoteCandidate)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::setRemoteCandidates(const CandidateList &remoteCandidates)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeRemoteCandidate(const GatherCandidate &remoteCandidate)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::keepWarm(const CandidatePair &candidatePair)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForICEGatherer
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::notifyRouteAdded(
                                        PUID routeID,
                                        IICETypes::CandidatePtr localCandidate,
                                        const IPAddress &fromIP
                                        )
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyRouteRemoved(PUID routeID)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    PUID routeID,
                                    STUNPacketPtr packet
                                    )
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    PUID routeID,
                                    const BYTE *buffer,
                                    size_t bufferSizeInBytes
                                    )
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::needsMoreCandidates() const
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForICETransportContoller
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::notifyControllerAttached(ICETransportControllerPtr controller)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyControllerDetached()
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    bool ICETransport::hasCandidatePairFoundation(
                                                  const String &localFoundation,
                                                  const String &remoteFoundation,
                                                  PromisePtr promise
                                                  )
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForRTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    bool ICETransport::sendPacket(
                                  const BYTE *buffer,
                                  size_t bufferSizeInBytes
                                  )
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => ITransportAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onWake()
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
    #pragma mark ICETransport => IICEGathererDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onICEGathererStateChanged(
                                                 IICEGathererPtr gatherer,
                                                 IICEGatherer::States state
                                                 )
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICEGathererLocalCandidate(
                                                   IICEGathererPtr inGatherer,
                                                   CandidatePtr inCandidate
                                                   )
    {
      CandidatePtr candidate(new Candidate(*inCandidate));
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about candidate on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + candidate->toDebug())
        return;
      }

      bool shouldRecalculate = false;

      if (mEndOfLocalCandidates) {
        mEndOfLocalCandidates = false;
        mLocalCandidatesHash.clear();
        shouldRecalculate = true;
      }

      auto hash = candidate->hash();

      auto found = mLocalCandidates.find(hash);
      if (found != mLocalCandidates.end()) {
        ZS_LOG_DEBUG(log("local candidate is already known (thus ignoring)") + candidate->toDebug())
        goto check_recalculate;
      }

      mLocalCandidates[hash] = candidate;
      shouldRecalculate = true;

      ZS_LOG_DEBUG(log("found new local candidate") + candidate->toDebug())

      goto check_recalculate;

    check_recalculate:
      {
        if (!shouldRecalculate) return;

        mLocalCandidatesHash.clear();
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICEGathererLocalCandidateComplete(
                                                           IICEGathererPtr inGatherer,
                                                           CandidateCompletePtr candidate
                                                           )
    {
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about candidate on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + candidate->toDebug())
        return;
      }

      if (mEndOfLocalCandidates) {
        ZS_LOG_TRACE(log("already notified end of local candidates"))
        return;
      }

      ZS_LOG_DEBUG(log("end of local candidates found") + candidate->toDebug())

      mEndOfLocalCandidates = true;
      
      mLocalCandidatesHash.clear();
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICEGathererLocalCandidateGone(
                                                       IICEGathererPtr inGatherer,
                                                       CandidatePtr inCandidate
                                                       )
    {
      CandidatePtr candidate(new Candidate(*inCandidate));

      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about candidate on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + candidate->toDebug())
        return;
      }
      
      auto hash = candidate->hash();

      auto found = mLocalCandidates.find(hash);
      if (found == mLocalCandidates.end()) {
        ZS_LOG_WARNING(Debug, log("notified local candidate gone that was never known") + candidate->toDebug())
        return;
      }

      ZS_LOG_DEBUG(log("local candidate is now gone") + candidate->toDebug())

      mLocalCandidates.erase(found);
      
      mLocalCandidatesHash.clear();
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICEGathererError(
                                          IICEGathererPtr inGatherer,
                                          ErrorCode errorCode,
                                          String errorReason
                                          )
    {
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about error on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + ZS_PARAMIZE(errorCode) + ZS_PARAMIZE(errorReason))
        return;
      }

      ZS_LOG_WARNING(Detail, log("notified gatherer has encountered an error") + ZS_PARAMIZE(errorCode) + ZS_PARAMIZE(errorReason))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ICETransport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICETransport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICETransport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr ICETransport::toDebug() const
    {
      ElementPtr resultEl = Element::create("ICETransport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", IICETransport::toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "gatherer", mGatherer ? mGatherer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "gatherer subscription", mGathererSubscription ? mGathererSubscription->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return IICETransportTypes::State_Closed == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::isContinousGathering() const
    {
      if (mGatherer) return mGatherer->isContinousGathering();
      return false;
    }

    //-------------------------------------------------------------------------
    void ICETransport::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

    }

    //-------------------------------------------------------------------------
    void ICETransport::cancel()
    {
      //.......................................................................
      // start the shutdown process
      if (isShutdown()) {
        ZS_LOG_WARNING(Trace, log("already shutdown"))
        return;
      }

      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
      }

      //.......................................................................
      // final cleanup

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();

      if (mGatherer) {
        // detacht the installed transport
        mGatherer->removeTransport(*this);
        mGatherer.reset();
      }

      setState(IICETransportTypes::State_Closed);
    }

    //-------------------------------------------------------------------------
    void ICETransport::setState(IICETransport::States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("old state", IICETransport::toString(mCurrentState)) + ZS_PARAM("new state", state))

      mCurrentState = state;

      ICETransportPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onICETransportStateChanged(pThis, mCurrentState);
      }
    }

    //-----------------------------------------------------------------------
    void ICETransport::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));

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
    #pragma mark IICETransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IICETransportFactory &IICETransportFactory::singleton()
    {
      return ICETransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr IICETransportFactory::create(
                                                 IICETransportDelegatePtr delegate,
                                                 IICEGathererPtr gatherer
                                                 )
    {
      if (this) {}
      return internal::ICETransport::create(delegate, gatherer);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETransportTypes::toString(States state)
  {
    switch (state) {
      case State_New:           return "new";
      case State_Checking:      return "checking";
      case State_Connected:     return "connected";
      case State_Completed:     return "completed";
      case State_Disconnected:  return "disconnected";
      case State_Closed:        return "closed";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IICETransportTypes::States IICETransportTypes::toState(const char *state)
  {
    static States states[] = {
      State_New,
      State_Checking,
      State_Connected,
      State_Completed,
      State_Disconnected,
      State_Closed,
    };

    String stateStr(state);
    for (size_t loop = 0; loop < (sizeof(states) / sizeof(states[0])); ++loop) {
      if (stateStr == toString(states[loop])) return states[loop];
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + stateStr)
    return State_Closed;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes::CandidatePair
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::CandidatePair::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETransport::CandidatePair");

    UseServicesHelper::debugAppend(resultEl, "local", mLocal ? mLocal->toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "remote", mRemote ? mRemote->toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICETransport::CandidatePair::hash(bool includePriorities) const
  {
    SHA1Hasher hasher;

    String localHash = mLocal ? mLocal->hash(includePriorities) : String();
    String remoteHash = mRemote ? mRemote->hash(includePriorities) : String();

    hasher.update("candidate-pair:");
    hasher.update(localHash);
    hasher.update(":");
    hasher.update(remoteHash);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes::Options
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::Options::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETransport::Options");

    UseServicesHelper::debugAppend(resultEl, "aggressive ice", mAggressiveICE);
    UseServicesHelper::debugAppend(resultEl, "role", IICETypes::toString(mRole));

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICETransport::Options::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IICETransport::Options:");
    hasher.update(mAggressiveICE ? "true:" : "false:");
    hasher.update(IICETypes::toString(mRole));

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::toDebug(IICETransportPtr transport)
  {
    return internal::ICETransport::toDebug(internal::ICETransport::convert(transport));
  }

  //---------------------------------------------------------------------------
  IICETransportPtr IICETransport::create(
                                         IICETransportDelegatePtr delegate,
                                         IICEGathererPtr gatherer
                                         )
  {
    return internal::IICETransportFactory::singleton().create(delegate, gatherer);
  }


}
