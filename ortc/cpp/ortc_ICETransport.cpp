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
#include <ortc/internal/ortc_ICETransportController.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>
#include <openpeer/services/ISettings.h>

#include <zsLib/XML.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)

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
    static QWORD getMaxQWORD()
    {
      QWORD maxQword {};
      memset(&maxQword, 0xFF, sizeof(maxQword));
      return maxQword;
    }

    //-------------------------------------------------------------------------
    static Time getLatest(const Time &time1, const Time &time2)
    {
      if (Time() == time1) return time2;
      if (Time() == time2) return time1;

      return time1 > time2 ? time1 : time2;
    }

    //-------------------------------------------------------------------------
    static bool isMagnitudeFaster(const Microseconds &fastestThusFar, const Microseconds &newSpeed)
    {
      if (Microseconds() == newSpeed) return false;
      if (newSpeed >= fastestThusFar) return false; // cannot be a magnide faster if equal or slower

      auto diff = fastestThusFar - newSpeed;

      if (fastestThusFar < Microseconds(40*1000)) { // less than 40ms requires a 10ms difference
        return diff > Microseconds(10*1000);
      }
      if (fastestThusFar < Microseconds(80*1000)) { // less than 80ms (but greater than 40) requires a 20ms difference
        return diff > Microseconds(20*1000);
      }
      if (fastestThusFar < Microseconds(150*1000)) { // less than 150ms (but greater than 80ms) requires a 30ms difference
        return diff > Microseconds(30*1000);
      }
      if (fastestThusFar < Microseconds(300*1000)) { // less than 300ms (but greater than 150ms) requires a 40ms difference
        return diff > Microseconds(40*1000);
      }
      if (fastestThusFar < Microseconds(1000*1000)) { // less than 1 second (but greater than 300ms) requires a 50ms difference
        return diff > Microseconds(50*1000);
      }
      if (fastestThusFar < Microseconds(2000*1000)) { // less than 2 seconds (but greater than 300) requires a 100ms difference
        return diff > Microseconds(100*1000);
      }
      if (fastestThusFar < Microseconds(5000*1000)) { // less than 5 seconds (but greater than 2 seconds) requires a 400ms difference
        return diff > Microseconds(400*1000);
      }

      // greater than 5 seconds requires a 1 second difference
      return diff > Microseconds(1000*1000);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IICETransportForSettings::applyDefaults()
    {
      UseSettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_MAX_CANDIDATE_PAIRS_TO_TEST, 100);

      UseSettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_ACTIVATION_TIMER_IN_MILLISECONDS, 40);

      UseSettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_NO_PACKETS_RECEVIED_RECHECK_CANDIDATES_IN_SECONDS, 7);

      UseSettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_EXPIRE_ROUTE_IN_SECONDS, 30);

      UseSettings::setBool(ORTC_SETTING_ICE_TRANSPORT_BLACKLIST_AFTER_CONSENT_REMOVAL, false);

      UseSettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_BASE_IN_MILLISECONDS, 4000);
      UseSettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_RANDOMIZED_ADD_TIME_IN_MILLISECONDS, 2000);
    }

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
      mGatherer(ICEGatherer::convert(gatherer)),
      mNoPacketsReceivedRecheckTime(UseSettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_NO_PACKETS_RECEVIED_RECHECK_CANDIDATES_IN_SECONDS)),
      mExpireRouteTime(UseSettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_EXPIRE_ROUTE_IN_SECONDS)),
      mBlacklistConsent(UseSettings::getBool(ORTC_SETTING_ICE_TRANSPORT_BLACKLIST_AFTER_CONSENT_REMOVAL)),
      mKeepWarmTimeBase(UseSettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_BASE_IN_MILLISECONDS)),
      mKeepWarmTimeRandomizedAddTime(UseSettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_RANDOMIZED_ADD_TIME_IN_MILLISECONDS))
    {
      ZS_LOG_BASIC(debug("created"))

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
      }

      auto resolverBuffer = UseServicesHelper::random(sizeof(mConflictResolver));
      memcpy(&mConflictResolver, resolverBuffer->BytePtr(), resolverBuffer->SizeInBytes());
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

      wakeUp();
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
    ICETransportPtr ICETransport::convert(ForSettingsPtr object)
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
          for (auto iter = mWarmRoutes.begin(); iter != mWarmRoutes.end(); ++iter) {
            auto route = (*iter).second;
            ZS_LOG_TRACE(log("reporting available candidate") + route->mCandidatePair->toDebug())
            delegate->onICETransportCandidatePairAvailable(pThis, cloneCandidatePair(route));
          }
        }

        if (mActiveRoute) {
          if (!mActiveRoute->mPrune) {
            delegate->onICETransportCandidatePairChanged(pThis, cloneCandidatePair(mActiveRoute));
          }
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

      if (!mActiveRoute) return CandidatePairPtr();

      CandidatePairPtr result(new CandidatePair);

      result->mLocal = CandidatePtr(new Candidate(*(mActiveRoute->mCandidatePair->mLocal)));
      result->mRemote = CandidatePtr(new Candidate(*(mActiveRoute->mCandidatePair->mRemote)));

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

      ZS_LOG_DETAIL(log("start called") + UseICEGatherer::toDebug(gatherer) + remoteParameters.toDebug() + (options.hasValue() ? options.mType.toDebug() : ElementPtr()))

      AutoRecursiveLock lock(*this);

      ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown())

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

      if (gatherer != mGatherer) {
        ZS_LOG_DETAIL(log("local gatherer has changed thus must flush out all local candidates"))
        if (mGathererSubscription) {
          mGathererSubscription->cancel();
          mGathererSubscription.reset();
        }
        if (mGatherer) {
          mGatherer->removeTransport(*this);
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
      }

      pruneAllCandidatePairs(true);

      wakeUp();
    }

    //-------------------------------------------------------------------------
    void ICETransport::stop()
    {
      ZS_LOG_DETAIL(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
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
      ICETransportPtr pThis;

      {
        AutoRecursiveLock lock(*this);

        UseICEGathererPtr rtcpGatherer;

        ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown())
        ORTC_THROW_INVALID_STATE_IF(IICETypes::Component_RTCP == mComponent)

        if (mGatherer) {
          rtcpGatherer = mGatherer->getRTCPGatherer();
          ORTC_THROW_INVALID_PARAMETERS_IF(Component_RTCP != rtcpGatherer->component())
          ORTC_THROW_INVALID_PARAMETERS_IF(IICEGatherer::State_Complete == rtcpGatherer->state())
        }

        pThis = ICETransportPtr(new ICETransport(IORTCForInternal::queueORTC(), IICETransportDelegatePtr(), rtcpGatherer));
        pThis->mThisWeak.lock();
        pThis->mRTPTransport = mThisWeak.lock();
        mRTCPTransport = pThis;
      }

      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void ICETransport::addRemoteCandidate(const GatherCandidate &remoteCandidate)
    {
      AutoRecursiveLock lock(*this);

      {
        const Candidate *tempCandidate = dynamic_cast<const IICETypes::Candidate *>(&remoteCandidate);
        if (tempCandidate) {
          auto hash = tempCandidate->hash();
          auto found = mRemoteCandidates.find(hash);
          if (found != mRemoteCandidates.end()) {
            if (mEndOfRemoteCandidates) {
              ZS_LOG_WARNING(Detail, log("told to add candidate that is already added (but no longer at end of remote candidates)") + tempCandidate->toDebug())
              mEndOfRemoteCandidates = false;
              goto changed;
            }
            ZS_LOG_WARNING(Detail, log("told to add candidate that is already added") + tempCandidate->toDebug())
            return;
          }

          ZS_LOG_DEBUG(log("adding remote candidate") + tempCandidate->toDebug())

          CandidatePtr candidate(new Candidate(*tempCandidate));
          mRemoteCandidates[hash] = candidate;
          goto changed;
        }

        const CandidateComplete *tempCandidateComplete = dynamic_cast<const IICETypes::CandidateComplete *>(&remoteCandidate);
        if (tempCandidateComplete) {
          if (!mEndOfRemoteCandidates) {
            ZS_LOG_DEBUG(log("end of remote candidates indicated"))
            mEndOfRemoteCandidates = true;
            goto changed;
          }

          ZS_LOG_WARNING(Debug, log("end of remote candidates indicated (but already handled)"))
          return;
        }
      }

      ZS_LOG_WARNING(Detail, log("remote candidate type is not understood (thus cannot add)"))
      return;

    changed:
      {
        mComputedPairsHash.clear();
        mRemoteCandidatesHash.clear();

        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setRemoteCandidates(const CandidateList &remoteCandidates)
    {
      AutoRecursiveLock lock(*this);

      bool changed = mEndOfRemoteCandidates;  // if was at end of remote candidates then it must have changed

      mEndOfRemoteCandidates = false;

      CandidateMap foundCandidates;

      for (auto iter = remoteCandidates.begin(); iter != remoteCandidates.end(); ++iter) {
        auto tempCandidate = (*iter);

        CandidatePtr candidate(new Candidate(tempCandidate));
        auto hash = candidate->hash();

        foundCandidates[hash] = candidate;
      }

      for (auto iter_doNotUse = mRemoteCandidates.begin(); iter_doNotUse != mRemoteCandidates.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto hash = (*current).first;
        auto candidate = (*current).second;

        auto found = foundCandidates.find(hash);

        if (found != foundCandidates.end()) continue;

        ZS_LOG_DEBUG(log("removing remote candidate") + candidate->toDebug())

        mRemoteCandidates.erase(current);
        changed = true;
      }

      for (auto iter = foundCandidates.begin(); iter != foundCandidates.end(); ++iter) {
        auto hash = (*iter).first;
        auto candidate = (*iter).second;

        auto found = mRemoteCandidates.find(hash);
        if (found != mRemoteCandidates.end()) {
          ZS_LOG_TRACE(log("already have remote candidate") + candidate->toDebug())
          continue;
        }

        ZS_LOG_DEBUG(log("adding remote candidate") + candidate->toDebug())
        mRemoteCandidates[hash] = candidate;
        changed = true;
      }

      if (!changed) {
        ZS_LOG_WARNING(Debug, log("set remote candidates has not changed any remote candidates"))
        return;
      }

      mComputedPairsHash.clear();
      mRemoteCandidatesHash.clear();

      wakeUp();
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeRemoteCandidate(const GatherCandidate &remoteCandidate)
    {
      AutoRecursiveLock lock(*this);

      {
        const Candidate *tempCandidate = dynamic_cast<const IICETypes::Candidate *>(&remoteCandidate);
        if (tempCandidate) {
          auto hash = tempCandidate->hash();
          auto found = mRemoteCandidates.find(hash);
          if (found == mRemoteCandidates.end()) {
            ZS_LOG_WARNING(Detail, log("told to remove candidate that is already removed") + tempCandidate->toDebug())
            return;
          }

          ZS_LOG_DEBUG(log("removing remote candidate") + tempCandidate->toDebug())

          mRemoteCandidates.erase(found);
          goto changed;
        }

        const CandidateComplete *tempCandidateComplete = dynamic_cast<const IICETypes::CandidateComplete *>(&remoteCandidate);
        if (tempCandidateComplete) {
          if (mEndOfRemoteCandidates) {
            ZS_LOG_DEBUG(log("end of remote candidates no longer indicated"))
            mEndOfRemoteCandidates = false;
            goto changed;
          }

          ZS_LOG_WARNING(Debug, log("end of remote candidates no longer indicated (but was not already indicated thus no change happened)"))
          return;
        }
      }

      ZS_LOG_WARNING(Detail, log("remote candidate type is not understood (thus cannot remove)"))
      return;

    changed:
      {
        mComputedPairsHash.clear();
        mRemoteCandidatesHash.clear();

        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::keepWarm(
                                const CandidatePair &candidatePair,
                                bool keepWarm
                                )
    {
      AutoRecursiveLock lock(*this);

      auto hash = candidatePair.hash();

      auto found = mLegalRoutes.find(hash);
      if (found == mLegalRoutes.end()) {
        ZS_LOG_DETAIL(log("did not find any route to keep warm") + candidatePair.toDebug() + ZS_PARAMIZE(hash))
        return;
      }

      auto route = (*found).second;

      if (route->isBlacklisted()) {
        ZS_LOG_WARNING(Detail, log("cannot keep warm as route is blacklisted") + route->toDebug())
        return;
      }

      ZS_LOG_DETAIL(log("route found for keep warm") + route->toDebug() + ZS_PARAMIZE(keepWarm) + ZS_PARAMIZE(hash))

      route->mKeepWarm = keepWarm;

      wakeUp();
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
      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) || (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("cannot install gatherer route when shutting down/shutdown"))
        return;
      }

      auto route = findRoute(localCandidate, fromIP);
      if (route) {
        removeGathererRoute(route); // remove old route
        route->mGathererRouteID = routeID;

        ZS_LOG_DEBUG(log("installed gatherer route") + route->toDebug() + localCandidate->toDebug() + ZS_PARAM("from ip", fromIP.string()))
        return;
      }

      mPendingGathererRoutes[routeID] = LocalCandidateFromIPPair(localCandidate, fromIP);
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyRouteRemoved(PUID routeID)
    {
      AutoRecursiveLock lock(*this);

      // scope: remove from pending gatherer routes
      {
        auto found = mPendingGathererRoutes.find(routeID);
        if (found != mPendingGathererRoutes.end()) {
          ZS_LOG_DEBUG(log("removing gatherer route") + ZS_PARAMIZE(routeID))
          mPendingGathererRoutes.erase(found);
          return;
        }
      }

      // scope: remove from installed gatherer routes
      {
        auto found = mGathererRoutes.find(routeID);

        if (found == mGathererRoutes.end()) {
          ZS_LOG_WARNING(Debug, log("did not find any route installed with this routet id") + ZS_PARAMIZE(routeID))
          return;
        }

        auto route = (*found).second;

        ZS_LOG_DEBUG(log("removing gatherer route") + route->toDebug())
        removeGathererRoute(route);
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    PUID routeID,
                                    STUNPacketPtr packet
                                    )
    {
      {
        AutoRecursiveLock lock(*this);

        if (STUNPacket::Method_Binding != packet->mMethod) goto not_related_stun_packet;
        if (STUNPacket::Class_Request != packet->mClass) goto not_related_stun_packet;

        if (handleSwitchRolesAndConflict(routeID, packet)) {
          ZS_LOG_DEBUG(log("role conflict handled") + packet->toDebug())
          return;
        }

        auto route = findOrCreateMissingRoute(routeID, packet);

        if (!route) {
          ZS_LOG_WARNING(Detail, log("route is not found that can handle this packet (thus ignoring packet)") + packet->toDebug())
          return;
        }

        mLastReceivedPacket = zsLib::now();

        if (mBlacklistConsent) {
          if (Time() != route->mLastReceivedCheck) {
            if (route->mLastReceivedCheck + mExpireRouteTime < mLastReceivedPacket) {
              ZS_LOG_DETAIL(log("consent on candidate has to be removed since too long since last consent granted") + route->toDebug())

              auto response = createErrorResponse(packet, STUNPacket::ErrorCode_Unauthorized);

              ZS_LOG_WARNING(Detail, log("returning conflict error to remote party") + ZS_PARAMIZE(routeID) + response->toDebug())
              sendPacket(routeID, response);

              setBlacklisted(route);
              return;
            }
          }
        }

        if (route->isBlacklisted()) {
          ZS_LOG_DETAIL(log("consent on candidate has already been removed (blacklisted)") + route->toDebug())

          auto response = createErrorResponse(packet, STUNPacket::ErrorCode_Unauthorized);

          ZS_LOG_WARNING(Debug, log("returning conflict error to remote party") + ZS_PARAMIZE(routeID) + response->toDebug())
          sendPacket(routeID, response);
          return;
        }

        route->mLastReceivedCheck = mLastReceivedPacket;
        if (Time() == route->mLastReceivedResponse) {
          if (route->mOutgoingCheck) {
            ZS_LOG_DEBUG(log("forcing a trigger check response immediately") + route->toDebug())
            route->mOutgoingCheck->retryRequestNow();
          }
        }

        if ((route->isFrozen()) ||
            (route->isPending()) ||
            (route->isFailed())) {
          ZS_LOG_DETAIL(log("going to activate candidate pair because of incoming request") + route->toDebug())
          setInProgress(route);
        }

        auto response = createBindResponse(packet, route);

        ZS_LOG_TRACE(log("sending binding response to remote party") + ZS_PARAMIZE(routeID) + route->toDebug() + response->toDebug())
        sendPacket(routeID, response);

        if (IICETypes::Role_Controlled == mOptions.mRole) {
          if (packet->mUseCandidateIncluded) {
            auto previousRoute = mActiveRoute;

            if (!mActiveRoute) {
              mActiveRoute = route;
            }

            if (mOptions.mAggressiveICE) {
              if (Time() == mLastReceivedUseCandidate) {
                mActiveRoute = route;
              } else if (mLastReceivedUseCandidate + Seconds(3) < mLastReceivedPacket) {
                ZS_LOG_DEBUG(log("enough time has passed from last receive candidate to this candidate to force it active (aggressive 3 second rule)") + ZS_PARAMIZE(mLastReceivedUseCandidate) + ZS_PARAMIZE(mLastReceivedPacket) + route->toDebug())
                mActiveRoute = route;
              } else {
                if (mActiveRoute->getPreference(false) > route->getPreference(false)) {
                  ZS_LOG_DEBUG(log("new route is preferred over active route (aggressive most preferred route rule)") + ZS_PARAM("new route", route->toDebug()) + ZS_PARAM("active route", mActiveRoute->toDebug()))
                  mActiveRoute = route;
                }
              }
            } else {
              mActiveRoute = route;
            }

            mLastReceivedUseCandidate = mLastReceivedPacket;

            if (previousRoute != mActiveRoute) {
              ZS_LOG_DEBUG(log("controlling side indicates to use this route") + mActiveRoute->toDebug())
              wakeUp();
            }
          }
        }

        updateAfterPacket(route);
        return;
      }

    not_related_stun_packet:
      {
        handlePassThroughSTUNPacket(routeID, packet);
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    PUID routeID,
                                    const BYTE *buffer,
                                    size_t bufferSizeInBytes
                                    )
    {
      {
        AutoRecursiveLock lock(*this);

        auto found = mGathererRoutes.find(routeID);
        if (found == mGathererRoutes.end()) {
          ZS_LOG_WARNING(Detail, log("route was not found") + ZS_PARAMIZE(routeID) + ZS_PARAMIZE(bufferSizeInBytes))
          return;
        }

        auto route = (*found).second;

        if (route->isBlacklisted()) {
          ZS_LOG_WARNING(Debug, log("no consent given on this route") + route->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
          return;
        }

        mLastReceivedPacket = zsLib::now();
        route->mLastReceivedMedia = mLastReceivedPacket;
        updateAfterPacket(route);

        if (!mActiveRoute) {
          ZS_LOG_WARNING(Debug, log("no active route chosen (thus must ignore packet)") + route->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
          return;
        }

        goto forward_attached_listener;
      }

    forward_attached_listener:
      {
#define TODO_FORWARD_PACKET_TO_ATTACHED_LISTENER 1
#define TODO_FORWARD_PACKET_TO_ATTACHED_LISTENER 2
      }
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::needsMoreCandidates() const
    {
      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Debug, log("do not need more candidate if shutting down"))
        return false;
      }

      if (isComplete()) {
        ZS_LOG_TRACE(log("no need for more candidates as transport is complete"))
        return false;
      }

      if (isContinousGathering()) {
        ZS_LOG_TRACE(log("need more candidates because not completed yet (and in continuous gathering mode)"))
        return true;
      }

      if (isDisconnected()) {
        if ((mEndOfLocalCandidates) &&
            (mEndOfRemoteCandidates)) {
          ZS_LOG_WARNING(Debug, log("no need to get more candidates as transport is disconnected"))
          return false;
        }

        ZS_LOG_TRACE(log("disconnected but end of candidates has not been received yet"))
        return true;
      }

      ZS_LOG_TRACE(log("more candidates are required as transport is not completed or disconnected"))
      return true;
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
      AutoRecursiveLock lock(*this);

      mTransportController = controller;
      wakeUp();
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyControllerDetached()
    {
      AutoRecursiveLock lock(*this);

      ZS_THROW_INVALID_ASSUMPTION_IF(IICETypes::Component_RTCP == mComponent)

      UseICETransportControllerPtr controller = mTransportController.lock();

      if (controller) {
        ZS_LOG_WARNING(Trace, log("notified about controller detachment for obsolete ice transport controller"))
        return;
      }

      mTransportController.reset();

      for (auto iter_doNotUse = mLegalRoutes.begin(); iter_doNotUse != mLegalRoutes.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        removeFrozenDependencies(route, false, ReasonNoMoreRelationshipPtr(new ReasonNoMoreRelationship));

        if (!route->isFrozen()) continue;

        ZS_LOG_WARNING(Debug, log("frozen dependency needs to be removed") + route->toDebug())
        setPending(route);
      }
    }

    //-------------------------------------------------------------------------
    bool ICETransport::hasCandidatePairFoundation(
                                                  const String &localFoundation,
                                                  const String &remoteFoundation,
                                                  PromisePtr promise
                                                  )
    {
      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) || (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("cannot attach promise to foundation when shutting down/shutdown"))
        return false;
      }

      LocalRemoteFoundationPair foundation(localFoundation, remoteFoundation);

      auto found = mFoundationRoutes.find(foundation);
      if (found == mFoundationRoutes.end()) {
        ZS_LOG_TRACE(log("no route found with this foundation") + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))
        return false;
      }

      RouteMap &routes = (*found).second;

      ZS_THROW_BAD_STATE_IF(0 == routes.size())

      if (routes.size() == 1) {
        auto foundRoute = routes.begin();
        auto route = (*foundRoute).second;

        ZS_LOG_DEBUG(log("installing dependency promise") + route->toDebug() + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))

        route->mDependentPromises.push_back(promise);
        return true;
      }

      // more than one route possible to freeze upon
      Promise::PromiseList promises;

      for (auto iter = routes.begin(); iter != routes.end(); ++iter) {
        auto route = (*iter).second;

        PromisePtr tempPromise(Promise::create(getAssociatedMessageQueue()));
        promises.push_back(tempPromise);

        ZS_LOG_DEBUG(log("installed dependency race promise") + route->toDebug() + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))

        route->mDependentPromises.push_back(tempPromise);
      }

      PromisePtr race = Promise::race(promises, getAssociatedMessageQueue());

      race->then(promise);
      race->background();

      ZS_LOG_DEBUG(log("installed dependency race promises") + ZS_PARAM("total", promises.size()) + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))
      return true;
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
      UseICEGathererPtr gatherer;
      RouteID route = 0;

      {
        AutoRecursiveLock lock(*this);

        if (!installGathererRoute(mActiveRoute)) {
          ZS_LOG_WARNING(Trace, log("cannot install a gatherer route") + (mActiveRoute ? mActiveRoute->toDebug() : ElementPtr()) + ZS_PARAM("buffer size", bufferSizeInBytes))
          return false;
        }

        gatherer = mGatherer;
        route = mActiveRoute->mGathererRouteID;
      }

      return gatherer->sendPacket(route, buffer, bufferSizeInBytes);
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
    void ICETransport::onNotifyPacketRetried(
                                             IICETypes::CandidatePtr localCandidate,
                                             IPAddress remoteIP,
                                             STUNPacketPtr stunPacket
                                             )
    {
      RouteID newRoute {};

      {
        AutoRecursiveLock lock(*this);

        if (!mGatherer) {
          ZS_LOG_WARNING(Detail, log("cannot handle packet as no gather is attached") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + stunPacket->toDebug())
          return;
        }

        newRoute = mGatherer->createRoute(mThisWeak.lock(), localCandidate, remoteIP);
        if (0 == newRoute) {
          ZS_LOG_WARNING(Detail, log("cannot handle packet as no route could be created") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + stunPacket->toDebug())
          return;
        }

        ZS_LOG_TRACE(log("retry handling of incoming packet via newly created pending route") + ZS_PARAM("new route id", newRoute) + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + stunPacket->toDebug())
        mPendingGathererRoutes[newRoute] = LocalCandidateFromIPPair(localCandidate, remoteIP);
      }

      // forward packet to be handled on newly established route
      notifyPacket(newRoute, stunPacket);
    }

    //-------------------------------------------------------------------------
    void ICETransport::onWarmRoutesChanged()
    {
      ZS_LOG_DEBUG(log("warm routes changed event fired"))

      AutoRecursiveLock lock(*this);
      mWarmRoutesChanged = -1;  // by setting negative this route will be considered handled (unless set positive again)

      step();
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
      mWakeUp = false;
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onTimer(TimerPtr timer)
    {
      ZS_LOG_TRACE(log("timer fired") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);

      if (timer == mExpireRouteTimer) {
        handleExpireRouteTimer();
        return;
      }
      if (timer == mLastReceivedPacketTimer) {
        handleLastReceivedPacket();
        return;
      }
      if (timer == mActivationTimer) {
        handleActivationTimer();
        return;
      }

      // scope: check keep warm timers
      {
        auto found = mNextKeepWarmTimers.find(timer);
        if (found == mNextKeepWarmTimers.end()) goto not_a_keep_warm_timer;

        RoutePtr route = (*found).second;
        mNextKeepWarmTimers.erase(found);
        handleNextKeepWarmTimer(route);
        return;
      }

    not_a_keep_warm_timer:
      {
        ZS_LOG_WARNING(Trace, log("notified about an obsolete timer") + ZS_PARAM("timer id", timer->getID()))
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onPromiseSettled(PromisePtr promise)
    {
      AutoRecursiveLock lock(*this);

      auto found = mFrozen.find(promise);
      if (found == mFrozen.end()) {
        ZS_LOG_WARNING(Debug, log("did not find any routes frozen upon promise"))
        return;
      }

      RoutePtr route = (*found).second;

      mFrozen.erase(found);
      route->mFrozenPromise.reset();

      if (!route->isFrozen()) {
        ZS_LOG_WARNING(Debug, log("route was not frozen (thus don't unfreeze)") + route->toDebug())
        return;
      }

      if (promise->isResolved()) {
        ZS_LOG_DEBUG(log("activating route now") + route->toDebug())
        setInProgress(route);
        return;
      }

      ReasonNoMoreRelationshipPtr reason = promise->reason<ReasonNoMoreRelationship>();
      if (reason) {
        ZS_LOG_DEBUG(log("activating route now (relationship to other frozen transport severed prematurely)") + route->toDebug())
        setInProgress(route);
        return;
      }

      ZS_LOG_WARNING(Debug, log("failing route with same foundation that failed in other transport"))
      setFailed(route);
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
      AutoRecursiveLock lock(*this);
      ZS_LOG_TRACE(log("ice gatherer state changed") + ZS_PARAM("gather id", gatherer->getID()) + ZS_PARAM("state", IICEGatherer::toString(state)))
      wakeUp();
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
        mComputedPairsHash.clear();
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
    #pragma mark ICETransport => ISTUNRequesterDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onSTUNRequesterSendPacket(
                                                 ISTUNRequesterPtr requester,
                                                 IPAddress destination,
                                                 SecureByteBlockPtr packet
                                                 )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!packet)

      PUID routeID = 0;
      UseICEGathererPtr gatherer;

      {
        AutoRecursiveLock lock(*this);

        if (!mGatherer) {
          ZS_LOG_WARNING(Trace, log("no gatherer attached thus cannot send stun request") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()))
          return;
        }

        gatherer = mGatherer;

        RoutePtr route;

        if (mUseCandidateRequest == requester) {
          if (mUseCandidateRoute) {
            route = mUseCandidateRoute;
            goto setup_route;
          }
          ZS_LOG_WARNING(Trace, log("no active route thus cannot sent use candidate request") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()))
          return;
        }

        {
          auto found = mOutgoingChecks.find(requester);
          if (found != mOutgoingChecks.end()) {
            route = (*found).second;
            goto setup_route;
          }
        }

        ZS_LOG_WARNING(Trace, log("stun requester is not found (thus ignoring request to send packet)") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()))
        return;

      setup_route:
        {
          if (0 == route->mGathererRouteID) {
            route->mGathererRouteID = mGatherer->createRoute(mThisWeak.lock(), route->mCandidatePair->mLocal, destination);
            if (0 != route->mGathererRouteID) mGathererRoutes[route->mGathererRouteID] = route;
            ZS_LOG_TRACE(log("installed new gatherer route (hopefully)") + route->toDebug())
          }

          routeID = route->mGathererRouteID;
          route->mLastSentCheck = zsLib::now();
        }
      }

      if (0 == routeID) {
        ZS_LOG_WARNING(Trace, log("cannot send packet as no route set") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()))
        return;
      }

      ZS_LOG_TRACE(log("sending stun packet") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()))
      gatherer->sendPacket(routeID, packet->BytePtr(), packet->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    bool ICETransport::handleSTUNRequesterResponse(
                                                   ISTUNRequesterPtr requester,
                                                   IPAddress fromIPAddress,
                                                   STUNPacketPtr response
                                                   )
    {
      AutoRecursiveLock lock(*this);

      if (!mGatherer) {
        ZS_LOG_WARNING(Debug, log("cannot handle stun response when no gatherer is associated"))
        return false;
      }

      RoutePtr route;

      // check if this is related to an outgoing check
      {
        auto found = mOutgoingChecks.find(requester);
        if (found != mOutgoingChecks.end()) {
          route = (*found).second;
          mOutgoingChecks.erase(found);
          goto found_route;
        }
      }

      {
        ZS_LOG_WARNING(Debug, log("response to request must be from obsolete request") + ZS_PARAM("request id", requester->getID()))
        return false;
      }

    found_route:

      // scope: verify the response comes from where request was sent (otherwise treat the result as invalid)
      {
        auto ip = route->mCandidatePair->mRemote->ip();
        if (fromIPAddress != ip) {
          ZS_LOG_WARNING(Detail, log("response ip does not match sent ip") + ZS_PARAM("from ip", fromIPAddress.string()) + ZS_PARAM("sent ip", ip.string()))
          return false;
        }
      }

      fix(response);

      mLastReceivedPacket = zsLib::now();

      if (STUNPacket::Class_ErrorResponse == response->mClass) {
        if (STUNPacket::ErrorCode_RoleConflict == response->mErrorCode) {
          // role conflict occured
          ZS_LOG_WARNING(Detail, log("role conflict detected") + response->toDebug())

          bool changed = false;

          if (response->mIceControllingIncluded) {
            changed = IICETypes::Role_Controlling == mOptions.mRole;
            mOptions.mRole = IICETypes::Role_Controlled;
          }
          if (response->mIceControlledIncluded) {
            changed = IICETypes::Role_Controlled == mOptions.mRole;
            mOptions.mRole = IICETypes::Role_Controlling;
          }

          if (changed) {
            mOptionsHash = mOptions.hash();
            pruneAllCandidatePairs(true);

            IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
          }
          return true;
        }

        ZS_LOG_WARNING(Debug, log("error response received to ice binding request and candidate is now being blacklisted (as consent was rejected)") + route->toDebug())
        setBlacklisted(route);
        return true;
      }

      route->mLastReceivedResponse = zsLib::now();

      if (route->mOutgoingCheck) {
        if (requester == route->mOutgoingCheck) {
          size_t totalTries = requester->getTotalTries();
          if (1 == totalTries) {
            if (Time() != route->mLastRoundTripCheck) {
              auto previousValue = route->mLastRouteTripMeasurement;

              route->mLastRouteTripMeasurement = zsLib::toMicroseconds(getLatest(mLastReceivedPacket, route->mLastRoundTripCheck) - route->mLastRoundTripCheck);
              ZS_LOG_TRACE(log("updated route round trip time") + route->toDebug())

              if (IICETypes::Role_Controlling == mOptions.mRole) {  // no need to pick if not in controlling role
                if (Microseconds() != previousValue) {
                  auto largest = previousValue > route->mLastRouteTripMeasurement ? previousValue : route->mLastRouteTripMeasurement;
                  auto smallest = previousValue < route->mLastRouteTripMeasurement ? previousValue : route->mLastRouteTripMeasurement;
                  if (isMagnitudeFaster(largest, smallest)) {
                    ZS_LOG_TRACE(log("magnitude difference in route measurement (pick route again)") + ZS_PARAM("previous", previousValue) + route->toDebug())
                    mForcePickRouteAgain = true;
                    wakeUp();
                  }
                } else {
                  ZS_LOG_TRACE(log("first time a round trip measurement has happened (pick route again)") + route->toDebug())
                  mForcePickRouteAgain = true;
                  wakeUp();
                }
              }
            }
          }
        }

        route->mLastRoundTripCheck = Time();
        route->mOutgoingCheck->cancel();
        route->mOutgoingCheck.reset();
      }

      bool keptWarm = false;
      if (mActiveRoute == route) {
        keptWarm = !(route->mPrune);
      }

      // scope: check if in the warm route list
      if (!keptWarm) {
        auto found = mWarmRoutes.find(route->mCandidatePairHash);
        if (found != mWarmRoutes.end()) {
          keptWarm = !(route->mPrune);
        }
      }

      if ((keptWarm) &&
          (!route->mNextKeepWarm)) {
        route->mNextKeepWarm = Timer::create(mThisWeak.lock(), zsLib::now() + mKeepWarmTimeBase + Milliseconds(UseServicesHelper::random(0, static_cast<size_t>(mKeepWarmTimeRandomizedAddTime.count()))));
        mNextKeepWarmTimers[route->mNextKeepWarm] = route;

        ZS_LOG_TRACE(log("installed keep warm timer") + route->toDebug())
      }

      updateAfterPacket(route);
      return false;
    }

    //-------------------------------------------------------------------------
    void ICETransport::onSTUNRequesterTimedOut(ISTUNRequesterPtr requester)
    {
      AutoRecursiveLock lock(*this);

      if (mUseCandidateRequest == requester) {
        ZS_LOG_WARNING(Detail, log("failed to select route to use") + (mUseCandidateRoute ? mUseCandidateRoute->toDebug() : ElementPtr()))
        mUseCandidateRequest->cancel();
        mUseCandidateRequest.reset();
        wakeUp();
        return;
      }

      auto found = mOutgoingChecks.find(requester);
      if (found == mOutgoingChecks.end()) {
        ZS_LOG_WARNING(Debug, log("timeout for an obsolete STUN requester") + ZS_PARAM("request id", requester->getID()))
        return;
      }

      auto route = (*found).second;

      ZS_LOG_WARNING(Debug, log("route has failed due to timeout") + route->toDebug())
      if (route->isSucceeded()) {
        if (mBlacklistConsent) {
          ZS_LOG_WARNING(Detail, log("route was successful but is now blacklisted from future attempts") + route->toDebug())
          setBlacklisted(route);
          return;
        }
      }
      setFailed(route);
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
      ElementPtr resultEl = Element::create("ortc::ICETransport");

      auto transportController = mTransportController.lock();
      auto rtcpTransport = mRTCPTransport.lock();

      UseServicesHelper::debugAppend(resultEl, "id", mID);


      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "component", IICETypes::toString(mComponent));

      UseServicesHelper::debugAppend(resultEl, "state", IICETransport::toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "gatherer", mGatherer ? mGatherer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "gatherer subscription", mGathererSubscription ? mGathererSubscription->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "transport controller", transportController ? transportController->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "rtp transport", mRTPTransport ? mRTPTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "rtcp transport", mRTPTransport ? mRTPTransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "wake up", mWakeUp);
      UseServicesHelper::debugAppend(resultEl, "warm routes changed", mWarmRoutesChanged);
      UseServicesHelper::debugAppend(resultEl, "force pick route again", mForcePickRouteAgain);

      UseServicesHelper::debugAppend(resultEl, "options hash", mOptionsHash);
      UseServicesHelper::debugAppend(resultEl, "options", mOptions.toDebug());
      UseServicesHelper::debugAppend(resultEl, "conflict resolver", mConflictResolver);

      UseServicesHelper::debugAppend(resultEl, "remote parameters hash", mRemoteParametersHash);
      UseServicesHelper::debugAppend(resultEl, "remote parameters", mRemoteParameters.toDebug());

      UseServicesHelper::debugAppend(resultEl, "local candidates hash", mLocalCandidatesHash);
      UseServicesHelper::debugAppend(resultEl, "local candidates", mLocalCandidates.size());
      UseServicesHelper::debugAppend(resultEl, "end of local candidates", mEndOfLocalCandidates);

      UseServicesHelper::debugAppend(resultEl, "remote candidates hash", mRemoteCandidatesHash);
      UseServicesHelper::debugAppend(resultEl, "remote candidates", mRemoteCandidates.size());
      UseServicesHelper::debugAppend(resultEl, "end of remote candidates", mEndOfRemoteCandidates);

      UseServicesHelper::debugAppend(resultEl, "computed pairs hash", mComputedPairsHash);
      UseServicesHelper::debugAppend(resultEl, "legal routes", mLegalRoutes.size());
      UseServicesHelper::debugAppend(resultEl, "foundation routes", mFoundationRoutes.size());

      UseServicesHelper::debugAppend(resultEl, "activation timer", mActivationTimer ? mActivationTimer->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "pending pairs hash", mPendingActivationPairsHash);
      UseServicesHelper::debugAppend(resultEl, "pending activation", mPendingActivation.size());

      UseServicesHelper::debugAppend(resultEl, "frozen", mFrozen.size());

      UseServicesHelper::debugAppend(resultEl, "active route", mActiveRoute ? mActiveRoute->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "warm routes", mWarmRoutes.size());

      UseServicesHelper::debugAppend(resultEl, "gatherer routes", mGathererRoutes.size());
      UseServicesHelper::debugAppend(resultEl, "pending gatherer routes", mPendingGathererRoutes.size());

      UseServicesHelper::debugAppend(resultEl, "outgoing checks", mOutgoingChecks.size());
      UseServicesHelper::debugAppend(resultEl, "next keep warm timers", mNextKeepWarmTimers.size());

      UseServicesHelper::debugAppend(resultEl, "use candidate route", mUseCandidateRoute ? mUseCandidateRoute->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "use candidate request", mUseCandidateRequest ? mUseCandidateRequest->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "last received use candidate", mLastReceivedUseCandidate);

      UseServicesHelper::debugAppend(resultEl, "last received packet", mLastReceivedPacket);
      UseServicesHelper::debugAppend(resultEl, "last received packet timer", mLastReceivedPacketTimer ? mLastReceivedPacketTimer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "no packets received recheck time", mNoPacketsReceivedRecheckTime);

      UseServicesHelper::debugAppend(resultEl, "expire route time", mExpireRouteTime);
      UseServicesHelper::debugAppend(resultEl, "expire route timer", mExpireRouteTimer ? mExpireRouteTimer->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "blacklist consent", mBlacklistConsent);

      UseServicesHelper::debugAppend(resultEl, "keep warm time base", mKeepWarmTimeBase);
      UseServicesHelper::debugAppend(resultEl, "keep warm randomized add time", mKeepWarmTimeRandomizedAddTime);

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

      if (!stepCalculateLegalPairs()) goto done;
      if (!stepPendingActivation()) goto done;
      if (!stepActivationTimer()) goto done;
      if (!stepPickRoute()) goto done;
      if (!stepDewarmRoutes()) goto done;
      if (!stepKeepWarmRoutes()) goto done;
      if (!stepExpireRouteTimer()) goto done;
      if (!stepLastReceivedPacketTimer()) goto done;

    done:
      {
        if (!stepSetCurrentState()) return;
      }
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepCalculateLegalPairs()
    {
      typedef std::map<Hash, CandidatePairPtr> CandidatePairMap;

      if (mComputedPairsHash.hasData()) {
        ZS_LOG_TRACE(log("already computed legal pairs"))
        return true;
      }

      CandidatePairMap pairings;

      for (auto iterLocal = mLocalCandidates.begin(); iterLocal != mLocalCandidates.end(); ++iterLocal) {
        auto localCandidate = (*iterLocal).second;

        if (IICETypes::CandidateType_Srflex == localCandidate->mCandidateType) {
          ZS_LOG_TRACE(log("eliminating server reflexive as a local candidate") + localCandidate->toDebug())
          continue;
        }

        for (auto iterRemote = mRemoteCandidates.begin(); iterRemote != mRemoteCandidates.end(); ++iterRemote) {
          auto remoteCandidate = (*iterRemote).second;

          // do not match unless protocols are compatible
          if (localCandidate->mProtocol != remoteCandidate->mProtocol) continue;
          if (IICETypes::Protocol_TCP == localCandidate->mProtocol) {
            switch (localCandidate->mTCPType) {
              case IICETypes::TCPCandidateType_Active:  if (IICETypes::TCPCandidateType_Passive != remoteCandidate->mTCPType) continue;
              case IICETypes::TCPCandidateType_Passive: if (IICETypes::TCPCandidateType_Active != remoteCandidate->mTCPType) continue;
              case IICETypes::TCPCandidateType_SO:      if (IICETypes::TCPCandidateType_SO != remoteCandidate->mTCPType) continue;
            }
          }

          CandidatePairPtr candidatePair(new CandidatePair);
          candidatePair->mLocal = localCandidate;
          candidatePair->mRemote = remoteCandidate;

          auto hash = candidatePair->hash();

          pairings[hash] = candidatePair;
        }
      }

      auto currentRoute = mActiveRoute;

      for (auto iter_doNotUse = mLegalRoutes.begin(); iter_doNotUse != mLegalRoutes.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        auto found = pairings.find(route->mCandidatePairHash);
        if (found != pairings.end()) {
          ZS_LOG_TRACE(log("route still exists (thus still legal)") + route->toDebug())
          pairings.erase(found);
          continue;
        }

        ZS_LOG_WARNING(Debug, log("route no longer valid (no candidate pairing found)") + route->toDebug())
        shutdown(route);
      }

      for (auto iter = pairings.begin(); iter != pairings.end(); ++iter)
      {
        auto hash = (*iter).first;
        auto candidatePair = (*iter).second;

        RoutePtr route(new Route);
        route->mCandidatePair = candidatePair;
        route->mCandidatePairHash = hash;

        ZS_LOG_DEBUG(log("found new legal route") + route->toDebug())
        mLegalRoutes[hash] = route;

        installFoundation(route);
      }

      SHA1Hasher hasher;

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
        auto hash = (*iter).first;

        hasher.update(hash);
        hasher.update(":");
      }

      mComputedPairsHash = hasher.final();
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepPendingActivation()
    {
      if (mPendingActivationPairsHash == mComputedPairsHash) {
        ZS_LOG_TRACE(log("already calculated pending activation"))
        return true;
      }

      ZS_LOG_DEBUG(log("calculating pending activation"))

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter)
      {
        auto hash = (*iter).first;
        auto route = (*iter).second;

        auto found = mPendingActivation.find(route->mPendingPriority);
        if (found != mPendingActivation.end()) {
          if ((*found).second == route) {
            ZS_LOG_TRACE(log("route is already pending activation") + route->toDebug())
            continue;
          }
          route->mPendingPriority = 0;
        }

        ZS_LOG_DEBUG(log("route is now pending activation") + route->toDebug())
        setPending(route);
      }

      mPendingActivationPairsHash = mComputedPairsHash;

      ZS_LOG_DEBUG(log("pending activation is now calculated") + ZS_PARAM("total pending", mPendingActivation.size()) + ZS_PARAM("hash", mPendingActivationPairsHash))
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepActivationTimer()
    {
      auto now = zsLib::now();

      if (isContinousGathering()) {
        if (Time() != mLastReceivedPacket) {
          if (mLastReceivedPacket + mNoPacketsReceivedRecheckTime < now) {
            ZS_LOG_TRACE(log("needs activation timer as no remote packet has been received for a while"))
            goto need_activation_timer;
          }
        }
      } else {
        if ((isComplete()) ||
            (isDisconnected())) {
          ZS_LOG_TRACE(log("no activation timer needed (nothing more to test)"))
          goto no_activation_timer;
        }
      }

      if (mPendingActivation.size() > 0) {
        ZS_LOG_TRACE(log("need activation timer as routes are pending activation"))
        goto need_activation_timer;
      }

      ZS_LOG_TRACE(log("no activation timer needed at this time"))

      goto no_activation_timer;

    no_activation_timer:
      {
        if (mActivationTimer) {
          ZS_LOG_DEBUG(log("no longer need activation timer"))
          mActivationTimer->cancel();
          mActivationTimer.reset();
        }
        return true;
      }

    need_activation_timer:
      {
        if (!mActivationTimer) {
          auto duration = Milliseconds(UseSettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_ACTIVATION_TIMER_IN_MILLISECONDS));
          ZS_LOG_DEBUG(log("creating activation timer") + ZS_PARAM("duration (ms)", duration))
          mActivationTimer = Timer::create(mThisWeak.lock(), duration);
        }
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepPickRoute()
    {
      if ((!hasWarmRoutesChanged()) &&
          (!mForcePickRouteAgain)) {
        ZS_LOG_TRACE(log("warm routes have not changed (thus no need to pick another route)"))
        return true;
      }

      if (Role_Controlled == mOptions.mRole) {
        ZS_LOG_TRACE(log("cannot choose route as in controlled role") + mOptions.toDebug())
        return true;
      }

      mForcePickRouteAgain = false;

      auto now = zsLib::now();

      QWORD chosenPreference = 0;
      RoutePtr chosenRoute;

      Microseconds fastestRoutePicked;

      for (auto iter = mWarmRoutes.begin(); iter != mWarmRoutes.end(); ++iter) {
        auto route = (*iter).second;
        auto preference = route->getPreference(Role_Controlling == mOptions.mRole);

        if (!chosenRoute) {
          ZS_LOG_TRACE(log("no route chosen thus must pick this route") + route->toDebug())
          goto chose_this_route;
        }

        // scope: can only consider if there has been remote activity inside the expected window of time
        {
          auto lastRemoteActivity = getLastRemoteActivity(route);
          if (Time() == lastRemoteActivity) {
            ZS_LOG_WARNING(Trace, log("will not consider this route because of inactivity") + route->toDebug())
            continue;
          }

          // if after double the no packets arrival window activity hasn't happened then do not consider this route as active anymore
          if (lastRemoteActivity + mNoPacketsReceivedRecheckTime + mNoPacketsReceivedRecheckTime < now) {
            ZS_LOG_WARNING(Trace, log("will not consider this route because of recent inactivity") + route->toDebug())
            continue;
          }
        }

        if (Microseconds() != fastestRoutePicked) {
          if (Microseconds() != route->mLastRouteTripMeasurement) {
            if (isMagnitudeFaster(fastestRoutePicked, route->mLastRouteTripMeasurement)) {
              ZS_LOG_TRACE(log("route has faster round trip time (thus will pick)") + route->toDebug() + ZS_PARAM("previous speed", fastestRoutePicked))
              goto chose_this_route;
            }
          }
        }

        if (preference >= chosenPreference) continue; // pick by s

      chose_this_route:
        {
          chosenRoute = route;
          chosenPreference = preference;
          fastestRoutePicked = route->mLastRouteTripMeasurement;
        }
      }

      if (mActiveRoute != chosenRoute) {
        if (!chosenRoute) {
          ZS_LOG_TRACE(log("no route to choose (thus cannot choose route at this time)"))
          return true;
        }

        mActiveRoute = chosenRoute;
        ZS_LOG_DETAIL(log("new route chosen") + mActiveRoute->toDebug())
        mSubscriptions.delegate()->onICETransportCandidatePairChanged(mThisWeak.lock(), cloneCandidatePair(mActiveRoute));
      } else {
        ZS_LOG_TRACE(log("no change in preferred route"))
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepUseCandidate()
    {
      if (IICETypes::Role_Controlling != mOptions.mRole) {
        ZS_LOG_TRACE(log("not in controlling role that cannot force candidate selection") + mOptions.toDebug())
        return true;
      }

      if (mOptions.mAggressiveICE) {
        ZS_LOG_TRACE(log("aggressive ice does not require nomination"))
        return true;
      }

      if (mActiveRoute == mUseCandidateRoute) {
        ZS_LOG_TRACE(log("active route is already nominated (or no route to nominate)") + (mActiveRoute ? mActiveRoute->toDebug() : ElementPtr()))
        return true;
      }

      if (!mActiveRoute) {
        ZS_LOG_TRACE(log("no route to nominate"))
        return true;
      }

      if (Time() == mActiveRoute->mLastReceivedResponse) {
        ZS_LOG_TRACE(log("cannot nominate candidate because a response was not received yet") + mActiveRoute->toDebug())
        return true;
      }

      if (mUseCandidateRequest) {
        ZS_LOG_DEBUG(log("removing previous use candidate"))
        mUseCandidateRequest->cancel();
        mUseCandidateRequest.reset();
      }

      ZS_LOG_DEBUG(log("nominating candidate") + mActiveRoute->toDebug())

      mUseCandidateRoute = mActiveRoute;
      mUseCandidateRequest = createBindRequest(mUseCandidateRoute, true);

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepDewarmRoutes()
    {
      auto now = zsLib::now();

      for (auto iter_doNotUse = mWarmRoutes.begin(); iter_doNotUse != mWarmRoutes.end();)
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        if (route == mActiveRoute) {
          ZS_LOG_INSANE(log("must keep active route warm") + route->toDebug())
          continue;
        }

        if (route->mLastReceivedCheck + mExpireRouteTime > now) {
          ZS_LOG_INSANE(log("have received check (thus do not expire)") + route->toDebug() + ZS_PARAM("expire (s)", mExpireRouteTime))
          continue;
        }

        if (route->mLastReceivedResponse + mExpireRouteTime > now) {
          ZS_LOG_INSANE(log("have received responce to check (thus do not expire)") + route->toDebug() + ZS_PARAM("expire (s)", mExpireRouteTime))
          continue;
        }

        ZS_LOG_DEBUG(log("route cannot be kept warm any longer (expired)") + route->toDebug() + ZS_PARAM("expires (s)", mExpireRouteTime))

        if (mBlacklistConsent) {
          setBlacklisted(route);
        } else {
          setFailed(route);
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepKeepWarmRoutes()
    {
      if (!mActiveRoute) {
        ZS_LOG_TRACE(log("cannot warm routes until an active route is picked"))
        return true;
      }

      for (auto iter_doNotUse = mWarmRoutes.begin(); iter_doNotUse != mWarmRoutes.end();)
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        if (route->mPrune) route->mKeepWarm = false;
        if (route->mKeepWarm) goto must_keep_warm;
        if (route == mActiveRoute) goto must_keep_warm;
        if (Microseconds() == route->mLastRouteTripMeasurement) goto must_keep_warm;  // need a round trip time measurement to happen

        goto do_not_keep_warm;

      must_keep_warm:
        {
          ZS_LOG_INSANE(log("route must be kept warm") + route->toDebug())

          if (route->mOutgoingCheck) {
            ZS_LOG_INSANE(log("already have outgoing check (thus do not need timer") + route->toDebug())
            continue;
          }
          if (route->mNextKeepWarm) {
            ZS_LOG_INSANE(log("already have next keep warm timer (thus do not need timer") + route->toDebug())
            continue;
          }

          ZS_LOG_DEBUG(log("installing keep warm timer") + route->toDebug())
          route->mNextKeepWarm = Timer::create(mThisWeak.lock(), zsLib::now() + mKeepWarmTimeBase + Milliseconds(UseServicesHelper::random(0, static_cast<size_t>(mKeepWarmTimeRandomizedAddTime.count()))));
          mNextKeepWarmTimers[route->mNextKeepWarm] = route;
          continue;
        }
      do_not_keep_warm:
        {
          ZS_LOG_INSANE(log("route does not need to be kept warm") + route->toDebug())
          continue;
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepExpireRouteTimer()
    {
      if (!hasWarmRoutesChanged()) {
        ZS_LOG_TRACE(log("warm routes have not changed thus nothing to do"))
        return true;
      }

      if ((mWarmRoutes.size() < 1) &&
          (!mActiveRoute)) {
        if (!mExpireRouteTimer) {
          ZS_LOG_TRACE(log("no expire route timer to shutdown (timer is not needed)"))
          return true;
        }
        ZS_LOG_DEBUG(log("shutting down expire route timer (as no warm routes to shutdown)"))
        mExpireRouteTimer->cancel();
        mExpireRouteTimer.reset();
        return true;
      }

      if (mExpireRouteTimer) {
        ZS_LOG_TRACE(log("already have an expire route timer (thus no need to setup again)"))
        return true;
      }

      ZS_LOG_DEBUG(log("setting up expire route timer") + ZS_PARAM("expire (s)", mExpireRouteTime))

      mExpireRouteTimer = Timer::create(mThisWeak.lock(), mExpireRouteTime);
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::stepLastReceivedPacketTimer()
    {
      if (!hasWarmRoutesChanged()) {
        ZS_LOG_TRACE(log("warm routes have not changed thus nothing to do"))
        return true;
      }

      if (isContinousGathering()) {
        ZS_LOG_TRACE(log("always need last received packet timer for continuous gathering"))
        goto needs_last_received_packet_timer;
      }

      if (mWarmRoutes.size() > 1) {
        ZS_LOG_TRACE(log("since more than one route warm - it is possible to resolve via an alternative warm candidate"))
        goto needs_last_received_packet_timer;
      }

      ZS_LOG_TRACE(log("no need to have last received packet timer when no alternative route is possible"))
      goto do_not_need_last_received_packet_timer;

    do_not_need_last_received_packet_timer:
      {
        if (!mLastReceivedPacketTimer) {
          ZS_LOG_TRACE(log("last received packet timer is already gone"))
          return true;
        }

        ZS_LOG_DEBUG(log("removing last received packet timer (no longer needed)"))
        mLastReceivedPacketTimer->cancel();
        mLastReceivedPacketTimer.reset();
        return true;
      }

    needs_last_received_packet_timer:
      {
        if (mLastReceivedPacketTimer) {
          ZS_LOG_TRACE(log("already have last received packet timer"))
          return true;
        }

        ZS_LOG_DEBUG(log("setting up last received packet timer") + ZS_PARAM("no packets received recheck time", mNoPacketsReceivedRecheckTime))
        mLastReceivedPacketTimer = Timer::create(mThisWeak.lock(), mNoPacketsReceivedRecheckTime);
      }
      
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::stepSetCurrentState()
    {
      if (mPendingGathererRoutes.size() > 0) {
        // pending routes but do we have active route
        if (mActiveRoute) {
          // connected at this moment
          setState(IICETransport::State_Connected);
          return true;
        }
        setState(IICETransport::State_Checking);
        return true;
      }

      if ((!mEndOfLocalCandidates) ||
          (!mEndOfRemoteCandidates)) {

        if (mActiveRoute) {
          setState(IICETransport::State_Connected);
          return true;
        }

        ZS_LOG_TRACE(log("end of local/remote candidates not present"))
        setState(IICETransport::State_Disconnected);
        return true;
      }

      // no more pending routes
      if (mActiveRoute) {
        setState(IICETransport::State_Completed);
        return true;
      }

      setState(IICETransport::State_Disconnected);
      return true;
    }

    //-------------------------------------------------------------------------
    void ICETransport::wakeUp()
    {
      if (mWakeUp) {
        ZS_LOG_TRACE(log("already requested wake-up"))
        return;
      }

      mWakeUp = true;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::warmRoutesChanged()
    {
      if (mWarmRoutesChanged > 0) {
        ZS_LOG_TRACE(log("already notified warm route changed"))
        return;
      }

      mWarmRoutesChanged = 1; // by setting to 1 warm routes will be checked again
      ITransportAsyncDelegateProxy::create(mThisWeak.lock())->onWarmRoutesChanged();
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::hasWarmRoutesChanged()
    {
      return 0 != mWarmRoutesChanged;
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
      setState(IICETransport::State_Disconnected);

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

      pruneAllCandidatePairs(false);

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
        if (mGatherer) {
          mGatherer->notifyTransportStateChange(pThis);
        }
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

    //-----------------------------------------------------------------------
    bool ICETransport::isConnected() const
    {
      return IICETransport::State_Connected == mCurrentState;
    }

    //-----------------------------------------------------------------------
    bool ICETransport::isComplete() const
    {
      return IICETransport::State_Completed == mCurrentState;
    }

    //-----------------------------------------------------------------------
    bool ICETransport::isDisconnected() const
    {
      return IICETransport::State_Disconnected == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void ICETransport::handleExpireRouteTimer()
    {
      ZS_LOG_DEBUG(log("handle expire route timer"))

      auto now = zsLib::now();

      bool expired = false;

      // scope: check if should expire active route
      {
        auto latest = getLastRemoteActivity(mActiveRoute);
        if (Time() != latest) {
          if (latest + mExpireRouteTime < now) {
            if (mBlacklistConsent) {
              ZS_LOG_WARNING(Detail, log("blacklisting consent on active route") + mActiveRoute->toDebug())
              setBlacklisted(mActiveRoute);
            } else {
              ZS_LOG_WARNING(Detail, log("active route has now failed") + mActiveRoute->toDebug())
              setFailed(mActiveRoute);
            }

            mActiveRoute.reset();
            expired = true;
          }
        }
      }

      // scope: check warm routes if they need to be expired
      {
        for (auto iter_doNotUse = mWarmRoutes.begin(); iter_doNotUse != mWarmRoutes.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto route = (*current).second;

          auto latest = getLastRemoteActivity(route);

          ZS_THROW_INVALID_ASSUMPTION_IF(Time() == latest)

          if (latest + mExpireRouteTime >= now) {
            ZS_LOG_TRACE(log("warm route is still warm") + route->toDebug())
            continue;
          }

          expired = true;
          mWarmRoutes.erase(current);
          warmRoutesChanged();

          ZS_LOG_WARNING(Debug, log("warm route no longer is warm") + route->toDebug())

          if (mBlacklistConsent) {
            ZS_LOG_WARNING(Detail, log("warm route is blacklisted due to consent removal") + route->toDebug())
            setBlacklisted(route);
          }
        }
      }

      if (!expired) return;

      ZS_LOG_DEBUG(log("at least one route was expired (thus need to recalculate state)"))
      step();
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::handleLastReceivedPacket()
    {
      ZS_LOG_DEBUG(log("handle last packet received timer"))

      auto now = zsLib::now();

      stepActivationTimer();  // the need for the activation timer might have changed

      if (Time() != mLastReceivedPacket) {
        if (mLastReceivedPacket + mNoPacketsReceivedRecheckTime > now) {
          ZS_LOG_TRACE(log("have received packet inside expecting window") + ZS_PARAMIZE(now) + ZS_PARAMIZE(mLastReceivedPacket) + ZS_PARAM("no packet received window (s)", mNoPacketsReceivedRecheckTime))
          return;
        }
      }

      ZS_LOG_WARNING(Debug, log("no packet received inside expected window (thus need to aggressively check for possible active routes)"))

      forceActive(mActiveRoute);

      for (auto iter = mWarmRoutes.begin(); iter != mWarmRoutes.end();)
      {
        auto route = (*iter).second;
        forceActive(route);
      }

      if (IICETypes::Role_Controlled == mOptions.mRole) {
        ZS_LOG_TRACE(log("cannot pick active route because in controlled state"))
        return;
      }

      auto previousRoute = mActiveRoute;

      ZS_LOG_TRACE(log("repick the route (in case it should change)"))
      mForcePickRouteAgain = true;
      stepPickRoute();

      if (previousRoute == mActiveRoute) {
        ZS_LOG_TRACE(log("active route did not change"))
        return;
      }

      ZS_LOG_DEBUG(log("active route changed (thus redo state machine)"))
      step();
    }

    //-------------------------------------------------------------------------
    void ICETransport::handleActivationTimer()
    {
      ZS_LOG_DEBUG(log("handle last packet received timer"))

      auto now = zsLib::now();

      for (auto iter_doNotUse = mPendingActivation.begin(); iter_doNotUse != mPendingActivation.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        if (IICETypes::Component_RTCP == mComponent) {

          // freeze upon RTP component
          if (!mRTPTransport) {
            ZS_LOG_TRACE(log("no ice transport to freeze upon (thus activating now)") + route->toDebug())
            goto activate_now;
          }

          PromisePtr promise = Promise::create(getAssociatedMessageQueue());
          bool found = mRTPTransport->hasCandidatePairFoundation(route->mCandidatePair->mLocal->mFoundation, route->mCandidatePair->mRemote->mFoundation, promise);
          if (!found) {
            ZS_LOG_TRACE(log("rtp transport does not have an appropriate candidate pair to freeze upon") + route->toDebug())
            goto activate_now;
          }

          ZS_LOG_TRACE(log("frozen upon base rtp transport (thus will attempt to start something else)") + route->toDebug())
          setFrozen(route, promise);
          continue;
        }

        // scope: freeze based upon transport controller
        {
          auto controller = mTransportController.lock();
          if (!controller) {
            ZS_LOG_TRACE(log("no ice controller found (thus activating now)") + route->toDebug())
            goto activate_now;
          }

          auto promise = controller->notifyWhenUnfrozen(mThisWeak.lock(), route->mCandidatePair->mLocal->mFoundation, route->mCandidatePair->mRemote->mFoundation);
          if (!promise) {
            ZS_LOG_TRACE(log("not frozen upon any other transport") + route->toDebug())
            goto activate_now;
          }

          ZS_LOG_TRACE(log("frozen upon another dependent rtp transport") + route->toDebug())

          setFrozen(route, promise);
          continue;
        }

      activate_now:
        {
          ZS_LOG_DEBUG(log("activating route") + route->toDebug())

          setInProgress(route);
          if (mPendingActivation.size() < 1) {
            ZS_LOG_DEBUG(log("might not need activation timer anymore (thus waking up)"))
            wakeUp();
          }
          goto check_activation_timer;
        }
      }

      if (!isContinousGathering()) {
        ZS_LOG_TRACE(log("not in continous activation mode (thus cannot activate anthing else)"))
        goto check_activation_timer;
      }

      if (Time() == mLastReceivedPacket) {
        ZS_LOG_TRACE(log("no packet received just yet (check if activation timer is needed)"))
        goto check_activation_timer;
      }

      if (mLastReceivedPacket + mNoPacketsReceivedRecheckTime > now) {
        ZS_LOG_TRACE(log("packet received recently thus no need to re-activate anything"))
        goto check_activation_timer;
      }

      // scope: try to activate an old route just in case something can be resolved
      {
        // try to activate previously successful routes
        for (int loop = 0; loop < 2; ++loop) {
          for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
            auto route = (*iter).second;

            if (0 == loop) {
              if (route->isSucceeded()) goto activate_previous_route;
            } else {
              if (route->isFailed()) goto activate_previous_route;
            }

            // do not activate this route (not in a state that can be activated)
            continue;

          activate_previous_route:
            {
              ZS_LOG_DEBUG(log("activating route due to possible failure situation about to happen") + route->toDebug())
              setInProgress(route);
              goto check_activation_timer;
            }
          }
        }
      }

      // scope: kill activation timer since nothing worked
      {
        ZS_LOG_DEBUG(log("no previous route was activated (thus temporarily disabling activation timer)"))

        if (mActivationTimer) {
          mActivationTimer->cancel();
          mActivationTimer.reset();
        }
        return;
      }

    check_activation_timer:
      {
        stepActivationTimer();  // need for activation timer might have changed
      }
    }

    //-----------------------------------------------------------------------
    void ICETransport::handleNextKeepWarmTimer(RoutePtr route)
    {
      if (route->mNextKeepWarm) {
        route->mNextKeepWarm->cancel();
        route->mNextKeepWarm.reset();
      }

      if (route->mOutgoingCheck) {
        ZS_LOG_TRACE(log("already have outgoing check (thus send a retry packet now)"))
        route->mOutgoingCheck->retryRequestNow();
        return;
      }

      if (route->isBlacklisted()) {
        ZS_LOG_WARNING(Debug, log("route is black listed (thus will not issue check)") + route->toDebug())
        return;
      }

      route->mOutgoingCheck = createBindRequest(route);
      mOutgoingChecks[route->mOutgoingCheck] = route;
    }

    //-----------------------------------------------------------------------
    void ICETransport::forceActive(RoutePtr route)
    {
      if (!route) return;
      if (route->mPrune) return;

      if (route->mNextKeepWarm) return;

      // install a temporary keep warm timer (to force route activate sooner)
      route->mNextKeepWarm = Timer::create(mThisWeak.lock(), zsLib::now() + Milliseconds(UseServicesHelper::random(0, static_cast<size_t>(mKeepWarmTimeRandomizedAddTime.count()))));
      mNextKeepWarmTimers[route->mNextKeepWarm] = route;

      ZS_LOG_TRACE(log("forcing route to generate activity") + route->toDebug())
    }

    //-----------------------------------------------------------------------
    void ICETransport::shutdown(RoutePtr route)
    {
      if (!route) return;

      ZS_LOG_TRACE(log("shutting down route") + route->toDebug())

      removeLegal(route);
      removeFoundation(route);
      removePendingActivation(route);
      removeFrozen(route);
      removeFrozenDependencies(route, false, ReasonNoMoreRelationshipPtr(new ReasonNoMoreRelationship));
      removeActive(route);
      removeWarm(route);
      removeGathererRoute(route);
      removeOutgoingCheck(route);
      removeKeepWarmTimer(route);

      route->mPrune = true;
      route->mKeepWarm = false;
    }

    //-------------------------------------------------------------------------
    void ICETransport::pruneAllCandidatePairs(bool keepActiveAlive)
    {
      ZS_LOG_DETAIL(log("pruning all candidate pairs"))

      if (mGatherer) {
        mGatherer->remoteAllRelatedRoutes(*this);
      }

      mPendingGathererRoutes.clear();

      mComputedPairsHash.clear();

      mFoundationRoutes.clear();
      mPendingActivation.clear();
      mFrozen.clear();
      mGathererRoutes.clear();
      mOutgoingChecks.clear();
      mNextKeepWarmTimers.clear();

      mLastReceivedUseCandidate = Time();
      mLastReceivedPacket = Time();

      RoutePtr currentRoute;

      for (auto iter_doNotUse = mLegalRoutes.begin(); iter_doNotUse != mLegalRoutes.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (keepActiveAlive) {
          if (mActiveRoute == route) {
            currentRoute = route;
            ZS_LOG_DEBUG(log("remembering previous active route (to keep it alive)") + route->toDebug())
          }
        }

        route->mGathererRouteID = 0;  // reset the route since it's gone

        ZS_LOG_DEBUG(log("shutting down route due to complete route pruning") + route->toDebug())
        shutdown(route);
      }

      mWarmRoutes.clear();
      warmRoutesChanged();

      if (currentRoute) {
        // reactivate this killed route
        mActiveRoute = currentRoute;
      }
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::CandidatePairPtr ICETransport::cloneCandidatePair(RoutePtr route) const
    {
      if (!route) return CandidatePairPtr();

      CandidatePairPtr tempPair(new CandidatePair);
      tempPair->mLocal = CandidatePtr(new Candidate(*(route->mCandidatePair->mLocal)));
      tempPair->mRemote = CandidatePtr(new Candidate(*(route->mCandidatePair->mRemote)));
      return tempPair;
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::setPending(RoutePtr route)
    {
      switch (route->mState) {
        case Route::State_Pending:    {
          auto found = mPendingActivation.find(route->mPendingPriority);
          if (found == mPendingActivation.end()) goto insert_pending;
          if ((*found).second == route) return;

          route->mPendingPriority = 0;
          goto insert_pending;
        }
        case Route::State_Frozen:     {
          ZS_LOG_DEBUG(log("route is now pending activation again") + route->toDebug())
          goto insert_pending;
        }
        case Route::State_InProgress: {
          ZS_LOG_WARNING(Detail, log("route is already in progress (no need to activate again)") + route->toDebug())
          return;
        }
        case Route::State_Succeeded: {
          ZS_LOG_DEBUG(log("route was succeeded but is being rechecked again") + route->toDebug())
          goto insert_pending;
        }
        case Route::State_Failed:    {
          ZS_LOG_DEBUG(log("route previously failed but is being rechecked again") + route->toDebug())
          goto insert_pending;
        }
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route cannot be reactivated because its blacklisted") + route->toDebug())
          return;
        }
      }

    insert_pending:
      {
        ZS_LOG_DEBUG(log("route is now being set to the pending state") + route->toDebug())

        removeFrozen(route);
        removeOutgoingCheck(route);
        removeKeepWarmTimer(route);
        removeWarm(route);
        route->mLastReceivedResponse = Time();  // need to recheck thus no response receieved

        auto priority = route->getActivationPriority(IICETypes::Role_Controlling == mOptions.mRole, mRemoteParameters.mUseCandidateFreezePriority);

        auto found = mPendingActivation.find(priority);
        while (found != mPendingActivation.end()) {
          auto comparePriority = (*found).first;
          if (priority != comparePriority) break;
          ++priority;
        }

        route->mPendingPriority = priority;
        mPendingActivation[priority] = route;

        route->mState = Route::State_Pending;
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setFrozen(
                                 RoutePtr route,
                                 PromisePtr promise
                                 )
    {
      switch (route->mState) {
        case Route::State_Pending:    {
          goto freeze;
        }
        case Route::State_Frozen:     {
          ZS_LOG_DEBUG(log("route will be frozen again") + route->toDebug())
          goto freeze;
        }
        case Route::State_InProgress: {
          ZS_LOG_WARNING(Detail, log("route is already in progress (no need to freeze)") + route->toDebug())
          return;
        }
        case Route::State_Succeeded: {
          ZS_LOG_DEBUG(log("route was succeeded (no need to freeze)") + route->toDebug())
          return;
        }
        case Route::State_Failed:    {
          ZS_LOG_DEBUG(log("route previously failed (no need to freeze)") + route->toDebug())
          return;
        }
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted") + route->toDebug())
          return;
        }
      }

    freeze:
      {
        ZS_LOG_DEBUG(log("route is now being frozen") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);

        route->mState = Route::State_Frozen;
        route->mFrozenPromise = promise;
        mFrozen[promise] = route;

        promise->thenWeak(mThisWeak.lock());
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setInProgress(RoutePtr route)
    {
      switch (route->mState) {
        case Route::State_Pending:    goto in_progress;
        case Route::State_Frozen:     goto in_progress;
        case Route::State_InProgress: {
          ZS_LOG_WARNING(Detail, log("route is already in progress (no need to activate again)") + route->toDebug())
          return;
        }
        case Route::State_Succeeded: {
          ZS_LOG_DEBUG(log("route was succeeded (but going to activate now again)") + route->toDebug())
          return;
        }
        case Route::State_Failed:    {
          ZS_LOG_DEBUG(log("route previously failed (but going to activate again)") + route->toDebug())
          return;
        }
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted") + route->toDebug())
          return;
        }
      }

    in_progress:
      {
        ZS_LOG_DEBUG(log("route is now being set in progess") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeOutgoingCheck(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->mState = Route::State_InProgress;
        route->mLastReceivedResponse = Time();

        if (Time() != route->mLastReceivedResponse) {
          route->mLastRoundTripCheck = zsLib::now();
        } else {
          route->mLastRoundTripCheck = Time();
        }

        route->mOutgoingCheck = createBindRequest(route);
        mOutgoingChecks[route->mOutgoingCheck] = route;
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setSucceeded(RoutePtr route)
    {
      switch (route->mState) {
        case Route::State_Pending:    goto succeeded;
        case Route::State_Frozen:     goto succeeded;
        case Route::State_InProgress: goto succeeded;
        case Route::State_Succeeded:  goto succeeded;
        case Route::State_Failed:     goto succeeded;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot succeed)") + route->toDebug())
          return;
        }
      }

    succeeded:
      {
        ZS_LOG_DEBUG(log("route is now successful") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, true);
        removeOutgoingCheck(route);

        route->mState = Route::State_Succeeded;

        // add to warm routes
        auto found = mWarmRoutes.find(route->mCandidatePairHash);
        if (found == mWarmRoutes.end()) {
          mWarmRoutes[route->mCandidatePairHash] = route;
          warmRoutesChanged();
          mSubscriptions.delegate()->onICETransportCandidatePairAvailable(mThisWeak.lock(), cloneCandidatePair(route));
        }
        wakeUp();
      }
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::setFailed(RoutePtr route)
    {
      switch (route->mState) {
        case Route::State_Pending:    goto failed;
        case Route::State_Frozen:     goto failed;
        case Route::State_InProgress: goto failed;
        case Route::State_Succeeded:  goto failed;
        case Route::State_Failed:     goto failed;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot fail)") + route->toDebug())
          return;
        }
      }

    failed:
      {
        ZS_LOG_DEBUG(log("route is now a failure") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, false);
        removeOutgoingCheck(route);
        removeGathererRoute(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->mState = Route::State_Failed;
        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setBlacklisted(RoutePtr route)
    {
      switch (route->mState) {
        case Route::State_Pending:    goto blacklist;
        case Route::State_Frozen:     goto blacklist;
        case Route::State_InProgress: goto blacklist;
        case Route::State_Succeeded:  goto blacklist;
        case Route::State_Failed:     goto blacklist;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot blacklist again)") + route->toDebug())
          return;
        }
      }

    blacklist:
      {
        ZS_LOG_DEBUG(log("route is now blacklisted") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, false);
        removeOutgoingCheck(route);
        removeGathererRoute(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->mKeepWarm = false;

        route->mState = Route::State_Blacklisted;
        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::updateAfterPacket(RoutePtr route)
    {
      if (route->isBlacklisted()) {
        ZS_LOG_WARNING(Trace, log("route cannot be made sucessful because it is blacklisted"))
        return;
      }

      if (!isContinousGathering()) {
        if (route->isFailed()) {
          ZS_LOG_WARNING(Trace, log("route cannot be made sucessful because it has already failed"))
          return;
        }
      }

      RoutePtr oldActiveRoute = mActiveRoute;

      if (!mActiveRoute) {
        if (Time() != route->mLastReceivedResponse) {
          ZS_LOG_DEBUG(log("setting route to active since received a response and no other route is available") + route->toDebug())
          mActiveRoute = route;
        } else if (Time() != route->mLastReceivedMedia) {
          ZS_LOG_DEBUG(log("setting route to active since incoming media was received on this route") + route->toDebug())
          mActiveRoute = route;
        }
      }

      auto latest = getLatest(route->mLastReceivedResponse, route->mLastReceivedCheck);
      if (Time() != latest) {
        auto now = zsLib::now();
        if (latest + mExpireRouteTime > now) {
          if (!route->isBlacklisted()) {
            ZS_LOG_TRACE(log("consent is granted") + route->toDebug())
            if (route->isSucceeded()) {
              if (mWarmRoutes.end() == mWarmRoutes.find(route->mCandidatePairHash)) {
                ZS_LOG_TRACE(log("need to keep candiate in the warm table since it's been woken up again"))
                setSucceeded(route);
              }
            } else {
              ZS_LOG_DEBUG(log("route is now a success") + route->toDebug())
              setSucceeded(route);
            }
          } else {
            ZS_LOG_WARNING(Trace, log("consent cannot be granted because route is blacklisted") + route->toDebug())
          }
        }
      }

      if (oldActiveRoute != mActiveRoute) {
        ZS_LOG_DETAIL(log("new route chosen") + mActiveRoute->toDebug())
        mSubscriptions.delegate()->onICETransportCandidatePairChanged(mThisWeak.lock(), cloneCandidatePair(mActiveRoute));

        wakeUp();
      }

      if (mUseCandidateRoute != mActiveRoute) {
        if (!mOptions.mAggressiveICE) {
          if (IICETypes::Role_Controlling == mOptions.mRole) {
            if (Time() != route->mLastReceivedResponse) {
              ZS_LOG_DEBUG(log("need to wake up and check if this route can be nominated"))
              wakeUp();
            }
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    bool ICETransport::installGathererRoute(RoutePtr route)
    {
      if (!route) {
        ZS_LOG_WARNING(Trace, log("not possible to install route as route is null"))
        return false;
      }
      if (!mGatherer) {
        ZS_LOG_WARNING(Trace, log("cannot install gatherer route as gatherer is not present") + route->toDebug())
        return false;
      }
      if (0 != route->mGathererRouteID) {
        ZS_LOG_INSANE(log("gatherer route already installed") + route->toDebug())
        return true;
      }
      auto ip = route->mCandidatePair->mRemote->ip();
      route->mGathererRouteID = mGatherer->createRoute(mThisWeak.lock(), route->mCandidatePair->mLocal, ip);
      if (0 == route->mGathererRouteID) {
        ZS_LOG_WARNING(Debug, log("failed to install gatherer route") + route->toDebug())
        return false;
      }
      mGathererRoutes[route->mGathererRouteID] = route;
      return true;
    }

    //-------------------------------------------------------------------------
    void ICETransport::installFoundation(RoutePtr route)
    {
      LocalRemoteFoundationPair foundation(route->mCandidatePair->mLocal->mFoundation, route->mCandidatePair->mRemote->mFoundation);

      auto found = mFoundationRoutes.find(foundation);

      if (found == mFoundationRoutes.end()) {
        RouteMap routes;
        routes[route->mCandidatePairHash] = route;

        mFoundationRoutes[foundation] = routes;
        return;
      }

      {
        RouteMap &routes = (*found).second;

        auto foundRoute = routes.find(route->mCandidatePairHash);
        if (foundRoute != routes.end()) return; // already installed

        routes[route->mCandidatePairHash] = route;
      }
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::removeLegal(RoutePtr route)
    {
      auto found = mLegalRoutes.find(route->mCandidatePairHash);
      if (found == mLegalRoutes.end()) return;

      mLegalRoutes.erase(found);
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeFoundation(RoutePtr route)
    {
      LocalRemoteFoundationPair foundation(route->mCandidatePair->mLocal->mFoundation, route->mCandidatePair->mRemote->mFoundation);

      auto found = mFoundationRoutes.find(foundation);

      if (found == mFoundationRoutes.end()) return;

      RouteMap &routes = (*found).second;

      auto foundRoute = routes.find(route->mCandidatePairHash);

      if (foundRoute == routes.end()) return;

      routes.erase(foundRoute);

      if (routes.size() > 0) return;

      mFoundationRoutes.erase(found);
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::removeFrozen(RoutePtr route)
    {
      if (!route->mFrozenPromise) return;

      auto found = mFrozen.find(route->mFrozenPromise);
      if (found == mFrozen.end()) return;

      mFrozen.erase(found);
      route->mFrozenPromise->resolve();
      route->mFrozenPromise.reset();
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeFrozenDependencies(
                                                RoutePtr route,
                                                bool succeeded,
                                                AnyPtr reason
                                                )
    {
      if (route->mDependentPromises.size() < 1) return;

      for (auto iter = route->mDependentPromises.begin(); iter != route->mDependentPromises.end(); ++iter)
      {
        auto promise = (*iter);
        if (succeeded) {
          promise->resolve(reason);
          continue;
        }
        promise->reject(reason);
        continue;
      }
      route->mDependentPromises.clear();
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeActive(RoutePtr route)
    {
      if (route == mActiveRoute) {
        mActiveRoute.reset();
        wakeUp();
      }
      if (route == mUseCandidateRoute) {
        if (mUseCandidateRequest) {
          mUseCandidateRequest->cancel();
          mUseCandidateRequest.reset();
        }
        mUseCandidateRoute.reset();
        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::removePendingActivation(RoutePtr route)
    {
      auto found = mPendingActivation.find(route->mPendingPriority);
      if (found != mPendingActivation.end()) {
        mPendingActivation.erase(found);
      }
      route->mPendingPriority = 0;
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeOutgoingCheck(RoutePtr route)
    {
      if (!route->mOutgoingCheck) return;

      auto found = mOutgoingChecks.find(route->mOutgoingCheck);
      if (found != mOutgoingChecks.end()) {
        mOutgoingChecks.erase(found);
      }

      route->mOutgoingCheck->cancel();
      route->mOutgoingCheck.reset();

      route->mLastRoundTripCheck = Time();
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeGathererRoute(RoutePtr route)
    {
      if (0 == route->mGathererRouteID) return;

      auto found = mGathererRoutes.find(route->mGathererRouteID);
      if (found != mGathererRoutes.end()) {
        mGathererRoutes.erase(found);
      }
      if (mGatherer) {
        mGatherer->removeRoute(route->mGathererRouteID);
      }

      route->mGathererRouteID = 0;
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::removeKeepWarmTimer(RoutePtr route)
    {
      if (!route->mNextKeepWarm) return;

      auto found = mNextKeepWarmTimers.find(route->mNextKeepWarm);

      if (found != mNextKeepWarmTimers.end()) {
        mNextKeepWarmTimers.erase(found);
      }

      route->mNextKeepWarm->cancel();
      route->mNextKeepWarm.reset();
    }

    //-------------------------------------------------------------------------
    void ICETransport::removeWarm(RoutePtr route)
    {
      // scope: remove from warm
      auto found = mWarmRoutes.find(route->mCandidatePairHash);
      if (found == mWarmRoutes.end()) return;

      auto pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onICETransportCandidatePairGone(mThisWeak.lock(), cloneCandidatePair(route));
      }
      mWarmRoutes.erase(found);
      warmRoutesChanged();
    }

    //-------------------------------------------------------------------------
    ICETransport::RoutePtr ICETransport::findRoute(
                                                   IICETypes::CandidatePtr localCandidate,
                                                   const IPAddress &inRemoteIP
                                                   )
    {
      if (!localCandidate) return RoutePtr();

      auto hash = localCandidate->hash();

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
        auto route = (*iter).second;

        auto localCandidateHash = route->mCandidatePair->mLocal->hash();
        if (localCandidateHash != hash) continue;

        auto remoteIP = route->mCandidatePair->mRemote->ip();

        if (inRemoteIP == remoteIP) return route;
      }
      return RoutePtr();
    }
    
    //-------------------------------------------------------------------------
    ICETransport::RoutePtr ICETransport::findClosestRoute(
                                                          IICETypes::CandidatePtr localCandidate,
                                                          const IPAddress &inRemoteIP,
                                                          DWORD remotePriority
                                                          )
    {
      if (!localCandidate) return RoutePtr();

      auto hash = localCandidate->hash();

      RoutePtr closeEnoughRoute;

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
        auto route = (*iter).second;

        auto localCandidateHash = route->mCandidatePair->mLocal->hash();
        if (localCandidateHash != hash) continue;

        auto remoteIP = route->mCandidatePair->mRemote->ip();

        if (inRemoteIP.isAddressEqualIgnoringIPv4Format(remoteIP)) {
          if (remotePriority == route->mCandidatePair->mRemote->mPriority) return route;
          if (0 == remotePriority) return route;
          closeEnoughRoute = route;
        }
      }
      return closeEnoughRoute;
    }

    //-------------------------------------------------------------------------
    Time ICETransport::getLastRemoteActivity(RoutePtr route) const
    {
      if (!route) return Time();

      auto lastest = getLatest(route->mLastReceivedCheck, route->mLastReceivedMedia);
      return getLatest(lastest, route->mLastReceivedResponse);
    }

    //-------------------------------------------------------------------------
    ISTUNRequesterPtr ICETransport::createBindRequest(
                                                      RoutePtr route,
                                                      bool useCandidate
                                                      ) const
    {
      ZS_THROW_BAD_STATE_IF(!mGatherer)

      if (IICETypes::Role_Controlling == mOptions.mRole) {
        useCandidate = mOptions.mAggressiveICE ? true : useCandidate; // force this flag if aggressive mode
      }

      STUNPacketPtr stunPacket = STUNPacket::createRequest(STUNPacket::Method_Binding);
      stunPacket->mPriorityIncluded = true;
      stunPacket->mPriority = route->mCandidatePair->mLocal->mPriority;
      setRole(stunPacket);
      if (useCandidate) {
        stunPacket->mUseCandidateIncluded = true;
      }

      stunPacket->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
      stunPacket->mUsername = mRemoteParameters.mUsernameFragment + ":" + mGatherer->getUsernameFrag();
      stunPacket->mPassword = mRemoteParameters.mPassword;

      fix(stunPacket);

      auto remoteIP = route->mCandidatePair->mRemote->ip();
      return ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), remoteIP, stunPacket, STUNPacket::RFC_5245_ICE);
    }

    //-------------------------------------------------------------------------
    STUNPacketPtr ICETransport::createBindResponse(
                                                   STUNPacketPtr request,
                                                   RoutePtr route
                                                   ) const
    {
      STUNPacketPtr stunPacket = STUNPacket::createResponse(request);

      setRole(stunPacket);
      stunPacket->mMappedAddress = route->mCandidatePair->mRemote->ip();

      if (mGatherer) {
        stunPacket->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
        stunPacket->mUsername = mRemoteParameters.mUsernameFragment + ":" + mGatherer->getUsernameFrag();
        stunPacket->mPassword = mRemoteParameters.mPassword;
      }

      fix(stunPacket);

      return stunPacket;
    }

    //-----------------------------------------------------------------------
    STUNPacketPtr ICETransport::createErrorResponse(
                                                    STUNPacketPtr request,
                                                    STUNPacket::ErrorCodes error
                                                    )
    {
      STUNPacketPtr response = STUNPacket::createErrorResponse(request);
      response->mErrorCode = error;
      fix(response);
      return response;
    }

    //-----------------------------------------------------------------------
    void ICETransport::setRole(STUNPacketPtr packet) const
    {
      if (IICETypes::Role_Controlling == mOptions.mRole) {
        packet->mIceControllingIncluded = true;
        packet->mIceControlling = mConflictResolver;
        return;
      }
      if (IICETypes::Role_Controlled == mOptions.mRole) {
        packet->mIceControlledIncluded = true;
        packet->mIceControlled = mConflictResolver;
        return;
      }
    }

    //-----------------------------------------------------------------------
    void ICETransport::fix(STUNPacketPtr stun) const
    {
      stun->mLogObject = "ortc::ICEGatherer";
      stun->mLogObjectID = mID;
    }

    //-------------------------------------------------------------------------
    void ICETransport::sendPacket(
                                  RouteID routeID,
                                  STUNPacketPtr packet
                                  )
    {
      if (!mGatherer) {
        ZS_LOG_WARNING(Detail, log("cannot respond as no gatherer is attached") + packet->toDebug())
        return;
      }

      SecureByteBlockPtr packetized = packet->packetize(STUNPacket::RFC_5245_ICE);
      mGatherer->sendPacket(routeID, packetized->BytePtr(), packetized->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    bool ICETransport::handleSwitchRolesAndConflict(
                                                    RouteID routeID,
                                                    STUNPacketPtr packet
                                                    )
    {
      if (packet->mIceControllingIncluded) {
        if (IICETypes::Role_Controlled == mOptions.mRole) {
          ZS_LOG_TRACE(log("no conflict in role detected"))
          return false;
        }

        if (mConflictResolver >= packet->mIceControlling) goto respond_with_conflict;
        goto switch_roles;
      }

      if (packet->mIceControlledIncluded) {
        if (IICETypes::Role_Controlling == mOptions.mRole) {
          ZS_LOG_TRACE(log("no conflict in role detected"))
          return false;
        }

        if (mConflictResolver >= packet->mIceControlled) goto switch_roles;
        goto respond_with_conflict;
      }

    respond_with_conflict:
      {
        auto foundPending = mPendingGathererRoutes.find(routeID);
        auto foundRoute = mGathererRoutes.find(routeID);

        auto response = createErrorResponse(packet, STUNPacket::ErrorCode_RoleConflict);
        setRole(response);
        if (foundPending != mPendingGathererRoutes.end()) {
          response->mMappedAddress = (*foundPending).second.second;
        } else if (foundRoute != mGathererRoutes.end()) {
          auto route = (*foundRoute).second;
          response->mMappedAddress = route->mCandidatePair->mRemote->ip();
        }

        ZS_LOG_WARNING(Detail, log("returning conflict error to remote party") + ZS_PARAMIZE(routeID) + response->toDebug())
        sendPacket(routeID, response);
        return true;
      }

    switch_roles:
      {
        auto foundPending = mPendingGathererRoutes.find(routeID);
        auto foundRoute = mGathererRoutes.find(routeID);

        CandidatePtr localCandidate;
        IPAddress remoteIP;

        if (foundPending != mPendingGathererRoutes.end()) {
          localCandidate = (*foundPending).second.first;
          remoteIP = (*foundPending).second.second;
        } else if (foundRoute != mGathererRoutes.end()) {
          auto route = (*foundRoute).second;
          localCandidate = route->mCandidatePair->mLocal;
          remoteIP = route->mCandidatePair->mRemote->ip();
        }

        ZS_LOG_WARNING(Detail, log("ice role conflict detected (switching roles)") + mOptions.toDebug() + packet->toDebug() + ZS_PARAM("conflict resolver", mConflictResolver))

        mOptions.mRole = (IICETypes::Role_Controlling == mOptions.mRole ? IICETypes::Role_Controlled : IICETypes::Role_Controlling);
        mOptionsHash = mOptions.hash();
        pruneAllCandidatePairs(true);
        wakeUp();

        if (localCandidate) {
          ZS_LOG_WARNING(Debug, log("will retry conflicting packet again after recomputing candidate pairings") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + packet->toDebug())
          ITransportAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyPacketRetried(localCandidate, remoteIP, packet);
        }
      }
      return true;
    }

    //-------------------------------------------------------------------------
    ICETransport::RoutePtr ICETransport::findOrCreateMissingRoute(
                                                                  RouteID routeID,
                                                                  STUNPacketPtr packet
                                                                  )
    {
      RoutePtr route;

      // scope: first check if route already exists
      {
        auto found = mGathererRoutes.find(routeID);

        if (found == mGathererRoutes.end()) goto existing_route_not_found;

        route = (*found).second;
        ZS_LOG_TRACE(log("found existing route to use") + route->toDebug())
        return route;
      }

    existing_route_not_found:
      {
        auto found = mPendingGathererRoutes.find(routeID);

        if (found == mPendingGathererRoutes.end()) {
          ZS_LOG_WARNING(Debug, log("route does not exist") + ZS_PARAMIZE(routeID))
          if (mGatherer) {
            mGatherer->removeRoute(routeID);
          }
          return RoutePtr();
        }

        // this route probably does not exist
        CandidatePtr localCandidate = (*found).second.first;
        IPAddress remoteIP((*found).second.second);
        mPendingGathererRoutes.erase(found);

        route = findRoute(localCandidate, remoteIP);

        if (route) {
          removeGathererRoute(route);         // remove the old route ID
          route->mGathererRouteID = routeID;  // now using the new route ID
          mGathererRoutes[routeID] = route;

          return route;
        }

        CandidatePtr remoteCandidate(new Candidate);
        remoteCandidate->mCandidateType = IICETypes::CandidateType_Prflx;
        remoteCandidate->mPriority = packet->mPriority;
        remoteCandidate->mIP = remoteIP.string(false);
        remoteCandidate->mPort = remoteIP.getPort();
        remoteCandidate->mProtocol = localCandidate->mProtocol;
        if (IICETypes::Protocol_TCP == localCandidate->mProtocol) {
          switch (localCandidate->mTCPType) {
            case IICETypes::TCPCandidateType_Active:  remoteCandidate->mTCPType = IICETypes::TCPCandidateType_Passive; break;
            case IICETypes::TCPCandidateType_Passive: remoteCandidate->mTCPType = IICETypes::TCPCandidateType_Active; break;
            case IICETypes::TCPCandidateType_SO:      remoteCandidate->mTCPType = IICETypes::TCPCandidateType_SO; break;
          }
        }

        RoutePtr closestRoute = findClosestRoute(localCandidate, remoteIP, packet->mPriority);
        if (closestRoute) {
          ZS_LOG_TRACE(log("found candidate close enough to borrow remote properties") + ZS_PARAM("new candidate", remoteCandidate->toDebug()) + ZS_PARAM("close candidate", closestRoute->mCandidatePair->mRemote->toDebug()))
          remoteCandidate->mUnfreezePriority = closestRoute->mCandidatePair->mRemote->mUnfreezePriority;
          remoteCandidate->mRelatedAddress = closestRoute->mCandidatePair->mRemote->mRelatedAddress;
          remoteCandidate->mRelatedPort = closestRoute->mCandidatePair->mRemote->mRelatedPort;
          remoteCandidate->mFoundation = closestRoute->mCandidatePair->mRemote->mFoundation;
        } else {
          remoteCandidate->mUnfreezePriority = packet->mPriority;
          remoteCandidate->mFoundation = remoteCandidate->foundation();

          ZS_LOG_WARNING(Debug, log("did not find close enough candidate to existing candidate (thus creating custom foundation)") + ZS_PARAM("new candidate", remoteCandidate->toDebug()))
        }

        // now have everything needed to create a brand new legal route
        route = RoutePtr(new Route);
        route->mCandidatePair = CandidatePairPtr(new CandidatePair);
        route->mCandidatePair->mLocal = localCandidate;
        route->mCandidatePair->mRemote = remoteCandidate;
        route->mCandidatePairHash = route->mCandidatePair->hash();

        // add as legal routes
        mLegalRoutes[route->mCandidatePairHash] = route;

        // add to gathering routes
        route->mGathererRouteID = routeID;
        mGathererRoutes[route->mGathererRouteID] = route;
        
        setPending(route);  // this is now a pending candidate
      }
      
      return route;
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::handlePassThroughSTUNPacket(
                                                   RouteID routeID,
                                                   STUNPacketPtr packet
                                                   )
    {
      {
        AutoRecursiveLock lock(*this);
        auto found = mGathererRoutes.find(routeID);
        if (found == mGathererRoutes.end()) {
          ZS_LOG_WARNING(Detail, log("route was not found") + ZS_PARAMIZE(routeID) + packet->toDebug())
          return;
        }

        auto route = (*found).second;

        if (route->isBlacklisted()) {
          ZS_LOG_WARNING(Debug, log("no consent given on this route") + route->toDebug() + packet->toDebug())
          return;
        }

        mLastReceivedPacket = zsLib::now();
        route->mLastReceivedMedia = mLastReceivedPacket;
        updateAfterPacket(route);

        if (!mActiveRoute) {
          ZS_LOG_WARNING(Debug, log("no active route chosen (thus must ignore packet)") + route->toDebug() + packet->toDebug())
          return;
        }

        goto forward_attached_listener;
      }

    forward_attached_listener:
      {
#define TODO_FORWARD_PACKET_TO_ATTACHED_LISTENER 1
#define TODO_FORWARD_PACKET_TO_ATTACHED_LISTENER 2
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport::Route
    #pragma mark

    //-------------------------------------------------------------------------
    const char *ICETransport::Route::toString(States state)
    {
      switch (state) {
        case State_Pending:     return "Pending";
        case State_Frozen:      return "Frozen";
        case State_InProgress:  return "In progress";
        case State_Succeeded:   return "Succeeded";
        case State_Failed:      return "Failed";
        case State_Blacklisted: return "Blacklisted";
      }
      return "Unknown";
    }

    //-------------------------------------------------------------------------
    ElementPtr ICETransport::Route::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICETransport::Route");

      UseServicesHelper::debugAppend(resultEl, mCandidatePair ? mCandidatePair->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "candidate pair hash", mCandidatePairHash);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mState));

      UseServicesHelper::debugAppend(resultEl, "gatherer route id", mGathererRouteID);

      UseServicesHelper::debugAppend(resultEl, "last received check", mLastReceivedCheck);
      UseServicesHelper::debugAppend(resultEl, "last sent check", mLastSentCheck);

      UseServicesHelper::debugAppend(resultEl, "last received media", mLastReceivedMedia);
      UseServicesHelper::debugAppend(resultEl, "last received response", mLastReceivedResponse);

      UseServicesHelper::debugAppend(resultEl, "prune", mPrune);
      UseServicesHelper::debugAppend(resultEl, "keep warm", mKeepWarm);
      UseServicesHelper::debugAppend(resultEl, "outgoing check", mOutgoingCheck ? mOutgoingCheck->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "keep warm timer", mNextKeepWarm ? mNextKeepWarm->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "last round trip check", mLastRoundTripCheck);
      UseServicesHelper::debugAppend(resultEl, "last round trip measurement", mLastRouteTripMeasurement);

      UseServicesHelper::debugAppend(resultEl, "frozen promise", (bool)mFrozenPromise);
      UseServicesHelper::debugAppend(resultEl, "dependent promises", mDependentPromises.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    QWORD ICETransport::Route::getPreference(bool localIsControlling) const
    {
      QWORD localPreference = mCandidatePair->mLocal->mPriority;
      QWORD remotePreference = mCandidatePair->mRemote->mPriority;

      QWORD g = (localIsControlling ? localPreference : remotePreference);
      QWORD d = (localIsControlling ? remotePreference : localPreference);

      QWORD priority = ((g < d ? g : d) << 32) + ((g > d ? g : d) << 1) + (g > d ? 1 : 0);
      return priority ^ getMaxQWORD();
    }

    //-------------------------------------------------------------------------
    QWORD ICETransport::Route::getActivationPriority(
                                                     bool localIsControlling,
                                                     bool useUnfreezePreference
                                                     ) const
    {
      QWORD localPreference = (useUnfreezePreference ? mCandidatePair->mLocal->mUnfreezePriority : mCandidatePair->mLocal->mPriority);
      QWORD remotePreference = (useUnfreezePreference ? mCandidatePair->mRemote->mUnfreezePriority : mCandidatePair->mRemote->mPriority);

      QWORD g = (localIsControlling ? localPreference : remotePreference);
      QWORD d = (localIsControlling ? remotePreference : localPreference);

      QWORD priority = ((g < d ? g : d) << 32) + ((g > d ? g : d) << 1) + (g > d ? 1 : 0);
      return priority ^ getMaxQWORD();
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
