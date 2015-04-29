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

      mDefaultSubscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);

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
      mWakeUp = false;
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
        IPAddress ip(route->mCandidatePair->mRemote->mIP, route->mCandidatePair->mRemote->mPort);
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

      if (!stepCalculateLegalPairs()) goto done;
      if (!stepPendingActivation()) goto done;
      if (!stepActivationTimer()) goto done;
      if (!stepLastReceivedPacketTimer()) goto done;
      if (!stepExpireRouteTimer()) goto done;
      if (!stepPickRoute()) goto done;
      if (!stepDewarmRoutes()) goto done;
      if (!stepKeepWarmRoutes()) goto done;

    done:
      {
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
    bool ICETransport::stepLastReceivedPacketTimer()
    {
      if (!mWarmRoutesChanged) {
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
    bool ICETransport::stepExpireRouteTimer()
    {
      if (!mWarmRoutesChanged) {
        ZS_LOG_TRACE(log("warm routes have not changed thus nothing to do"))
        return true;
      }

      if (mWarmRoutes.size() < 1) {
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
    bool ICETransport::stepPickRoute()
    {
      if (!mWarmRoutesChanged) {
        ZS_LOG_TRACE(log("warm routes have not changed"))
        return true;
      }

      if (mUseCandidateRequest) {
        ZS_LOG_TRACE(log("cannot switch routes until use candidate request completes"))
        return true;
      }

      if (Role_Controlled == mOptions.mRole) {
        ZS_LOG_TRACE(log("cannot choose route as in controlled role") + mOptions.toDebug())
        return true;
      }

      QWORD chosenPreference = 0;
      RoutePtr chosenRoute;

      for (auto iter = mWarmRoutes.begin(); iter != mWarmRoutes.end(); ++iter) {
        auto route = (*iter).second;

        auto preference = route->getPreference(Role_Controlling == mOptions.mRole);

        if (!chosenRoute) {
          chosenPreference = preference;
          chosenRoute = route;
          continue;
        }

        if (preference >= chosenPreference) continue;

        chosenRoute = route;
        chosenPreference = preference;
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

      mWarmRoutesChanged = false;
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
          route->mNextKeepWarm = Timer::create(mThisWeak.lock(), zsLib::now() + mKeepWarmTimeBase + Milliseconds(UseServicesHelper::random(0, mKeepWarmTimeRandomizedAddTime.count())));
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

    //-----------------------------------------------------------------------
    void ICETransport::shutdown(RoutePtr route)
    {
      if (!route) return;

      ZS_LOG_TRACE(log("shutting down route") + route->toDebug())

      removeLegal(route);
      removePendingActivation(route);
      removeFrozen(route);
      removeFrozenDependencies(route, false);
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

      mComputedPairsHash.clear();

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
      mWarmRoutesChanged = true;
      wakeUp();

      if (currentRoute) {
        // reactivate this killed route
        mActiveRoute = currentRoute;
      }
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
        removeGathererRoute(route);
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
        removeGathererRoute(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->mState = Route::State_InProgress;
        route->mLastReceivedResponse = Time();

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
          mWarmRoutesChanged = true;
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

      if ((Time() != route->mLastReceivedCheck) &&
          (Time() != route->mLastReceivedResponse)) {
        auto latest = route->mLastReceivedCheck > route->mLastReceivedResponse  ? route->mLastReceivedCheck : route->mLastReceivedResponse;

        auto now = zsLib::now();
        if (latest + mExpireRouteTime > now) {
          ZS_LOG_TRACE(log("consent is granted (if not blacklisted)") + route->toDebug())
          if (route->isSucceeded()) {
            if (mWarmRoutes.end() == mWarmRoutes.find(route->mCandidatePairHash)) {
              ZS_LOG_TRACE(log("need to keep candiate in the warm table since it's been woken up again"))
              setSucceeded(route);
            }
          } else {
            ZS_LOG_DEBUG(log("route is now a success") + route->toDebug())
            setSucceeded(route);
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
    void ICETransport::removeLegal(RoutePtr route)
    {
      auto found = mLegalRoutes.find(route->mCandidatePairHash);
      if (found == mLegalRoutes.end()) return;

      mLegalRoutes.erase(found);
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
                                                bool succeeded
                                                )
    {
      if (route->mDependentPromises.size() < 1) return;

      for (auto iter = route->mDependentPromises.begin(); iter != route->mDependentPromises.end(); ++iter)
      {
        auto promise = (*iter);
        if (succeeded) {
          promise->resolve();
          continue;
        }
        promise->reject();
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
      mWarmRoutesChanged = true;
      wakeUp();
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::CandidatePairPtr ICETransport::cloneCandidatePair(RoutePtr route)
    {
      if (!route) return CandidatePairPtr();

      CandidatePairPtr tempPair(new CandidatePair);
      tempPair->mLocal = CandidatePtr(new Candidate(*(route->mCandidatePair->mLocal)));
      tempPair->mRemote = CandidatePtr(new Candidate(*(route->mCandidatePair->mRemote)));
      return tempPair;
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
      if (IICETypes::Role_Controlling == mOptions.mRole) {
        stunPacket->mIceControllingIncluded = true;
        stunPacket->mIceControlling = mConflictResolver;
      } else {
        stunPacket->mIceControlledIncluded = true;
        stunPacket->mIceControlled = mConflictResolver;
      }
      if (useCandidate) {
        stunPacket->mUseCandidateIncluded = true;
      }

      stunPacket->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
      stunPacket->mUsername = mRemoteParameters.mUsernameFragment + ":" + mGatherer->getUsernameFrag();
      stunPacket->mPassword = mRemoteParameters.mPassword;

      fix(stunPacket);

      IPAddress remoteIP(route->mCandidatePair->mRemote->mIP, route->mCandidatePair->mRemote->mPort);

      return ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), remoteIP, stunPacket, STUNPacket::RFC_5245_ICE);
    }

    //-----------------------------------------------------------------------
    void ICETransport::fix(STUNPacketPtr stun) const
    {
      stun->mLogObject = "ortc::ICEGatherer";
      stun->mLogObjectID = mID;
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
