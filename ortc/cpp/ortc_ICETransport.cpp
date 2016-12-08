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
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/XML.h>

#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_icetransport) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);


  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(ICETransportSettingsDefaults);

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

      auto fastestThusFarCount = fastestThusFar.count();
      Microseconds comparison(fastestThusFarCount/4);

      return diff > comparison;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportSettingsDefaults
    #pragma mark

    class ICETransportSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~ICETransportSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static ICETransportSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<ICETransportSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static ICETransportSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<ICETransportSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_MAX_CANDIDATE_PAIRS_TO_TEST, 100);

        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_ACTIVATION_TIMER_IN_MILLISECONDS, 40);

        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_NO_PACKETS_RECEVIED_RECHECK_CANDIDATES_IN_SECONDS, 7);

        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_EXPIRE_ROUTE_IN_SECONDS, 60);

        ISettings::setBool(ORTC_SETTING_ICE_TRANSPORT_BLACKLIST_AFTER_CONSENT_REMOVAL, false);

        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_BASE_IN_MILLISECONDS, 4000);
        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_RANDOMIZED_ADD_TIME_IN_MILLISECONDS, 2000);

        ISettings::setBool(ORTC_SETTING_ICE_TRANSPORT_TEST_CANDIDATE_PAIRS_OF_LOWER_PREFERENCE, false);

        ISettings::setUInt(ORTC_SETTING_ICE_TRANSPORT_MAX_BUFFERED_FOR_SECURE_TRANSPORT, 5);
      }
      
    };

    //-------------------------------------------------------------------------
    void installICETransportSettingsDefaults()
    {
      ICETransportSettingsDefaults::singleton();
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
    #pragma mark IICETransportForSecureTransport
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
                               const make_private &,
                               IMessageQueuePtr queue,
                               IICETransportDelegatePtr originalDelegate,
                               UseICEGathererPtr gatherer
                               ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mGatherer(ICEGatherer::convert(gatherer)),
      mRouteStateTracker(make_shared<RouteStateTracker>(mID)),
      mNoPacketsReceivedRecheckTime(ISettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_NO_PACKETS_RECEVIED_RECHECK_CANDIDATES_IN_SECONDS)),
      mExpireRouteTime(ISettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_EXPIRE_ROUTE_IN_SECONDS)),
      mTestLowerPreferenceCandidatePairs(ISettings::getBool(ORTC_SETTING_ICE_TRANSPORT_TEST_CANDIDATE_PAIRS_OF_LOWER_PREFERENCE)),
      mBlacklistConsent(ISettings::getBool(ORTC_SETTING_ICE_TRANSPORT_BLACKLIST_AFTER_CONSENT_REMOVAL)),
      mKeepWarmTimeBase(ISettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_BASE_IN_MILLISECONDS)),
      mKeepWarmTimeRandomizedAddTime(ISettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_RANDOMIZED_ADD_TIME_IN_MILLISECONDS)),
      mMaxBufferedPackets(ISettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_MAX_BUFFERED_FOR_SECURE_TRANSPORT))
    {
      ZS_LOG_BASIC(debug("created"));

      if (mGatherer) {
        mGathererRouter = mGatherer->getGathererRouter();
        ZS_THROW_INVALID_ASSUMPTION_IF(!mGathererRouter)
      }

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }

      auto resolverBuffer = IHelper::random(sizeof(mConflictResolver));
      memcpy(&mConflictResolver, resolverBuffer->BytePtr(), resolverBuffer->SizeInBytes());

      ZS_EVENTING_11(
                     x, i, Detail, IceTransportCreate, ol, IceTransport, Start,
                     puid, id, mID,
                     puid, iceGathererId, ((bool)gatherer) ? gatherer->getID() : 0,
                     puid, iceGathererRouterId, ((bool)mGathererRouter) ? mGathererRouter->getID() : 0,
                     duration, noPacketsReceivedRecheckTime, mNoPacketsReceivedRecheckTime.count(),
                     duration, expireRouteTime, mExpireRouteTime.count(),
                     bool, testLowerPreferenceCandidatePairs, mTestLowerPreferenceCandidatePairs,
                     bool, blacklistConsent, mBlacklistConsent,
                     duration, keepWarmTimeBase, mKeepWarmTimeBase.count(),
                     duration, keepWarmTimeRandomizedAddTime, mKeepWarmTimeRandomizedAddTime.count(),
                     size_t, maxBufferedPackets, mMaxBufferedPackets,
                     QWORD, conflictResolver, mConflictResolver
                     );
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

          CandidatePtr candidate(make_shared<Candidate>(localCandidate));
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
      ZS_EVENTING_1(x, i, Detail, IceTransportDestroy, ol, IceTransport, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(IICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(IRTCPTransportPtr object)
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
    ICETransportPtr ICETransport::convert(ForSecureTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForDataTransportPtr object)
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
    IStatsProvider::PromiseWithStatsReportPtr ICETransport::getStats(const StatsTypeSet &stats) const
    {
      if ((!stats.hasStatType(IStatsReportTypes::StatsType_ICETransport)) &&
          (!stats.hasStatType(IStatsReportTypes::StatsType_LocalCandidate)) &&
          (!stats.hasStatType(IStatsReportTypes::StatsType_RemoteCandidate)) &&
          (!stats.hasStatType(IStatsReportTypes::StatsType_CandidatePair))) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }
      AutoRecursiveLock lock(*this);

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Debug, log("cannot collect stats while shutdown / shutting down"));
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      IICETransportAsyncDelegateProxy::create(mThisWeak.lock())->onResolveStatsPromise(promise);
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
        ORTC_THROW_INVALID_PARAMETERS_IF(IICEGatherer::State_Closed == gatherer->state())
      }

      ICETransportPtr pThis(make_shared<ICETransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, gatherer));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IICETransportSubscriptionPtr ICETransport::subscribe(IICETransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IICETransportSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IICETransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        ICETransportPtr pThis = mThisWeak.lock();

        if (IICETransportTypes::State_New != mCurrentState) {
          delegate->onICETransportStateChange(pThis, mCurrentState);
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

      CandidateListPtr result(make_shared<CandidateList>());
      for (auto iter = mRemoteCandidates.begin(); iter != mRemoteCandidates.end(); ++iter) {
        auto tempCandidate = (*iter).second;
        result->push_back(*tempCandidate);
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::CandidatePairPtr ICETransport::getSelectedCandidatePair() const
    {
      AutoRecursiveLock lock(*this);

      if (!mActiveRoute) return CandidatePairPtr();

      CandidatePairPtr result(make_shared<CandidatePair>());

      result->mLocal = make_shared<Candidate>(*(mActiveRoute->mCandidatePair->mLocal));
      result->mRemote = make_shared<Candidate>(*(mActiveRoute->mCandidatePair->mRemote));

      return result;
    }

    //-------------------------------------------------------------------------
    void ICETransport::start(
                             IICEGathererPtr inGatherer,
                             const Parameters &remoteParameters,
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

      bool hadRemoteUsernameFragment = mRemoteParameters.mUsernameFragment.hasData();

      mRemoteParametersHash = remoteParameters.hash();
      mRemoteParameters = remoteParameters;

      ZS_EVENTING_9(
                    x, i, Detail, IceTransportStart, ol, IceTransport, Start,
                    puid, id, mID,
                    puid, iceGathererId, ((bool)inGatherer) ? inGatherer->getID() : 0,
                    bool, remoteUseUnfreezePriority, remoteParameters.mUseUnfreezePriority,
                    string, remoteUsernameFragement, remoteParameters.mUsernameFragment,
                    string, remotePassword, remoteParameters.mPassword,
                    bool, remoteIceList, remoteParameters.mICELite,
                    string, remoteParametersHash, mRemoteParametersHash,
                    string, oldRemoteParametersHash, oldParamHash,
                    bool, hadRemoteUsernameFragment, hadRemoteUsernameFragment
                    );

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
        mGathererRouter = gatherer->getGathererRouter();
        ZS_THROW_INVALID_ASSUMPTION_IF(!mGathererRouter)

        mGatherer->installTransport(mThisWeak.lock(), String());
        mGathererSubscription = mGatherer->subscribe(mThisWeak.lock());

        auto localCandidates = mGatherer->getLocalCandidates();
        for (auto iter = localCandidates->begin(); iter != localCandidates->end(); ++iter) {
          auto localCandidate = (*iter);

          CandidatePtr candidate(make_shared<Candidate>(localCandidate));
          mLocalCandidates[candidate->hash()] = candidate;
        }
        mLocalCandidatesComplete = false;
        mLocalCandidatesHash.clear();

        pruneAllCandidatePairs(true);
      } else {
        if (!hadRemoteUsernameFragment) {
          mGatherer->installTransport(mThisWeak.lock(), mRemoteParameters.mUsernameFragment);
          mNextActivationCausesAllRoutesThatReceivedChecksToActivate = true;
        } else {
          pruneAllCandidatePairs(true);
        }
      }

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

      ParametersPtr parameters(make_shared<Parameters>());
      (*parameters) = mRemoteParameters;

      ZS_LOG_TRACE(log("obtained remote parameters") + parameters->toDebug())
      return parameters;
    }

    //-------------------------------------------------------------------------
    IICETransportPtr ICETransport::createAssociatedTransport(IICETransportDelegatePtr delegate) throw (InvalidStateError)
    {
      ICETransportPtr pThis;
      UseSecureTransportPtr secureTransport;

      {
        AutoRecursiveLock lock(*this);

        UseICEGathererPtr rtcpGatherer;

        ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown())
        ORTC_THROW_INVALID_STATE_IF(IICETypes::Component_RTCP == mComponent)

        if (mGatherer) {
          rtcpGatherer = mGatherer->getRTCPGatherer();
          ORTC_THROW_INVALID_PARAMETERS_IF(Component_RTCP != rtcpGatherer->component())
          ORTC_THROW_INVALID_PARAMETERS_IF(IICEGatherer::State_Closed == rtcpGatherer->state())
        }

        pThis = make_shared<ICETransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, rtcpGatherer);
        pThis->mThisWeak.lock();
        pThis->mRTPTransport = mThisWeak.lock();
        mRTCPTransport = pThis;

        secureTransport = mSecureTransport.lock();

        ZS_EVENTING_2(
                      x, i, Detail, IceTransportCreateAssociatedTransport, ol, IceTransport, CreateAssociated,
                      puid, id, mID,
                      puid, associatedGathererId, pThis->mID
                      );
      }

      pThis->init();

      if (secureTransport) {
        secureTransport->notifyAssociateTransportCreated(IICETypes::Component_RTCP, pThis);
      }

      return pThis;
    }

    //-------------------------------------------------------------------------
    void ICETransport::addRemoteCandidate(const GatherCandidate &remoteCandidate) throw (InvalidStateError, InvalidParameters)
    {
      AutoRecursiveLock lock(*this);

      ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown());

      {
        const Candidate *tempCandidate = dynamic_cast<const IICETypes::Candidate *>(&remoteCandidate);
        if (tempCandidate) {
          auto hash = tempCandidate->hash();
          auto found = mRemoteCandidates.find(hash);

          ORTC_THROW_INVALID_PARAMETERS_IF(mComponent != tempCandidate->mComponent);

          ZS_EVENTING_15(
                         x, i, Detail, IceTransportAddRemoteCandidate, ol, IceTransport, AddCandidate,
                         puid, id, mID,
                         string, hash, hash,
                         string, interfaceType, tempCandidate->mInterfaceType,
                         string, foundation, tempCandidate->mFoundation,
                         enum, component, tempCandidate->mComponent,
                         dword, priority, tempCandidate->mPriority,
                         dword, unfreezePriority, tempCandidate->mUnfreezePriority,
                         string, protocol, IICETypes::toString(tempCandidate->mProtocol),
                         string, ip, tempCandidate->mIP,
                         word, port, tempCandidate->mPort,
                         string, candidateType, IICETypes::toString(tempCandidate->mCandidateType),
                         string, tcpType, IICETypes::toString(tempCandidate->mTCPType),
                         string, relatedAddress, tempCandidate->mRelatedAddress,
                         word, relatedPort, tempCandidate->mRelatedPort,
                         bool, previouslyAddress, found != mRemoteCandidates.end()
                         );

          if (found != mRemoteCandidates.end()) {
            if (mRemoteCandidatesComplete) {
              ZS_LOG_WARNING(Detail, log("told to add candidate that is already added (but no longer at end of remote candidates)") + tempCandidate->toDebug())
              mRemoteCandidatesComplete = false;
              goto changed;
            }
            ZS_LOG_WARNING(Detail, log("told to add candidate that is already added") + tempCandidate->toDebug())
            return;
          }

          ZS_LOG_DEBUG(log("adding remote candidate") + tempCandidate->toDebug())

          CandidatePtr candidate(make_shared<Candidate>(*tempCandidate));
          mRemoteCandidates[hash] = candidate;
          goto changed;
        }

        const CandidateComplete *tempCandidateComplete = dynamic_cast<const IICETypes::CandidateComplete *>(&remoteCandidate);
        if (tempCandidateComplete) {
          ZS_EVENTING_2(
                        x, i, Detail, IceTransportAddRemoteCandidateComplete, ol, IceTransport, AddCandidate,
                        puid, id, mID,
                        enum, component, tempCandidateComplete->mComponent
                        );

          ORTC_THROW_INVALID_PARAMETERS_IF(tempCandidateComplete->mComponent != mComponent);
          if (!mRemoteCandidatesComplete) {
            ZS_LOG_DEBUG(log("end of remote candidates indicated"))
            mRemoteCandidatesComplete = true;
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
    void ICETransport::setRemoteCandidates(const CandidateList &remoteCandidates) throw (InvalidStateError, InvalidParameters)
    {
      AutoRecursiveLock lock(*this);

      ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown())

      bool changed = mRemoteCandidatesComplete;  // if was at end of remote candidates then it must have changed

      mRemoteCandidatesComplete = false;

      CandidateMap foundCandidates;

      for (auto iter = remoteCandidates.begin(); iter != remoteCandidates.end(); ++iter) {
        auto tempCandidate = (*iter);

        CandidatePtr candidate(make_shared<Candidate>(tempCandidate));
        auto hash = candidate->hash();

        ORTC_THROW_INVALID_PARAMETERS_IF(mComponent != candidate->mComponent);

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

        ZS_EVENTING_15(
                       x, i, Detail, IceTransportRemoveRemoteCandidate, ol, IceTransport, RemoveCandidate,
                       puid, id, mID,
                       string, hash, hash,
                       string, interfaceType, candidate->mInterfaceType,
                       string, foundation, candidate->mFoundation,
                       enum, component, candidate->mComponent,
                       dword, priority, candidate->mPriority,
                       dword, unfreezePriority, candidate->mUnfreezePriority,
                       string, protocol, IICETypes::toString(candidate->mProtocol),
                       string, ip, candidate->mIP,
                       word, port, candidate->mPort,
                       string, candidateType, IICETypes::toString(candidate->mCandidateType),
                       string, tcpType, IICETypes::toString(candidate->mTCPType),
                       string, relatedAddress, candidate->mRelatedAddress,
                       word, relatedPort, candidate->mRelatedPort,
                       bool, previouslyAddress, found != mRemoteCandidates.end()
                       );

        mRemoteCandidates.erase(current);
        changed = true;
      }

      for (auto iter = foundCandidates.begin(); iter != foundCandidates.end(); ++iter) {
        auto hash = (*iter).first;
        auto candidate = (*iter).second;

        auto found = mRemoteCandidates.find(hash);

        ZS_EVENTING_15(
                       x, i, Detail, IceTransportAddRemoteCandidate, ol, IceTransport, AddCandidate,
                       puid, id, mID,
                       string, hash, hash,
                       string, interfaceType, candidate->mInterfaceType,
                       string, foundation, candidate->mFoundation,
                       enum, component, candidate->mComponent,
                       dword, priority, candidate->mPriority,
                       dword, unfreezePriority, candidate->mUnfreezePriority,
                       string, protocol, IICETypes::toString(candidate->mProtocol),
                       string, ip, candidate->mIP,
                       word, port, candidate->mPort,
                       string, candidateType, IICETypes::toString(candidate->mCandidateType),
                       string, tcpType, IICETypes::toString(candidate->mTCPType),
                       string, relatedAddress, candidate->mRelatedAddress,
                       word, relatedPort, candidate->mRelatedPort,
                       bool, previouslyAddress, found != mRemoteCandidates.end()
                       );

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
    void ICETransport::removeRemoteCandidate(const GatherCandidate &remoteCandidate) throw (InvalidStateError, InvalidParameters)
    {
      AutoRecursiveLock lock(*this);

      ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown())

      {
        const Candidate *tempCandidate = dynamic_cast<const IICETypes::Candidate *>(&remoteCandidate);
        if (tempCandidate) {
          auto hash = tempCandidate->hash();
          auto found = mRemoteCandidates.find(hash);

          ORTC_THROW_INVALID_PARAMETERS_IF(tempCandidate->mComponent != mComponent);

          ZS_EVENTING_15(
                         x, i, Detail, IceTransportRemoveRemoteCandidate, ol, IceTransport, RemoveCandidate,
                         puid, id, mID,
                         string, hash, hash,
                         string, interfaceType, tempCandidate->mInterfaceType,
                         string, foundation, tempCandidate->mFoundation,
                         enum, component, tempCandidate->mComponent,
                         dword, priority, tempCandidate->mPriority,
                         dword, unfreezePriority, tempCandidate->mUnfreezePriority,
                         string, protocol, IICETypes::toString(tempCandidate->mProtocol),
                         string, ip, tempCandidate->mIP,
                         word, port, tempCandidate->mPort,
                         string, candidateType, IICETypes::toString(tempCandidate->mCandidateType),
                         string, tcpType, IICETypes::toString(tempCandidate->mTCPType),
                         string, relatedAddress, tempCandidate->mRelatedAddress,
                         word, relatedPort, tempCandidate->mRelatedPort,
                         bool, previouslyAddress, found != mRemoteCandidates.end()
                         );

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
          ZS_EVENTING_2(
                        x, i, Detail, IceTransportRemoveRemoteCandidateComplete, ol, IceTransport, RemoveCandidate,
                        puid, id, mID,
                        enum, component, tempCandidateComplete->mComponent
                        );

          ORTC_THROW_INVALID_PARAMETERS_IF(tempCandidateComplete->mComponent != mComponent);
          if (mRemoteCandidatesComplete) {
            ZS_LOG_DEBUG(log("end of remote candidates no longer indicated"))
            mRemoteCandidatesComplete = false;
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
                                ) throw (InvalidStateError)
    {
      AutoRecursiveLock lock(*this);

      ORTC_THROW_INVALID_STATE_IF(isShuttingDown() || isShutdown())

      auto hash = candidatePair.hash();

      auto found = mLegalRoutes.find(hash);
      if (found == mLegalRoutes.end()) {
        ZS_EVENTING_4(
                      x, i, Detail, IceTransportKeepWarm, ol, IceTransport, Warm,
                      puid, id, mID,
                      string, reason, "not found",
                      string, candidatePairHash, hash,
                      bool, keepWarm, keepWarm
                      );

        ZS_LOG_DETAIL(log("did not find any route to keep warm") + candidatePair.toDebug() + ZS_PARAMIZE(hash))
        return;
      }

      auto route = (*found).second;

      if (route->isBlacklisted()) {
        ZS_EVENTING_4(
                      x, i, Detail, IceTransportKeepWarm, ol, IceTransport, Warm,
                      puid, id, mID,
                      string, reason, "blacklisted",
                      string, candidatePairHash, hash,
                      bool, keepWarm, keepWarm
                      );

        route->trace(__func__, "keepp warm blacklisted");
        ZS_LOG_WARNING(Detail, log("cannot keep warm as route is blacklisted") + route->toDebug())
        return;
      }

      ZS_EVENTING_4(
                    x, i, Detail, IceTransportKeepWarm, ol, IceTransport, Warm,
                    puid, id, mID,
                    string, reason, "found",
                    string, candidatePairHash, hash,
                    bool, keepWarm, keepWarm
                    );

      route->trace(__func__, "keep warm");

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
    ICETransport::ForICEGathererPtr ICETransport::getForGatherer() const
    {
      return mThisWeak.lock();
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    RouterRoutePtr routerRoute,
                                    STUNPacketPtr packet
                                    )
    {
      ZS_EVENTING_2(
                    x, i, Trace, IceTransportReceivedStunPacketFromGatherer, ol, IceTransport, Receive,
                    puid, id, mID,
                    puid, routerRouteId, routerRoute->mID
                    );
      routerRoute->trace(__func__, "received stun packet");
      packet->trace(__func__);

      {
        AutoRecursiveLock lock(*this);

        if (!mGatherer) {
          ZS_LOG_WARNING(Debug, log("notify packet but no gatherer is attached") + routerRoute->toDebug() + packet->toDebug())
          return;
        }

        if (STUNPacket::Method_Binding != packet->mMethod) goto not_related_stun_packet;
        if (STUNPacket::Class_Request != packet->mClass) {
          if (STUNPacket::Class_Response == packet->mClass) {
            if (packet->mUsername.hasData()) {
              // Some ICE implementation incorrectly send username in ICE
              // binding response. Remember if a response comes back from
              // the remote party with this incorrect behaviour and mimic
              /// it in STUN request packets sent out.
              mSTUNPacketOptions.mBindResponseRequiresUsernameAttribute = true;
            }
          }
          goto not_related_stun_packet;
        }

        if (handleSwitchRolesAndConflict(routerRoute, packet)) {
          ZS_LOG_DEBUG(log("role conflict handled") + packet->toDebug())
          return;
        }

        auto route = findOrCreateMissingRoute(routerRoute, packet);

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

              ZS_LOG_WARNING(Detail, log("returning unauthorized error to remote party (as route is blacklisted)") + routerRoute->toDebug() + response->toDebug())
              sendPacket(routerRoute, response);

              setBlacklisted(route);
              route->trace(__func__, "blacklist response to stun binding request");
              response->trace(__func__);
              return;
            }
          }
        }

        if (route->isBlacklisted()) {
          ZS_LOG_DETAIL(log("consent on candidate has already been removed (blacklisted)") + route->toDebug())

          auto response = createErrorResponse(packet, STUNPacket::ErrorCode_Unauthorized);

          ZS_LOG_WARNING(Debug, log("returning conflict error to remote party") + routerRoute->toDebug() + response->toDebug())
          sendPacket(routerRoute, response);
          route->trace(__func__, "blacklist response to stun binding request");
          response->trace(__func__);
          return;
        }

        route->mLastReceivedCheck = mLastReceivedPacket;
        if (Time() == route->mLastReceivedResponse) {
          if (route->mOutgoingCheck) {
            ZS_LOG_DEBUG(log("forcing a trigger check response immediately") + route->toDebug())
            route->trace(__func__, "activate trigger check");
            route->mOutgoingCheck->retryRequestNow();
          }
        }

        if ((route->isNew()) ||
            (route->isFrozen()) ||
            (route->isPending()) ||
            (route->isFailed()) ||
            (route->isIgnored())) {
          if (mRemoteParameters.mUsernameFragment.hasData()) {
            ZS_LOG_DETAIL(log("going to activate candidate pair because of incoming request") + route->toDebug())
            route->trace(__func__, "activate route (due to incoming request)");
            setInProgress(route);
          } else {
            mNextActivationCausesAllRoutesThatReceivedChecksToActivate = true;
          }
        }

        auto response = createBindResponse(packet, route);

        route->trace(__func__, "create response to stun binding request");
        response->trace(__func__);

        ZS_LOG_TRACE(log("sending binding response to remote party") + route->toDebug() + response->toDebug())
        sendPacket(routerRoute, response);

        if (IICETypes::Role_Controlled == mOptions.mRole) {
          if (packet->mUseCandidateIncluded) {
            auto previousRoute = mActiveRoute;
            const char *reason = NULL;

            if (!mActiveRoute) {
              mActiveRoute = route;
              reason = "activating route from incoming stun request (as no route was active)";
            }

            if (mOptions.mAggressiveICE) {
              if (Time() == mLastReceivedUseCandidate) {
                mActiveRoute = route;
                reason = "aggressive ice is activating route from incoming stun request (first time received use candidate)";
              } else if (mLastReceivedUseCandidate + Seconds(3) < mLastReceivedPacket) {
                ZS_LOG_DEBUG(log("enough time has passed from last receive candidate to this candidate to force it active (aggressive 3 second rule)") + ZS_PARAMIZE(mLastReceivedUseCandidate) + ZS_PARAMIZE(mLastReceivedPacket) + route->toDebug())
                mActiveRoute = route;
                reason = "activating route from incoming stun request (as enough time has passed since last aggressive use candidate to change)";
              } else {
                if (mActiveRoute->getPreference(false) < route->getPreference(false)) {
                  ZS_LOG_DEBUG(log("new route is preferred over active route (aggressive most preferred route rule)") + ZS_PARAM("new route", route->toDebug()) + ZS_PARAM("active route", mActiveRoute->toDebug()))
                  mActiveRoute = route;
                  reason = "activating route from incoming stun request (as new route is preferred to older route)";
                }
              }
            } else {
              mActiveRoute = route;
              reason = "activating route from incoming stun request (as route was chosen by controlling party)";
            }

            mLastReceivedUseCandidate = mLastReceivedPacket;

            if (previousRoute != mActiveRoute) {
              mActiveRoute->trace(__func__, reason);
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
        handlePassThroughSTUNPacket(routerRoute, packet);
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    RouterRoutePtr routerRoute,
                                    const BYTE *buffer,
                                    size_t bufferSizeInBytes
                                    )
    {
      ZS_EVENTING_4(
                    x, i, Trace, IceTransportReceivedPacketFromGatherer, ol, IceTransport, Receive,
                    puid, id, mID,
                    puid, routerRouteId, routerRoute->mID,
                    buffer, packet, buffer,
                    size, size, bufferSizeInBytes
                    );

      UseSecureTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);

        auto found = mGathererRoutes.find(routerRoute->mID);
        if (found == mGathererRoutes.end()) {
          ZS_LOG_WARNING(Detail, log("route was not found") + routerRoute->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
          return;
        }

        auto route = (*found).second;

        if (route->isBlacklisted()) {
          ZS_LOG_WARNING(Debug, log("no consent given on this route") + route->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
          route->trace(__func__, "ignoring incoming packet (route was blacklisted)");
          return;
        }

        mLastReceivedPacket = zsLib::now();
        route->mLastReceivedMedia = mLastReceivedPacket;
        updateAfterPacket(route);

        if (!mActiveRoute) {
          ZS_LOG_WARNING(Debug, log("no active route chosen (thus must ignore packet)") + route->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
          route->trace(__func__, "ignoring incoming packet (no route was chosen)");
          return;
        }

        transport = mSecureTransport.lock();
        if (!transport) {
          ZS_LOG_WARNING(Debug, log("no secure transport attached (packet is being buffered)"))
          mMustBufferPackets = true;
        }

        if (mMustBufferPackets) {
          // packets must be buffered
          ZS_LOG_TRACE(log("buffering packet for secure transport") + ZS_PARAM("buffer length", bufferSizeInBytes));
          ZS_EVENTING_4(
                        x, i, Trace, IceTransportBufferingIncomingPacket, ol, IceTransport, Buffer,
                        puid, id, mID,
                        puid, routerRouteId, routerRoute->mID,
                        buffer, packet, buffer,
                        size, size, bufferSizeInBytes
                        );

          mBufferedPackets.push(make_shared<SecureByteBlock>(buffer, bufferSizeInBytes));
          while (mBufferedPackets.size() > mMaxBufferedPackets) {
            auto &poppedBuffer = mBufferedPackets.front();
            ZS_EVENTING_4(
                          x, i, Debug, IceTransportDisposingBufferedIncomingPacket, ol, IceTransport, Dispose,
                          puid, id, mID,
                          puid, routerRouteId, routerRoute->mID,
                          buffer, packet, poppedBuffer->BytePtr(),
                          size, size, poppedBuffer->SizeInBytes()
                          );

            ZS_LOG_TRACE(log("too many packets in buffered packet list (dropping packet") + ZS_PARAM("max packets", mMaxBufferedPackets) + ZS_PARAM("total packets", mBufferedPackets.size()));
            mBufferedPackets.pop();
          }
          return;
        }

        goto forward_attached_secure_transport;
      }

    forward_attached_secure_transport:
      {
        ZS_EVENTING_4(
                      x, i, Trace, IceTransportDeliveringIncomingPacketToSecureTransport, ol, IceTransport, Deliver,
                      puid, id, mID,
                      puid, secureTransportId, transport->getID(),
                      buffer, packet, buffer,
                      size, size, bufferSizeInBytes
                      );

        bool handled = transport->handleReceivedPacket(mComponent, buffer, bufferSizeInBytes);

        if (!handled) goto forward_old_transport;
        return;
      }

    forward_old_transport:
      {
        {
          AutoRecursiveLock lock(*this);
          transport = mSecureTransportOld.lock();
          if (!transport) {
            ZS_LOG_WARNING(Debug, log("no older transport available to send packet") + routerRoute->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
            return;
          }
        }

        ZS_EVENTING_4(
                      x, i, Trace, IceTransportDeliveringIncomingPacketToSecureTransport, ol, IceTransport, Deliver,
                      puid, id, mID,
                      puid, secureTransportId, transport->getID(),
                      buffer, packet, buffer,
                      size, size, bufferSizeInBytes
                      );
        bool handled = transport->handleReceivedPacket(mComponent, buffer, bufferSizeInBytes);
        if (!handled) {
          AutoRecursiveLock lock(*this);

          auto oldTransportID = transport->getID();

          mSecureTransportOld.reset();
          ZS_LOG_DEBUG(log("old transport did not handle packet either (thus disposing of old transport)") + ZS_PARAM("old transport id", oldTransportID) + routerRoute->toDebug() + ZS_PARAMIZE(bufferSizeInBytes))
          return;
        }
      }
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::needsMoreCandidates() const
    {
      bool needMoreCandidates = mNeedsMoreCandidates;
      ZS_LOG_TRACE(log("needs more candidates") + ZS_PARAM("need", needMoreCandidates))
      return needMoreCandidates;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForICETransportContoller
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::notifyControllerAttached(ICETransportControllerPtr inController)
    {
      UseICETransportControllerPtr controller = inController;

      ZS_EVENTING_2(
                    x, i, Detail, IceTransportInternalControllerAttachedEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, iceTransportControllerId, controller->getID()
                    );

      AutoRecursiveLock lock(*this);

      mTransportController = controller;
      wakeUp();
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyControllerDetached(ICETransportController &inController)
    {
      UseICETransportController &controller = inController;

      AutoRecursiveLock lock(*this);

      ZS_THROW_INVALID_ASSUMPTION_IF(IICETypes::Component_RTCP == mComponent)

      UseICETransportControllerPtr currentController = mTransportController.lock();

      if (currentController) {
        if (currentController->getID() != controller.getID()) {
          ZS_LOG_WARNING(Trace, log("notified about controller detachment for obsolete ice transport controller"))
          return;
        }
      }

      ZS_EVENTING_2(
                    x, i, Detail, IceTransportInternalControllerDetachedEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, iceTransportControllerId, controller.getID()
                    );

      mTransportController.reset();

      for (auto iter_doNotUse = mLegalRoutes.begin(); iter_doNotUse != mLegalRoutes.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        removeFrozenDependencies(route, false, make_shared<ReasonNoMoreRelationship>());

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

        ZS_EVENTING_3(
                      x, i, Detail, IceTransportInstallFoundationDependencyFreezePromise, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      string, localFoundation, localFoundation,
                      string, remoteFoundation, remoteFoundation
                      );
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

      ZS_EVENTING_3(
                    x, i, Detail, IceTransportInstallFoundationDependencyFreezePromise, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    string, localFoundation, localFoundation,
                    string, remoteFoundation, remoteFoundation
                    );

      ZS_LOG_DEBUG(log("installed dependency race promises") + ZS_PARAM("total", promises.size()) + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::notifyAttached(
                                      PUID secureTransportID,
                                      UseSecureTransportPtr transport
                                      )
    {
      ZS_EVENTING_2(
                    x, i, Detail, IceTransportInternalSecureTransportAttachedEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, secureTransportId, secureTransportID
                    );

      ZS_LOG_DETAIL(log("notify attached") + ZS_PARAM("secure transport id", secureTransportID))

      AutoRecursiveLock lock(*this);

      if (isShutdown()) {
        ZS_LOG_WARNING(Detail, log("cannot attach secure transport while shutdown"))
        return;
      }

      mSecureTransportID = secureTransportID;
      mSecureTransport = transport;

      IICETransportAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyAttached(secureTransportID);
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyDetached(PUID secureTransportID)
    {
      ZS_EVENTING_2(
                    x, i, Detail, IceTransportInternalSecureTransportDetachedEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, secureTransportId, secureTransportID
                    );

      IICETransportAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyDetached(secureTransportID);
    }

    //-------------------------------------------------------------------------
    IICETypes::Roles ICETransport::getRole() const
    {
      AutoRecursiveLock lock(*this);
      return mOptions.mRole;
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::getRTPTransport() const
    {
      return mRTPTransport.lock();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::getRTCPTransport() const
    {
      return mRTCPTransport;
    }

    //-------------------------------------------------------------------------
    ICETransport::UseSecureTransportPtr ICETransport::getSecureTransport() const
    {
      AutoRecursiveLock lock(*this);
      return mSecureTransport.lock();
    }

    //-------------------------------------------------------------------------
    bool ICETransport::sendPacket(
                                  const BYTE *buffer,
                                  size_t bufferSizeInBytes
                                  )
    {
      ZS_EVENTING_3(
                    x, i, Trace, IceTransportSecureTransportSendPacket, ol, IceTransport, Send,
                    puid, id, mID,
                    buffer, packet, buffer,
                    size, size, bufferSizeInBytes
                    );

      UseICEGathererPtr gatherer;
      RouterRoutePtr routerRoute;

      {
        AutoRecursiveLock lock(*this);

        if (!installGathererRoute(mActiveRoute)) {
          ZS_LOG_WARNING(Trace, log("cannot install a gatherer route") + (mActiveRoute ? mActiveRoute->toDebug() : ElementPtr()) + ZS_PARAM("buffer size", bufferSizeInBytes))
          return false;
        }

        gatherer = mGatherer;
        routerRoute = mActiveRoute->mGathererRoute;
      }

      ZS_EVENTING_4(
                    x, i, Trace, IceTransportForwardSecureTransportPacketToGatherer, ol, IceTransport, Send,
                    puid, id, mID,
                    puid, iceGathererId, gatherer->getID(),
                    buffer, packet, buffer,
                    size, size, bufferSizeInBytes
                    );

      routerRoute->trace(__func__, "gatherer to use this route to send secure packet");
      return gatherer->sendPacket(*this, routerRoute, buffer, bufferSizeInBytes);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      promise->reject();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onNotifyPacketRetried(
                                             IICETypes::CandidatePtr localCandidate,
                                             IPAddress remoteIP,
                                             STUNPacketPtr stunPacket
                                             )
    {
      RouterRoutePtr routerRoute;

      {
        AutoRecursiveLock lock(*this);

        if (!mGathererRouter) {
          ZS_LOG_WARNING(Detail, log("cannot handle packet as no gather is attached") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + stunPacket->toDebug())
          return;
        }

        routerRoute = mGathererRouter->findRoute(localCandidate, remoteIP, true);
        if (!routerRoute) {
          ZS_LOG_WARNING(Detail, log("cannot handle packet as no route could be created") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + stunPacket->toDebug())
          return;
        }
      }

      ZS_EVENTING_2(
                    x, i, Trace, IceTransportRetryReceivedStunPacketFromGatherer, ol, IceTransport, Receive,
                    puid, id, mID,
                    puid, routerRouteId, routerRoute->mID
                    );
      routerRoute->trace(__func__, "retry received stun packet");
      stunPacket->trace(__func__);

      ZS_LOG_TRACE(log("retrying to handle packet again") + routerRoute->toDebug() + stunPacket->toDebug())

      // forward packet to be handled on newly established route
      notifyPacket(routerRoute, stunPacket);
    }

    //-------------------------------------------------------------------------
    void ICETransport::onWarmRoutesChanged()
    {
      ZS_EVENTING_1(x, i, Debug, OrtcIceTransportInternalWarmRoutesChangedEvent, ol, IceTransport, InternalEvent, puid, id, mID);

      ZS_LOG_DEBUG(log("warm routes changed event fired"));

      AutoRecursiveLock lock(*this);
      mWarmRoutesChanged = -1;  // by setting negative this route will be considered handled (unless set positive again)

      step();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onNotifyAttached(PUID secureTransportID)
    {
      ZS_LOG_DETAIL(log("on notify attached") + ZS_PARAM("secure transport id", secureTransportID));

      onDeliverPendingPackets();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onNotifyDetached(PUID secureTransportID)
    {
      ZS_LOG_DETAIL(log("notify detached") + ZS_PARAM("secure transport id", secureTransportID))

      AutoRecursiveLock lock(*this);

      if (secureTransportID != mSecureTransportID) {
        ZS_LOG_WARNING(Detail, log("secure transport is not attached") + ZS_PARAM("secure transport id", secureTransportID))
        return;
      }

      mSecureTransportOld = mSecureTransport;

      mSecureTransportID = 0;
      mSecureTransport.reset();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onDeliverPendingPackets()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportInternalDeliveryPendingPacketsEvent, ol, IceTransport, InternalEvent, puid, id, mID);

      ZS_LOG_TRACE(log("attempting to deliver pending packets"));

      UseSecureTransportPtr transport;
      UseSecureTransportPtr oldTransport;

      bool firstTimeOldTransport {true};

      PacketQueue packets;

      {
        AutoRecursiveLock lock(*this);

        transport = mSecureTransport.lock();
        if (!transport) {
          ZS_LOG_WARNING(Debug, log("secure transport is not attached (thus must still buffer)"))
          return;
        }

        packets = mBufferedPackets;
        mBufferedPackets = PacketQueue();
      }

      while (packets.size() > 0) {
        SecureByteBlockPtr deliverPacket = packets.front();
        packets.pop();

        {
          ZS_EVENTING_4(
                        x, i, Trace, IceTransportDeliveringBufferedIncomingPacketToSecureTransport, ol, IceTransport, Deliver,
                        puid, id, mID,
                        puid, secureTransportId, transport->getID(), 
                        buffer, packet, deliverPacket->BytePtr(),
                        size, size, deliverPacket->SizeInBytes()
                        );
          bool handled = transport->handleReceivedPacket(mComponent, deliverPacket->BytePtr(), deliverPacket->SizeInBytes());

          if (!handled) goto forward_old_transport;
          goto deliver_next;
        }

      forward_old_transport:
        {
          if (firstTimeOldTransport) {
            AutoRecursiveLock lock(*this);
            oldTransport = mSecureTransportOld.lock();
          }

          if (!oldTransport) {
            ZS_LOG_WARNING(Debug, log("no older transport available to send packet (thus discarding packet)") + ZS_PARAM("packet size", deliverPacket->SizeInBytes()))
            goto deliver_next;
          }

          ZS_EVENTING_4(
                        x, i, Trace, IceTransportDeliveringBufferedIncomingPacketToSecureTransport, ol, IceTransport, Deliver,
                        puid, id, mID,
                        puid, secureTransportId, oldTransport->getID(),
                        buffer, packet, deliverPacket->BytePtr(),
                        size, size, deliverPacket->SizeInBytes()
                        );

          bool handled = oldTransport->handleReceivedPacket(mComponent, deliverPacket->BytePtr(), deliverPacket->SizeInBytes());
          if (!handled) {
            AutoRecursiveLock lock(*this);

            auto oldTransportID = transport->getID();

            mSecureTransportOld.reset();
            ZS_LOG_DEBUG(log("old transport did not handle packet either (thus disposing of old transport)") + ZS_PARAM("old transport id", oldTransportID) + ZS_PARAMIZE(deliverPacket->SizeInBytes()))
          }
          goto deliver_next;
        }

      deliver_next:
        {
        }
      }


      {
        AutoRecursiveLock lock(*this);
        if (mBufferedPackets.size() > 0) {
          ZS_LOG_WARNING(Debug, log("more packets are pending thus attempt to deliver again"))
          IICETransportAsyncDelegateProxy::create(mThisWeak.lock())->onDeliverPendingPackets();
          return;
        }

        mMustBufferPackets = false;
      }
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
      ZS_EVENTING_1(x, i, Trace, IceTransportInternalWakeEvent, ol, IceTransport, InternalEvent, puid, id, mID);

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
    void ICETransport::onTimer(ITimerPtr timer)
    {
      ZS_EVENTING_3(
                    x, i, Trace, IceTransportInternalTimerEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, timerId, timer->getID(),
                    string, timerType, (const char *)NULL
                    );

      ZS_LOG_TRACE(log("timer fired") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);

      if (timer == mExpireRouteTimer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceTransportInternalTimerEvent, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      puid, timerId, timer->getID(),
                      string, timerType, "expire route timer"
                      );

        handleExpireRouteTimer();
        return;
      }
      if (timer == mLastReceivedPacketTimer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceTransportInternalTimerEvent, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      puid, timerId, timer->getID(),
                      string, timerType, "last received packet timer"
                      );
        handleLastReceivedPacket();
        return;
      }
      if (timer == mActivationTimer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceTransportInternalTimerEvent, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      puid, timerId, timer->getID(),
                      string, timerType, "activation timer"
                      );
        handleActivationTimer();
        return;
      }

      // scope: check keep warm timers
      {
        auto found = mNextKeepWarmTimers.find(timer);
        if (found == mNextKeepWarmTimers.end()) goto not_a_keep_warm_timer;

        ZS_EVENTING_3(
                      x, i, Trace, IceTransportInternalTimerEvent, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      puid, timerId, timer->getID(),
                      string, timerType, "next keep warm timer"
                      );

        RoutePtr route = (*found).second;
        mNextKeepWarmTimers.erase(found);
        handleNextKeepWarmTimer(route);
        return;
      }

    not_a_keep_warm_timer:
      {
        ZS_EVENTING_3(
                      x, i, Trace, IceTransportInternalTimerEvent, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      puid, timerId, timer->getID(),
                      string, timerType, "obsolete timer"
                      );

        ZS_LOG_WARNING(Trace, log("notified about an obsolete timer") + ZS_PARAM("timer id", timer->getID()));
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

      ZS_EVENTING_2(
                    x, i, Trace, IceTransportInternalUnfrozenPromiseEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, routeId, route->mID
                    );

      route->trace(__func__, "unfrozen promise settled");

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
    void ICETransport::onICEGathererStateChange(
                                                IICEGathererPtr gatherer,
                                                IICEGatherer::States state
                                                )
    {
      ZS_EVENTING_3(
                    x, i, Detail, IceTransportInternalGathererStateChangedEvent, ol, IceTransport, InternalEvent,
                    puid, id, mID,
                    puid, iceGathererId, gatherer->getID(),
                    string, state, IICEGatherer::toString(state)
                    );

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
      CandidatePtr candidate(make_shared<Candidate>(*inCandidate));
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      ZS_EVENTING_13(
                     x, i, Detail, IceTransportInternalGathererAddLocalCandidateEvent, ol, IceTransport, AddCandidate,
                     puid, id, mID,
                     puid, iceGathererId, gatherer->getID(),
                     string, interfaceType, candidate->mInterfaceType,
                     string, foundation, candidate->mFoundation,
                     dword, priority,candidate->mPriority,
                     dword, unfreezePriority, candidate->mUnfreezePriority,
                     string, protocol, IICETypes::toString(candidate->mProtocol),
                     string, ip, candidate->mIP,
                     word, port, candidate->mPort,
                     string, candidateType, IICETypes::toString(candidate->mCandidateType),
                     string, tcpCandidateType, IICETypes::toString(candidate->mTCPType),
                     string, relatedAddress, candidate->mRelatedAddress,
                     word, relatedPort, candidate->mRelatedPort
                     );

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about candidate on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + candidate->toDebug())
        return;
      }

      bool shouldRecalculate = false;

      auto hash = candidate->hash();

      auto found = mLocalCandidates.find(hash);
      if (found != mLocalCandidates.end()) {
        ZS_LOG_DEBUG(log("local candidate is already known (thus ignoring)") + candidate->toDebug())
        goto check_recalculate;
      }

      if (mLocalCandidatesComplete) {
        mLocalCandidatesComplete = false;
        mLocalCandidatesHash.clear();
        shouldRecalculate = true;
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

      ZS_EVENTING_3(
                    x, i, Detail, IceTransportInternalGathererAddLocalCandidateCompleteEvent, ol, IceTransport, AddCandidate,
                    puid, id, mID,
                    puid, iceGathererId, gatherer->getID(),
                    enum, component, candidate->mComponent
                    );

      ZS_THROW_INVALID_ASSUMPTION_IF(candidate->mComponent != mComponent);

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about candidate on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + candidate->toDebug())
        return;
      }

      if (mLocalCandidatesComplete) {
        ZS_LOG_TRACE(log("already notified end of local candidates"))
        return;
      }

      ZS_LOG_DEBUG(log("end of local candidates found") + candidate->toDebug())

      mLocalCandidatesComplete = true;

      mLocalCandidatesHash.clear();
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICEGathererLocalCandidateGone(
                                                       IICEGathererPtr inGatherer,
                                                       CandidatePtr inCandidate
                                                       )
    {
      CandidatePtr candidate(make_shared<Candidate>(*inCandidate));
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      ZS_EVENTING_13(
                     x, i, Detail, IceTransportInternalGathererRemoveLocalCandidateEvent, ol, IceTransport, RemoveCandidate,
                     puid, id, mID,
                     puid, iceGathererId, gatherer->getID(),
                     string, interfaceType, candidate->mInterfaceType,
                     string, foundation, candidate->mFoundation,
                     dword, priority, candidate->mPriority,
                     dword, unfreezePriority, candidate->mUnfreezePriority,
                     string, protocol, IICETypes::toString(candidate->mProtocol),
                     string, ip, candidate->mIP,
                     word, port, candidate->mPort,
                     string, candidateType, IICETypes::toString(candidate->mCandidateType),
                     string, tcpCandidateType, IICETypes::toString(candidate->mTCPType),
                     string, relatedAddress, candidate->mRelatedAddress,
                     word, relatedPort, candidate->mRelatedPort
                     );

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
                                          ErrorEventPtr errorEvent
                                          )
    {
      UseICEGathererPtr gatherer = ICEGatherer::convert(inGatherer);

      AutoRecursiveLock lock(*this);

      if (gatherer != mGatherer) {
        ZS_LOG_WARNING(Debug, log("notified about error on non-associated gatherer") + ZS_PARAM("gatherer", gatherer->getID()) + errorEvent->toDebug())
        return;
      }

      ZS_LOG_WARNING(Detail, log("notified gatherer has encountered an error") + errorEvent->toDebug())
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
      ZS_EVENTING_5(
                    x, i, Trace, IceTransportInternalStunRequesterSendPacket, ol, IceTransport, Send,
                    puid, id, mID,
                    puid, stunRequesterId, requester->getID(),
                    string, destinationIp, destination.string(),
                    buffer, packet, packet->BytePtr(),
                    size, size, packet->SizeInBytes()
                    );

      ZS_THROW_INVALID_ARGUMENT_IF(!packet);

      RouterRoutePtr routerRoute;
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
          installGathererRoute(route);

          routerRoute = route->mGathererRoute;
          route->mLastSentCheck = zsLib::now();
        }
      }

      if (!routerRoute) {
        ZS_LOG_WARNING(Trace, log("cannot send packet as no route set") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()))
        return;
      }

      ZS_LOG_TRACE(log("sending stun packet") + ZS_PARAM("stun requester", requester->getID()) + ZS_PARAM("destination ip", destination.string()) + ZS_PARAM("packet size", packet->SizeInBytes()) + routerRoute->toDebug())
      gatherer->sendPacket(*this, routerRoute, packet->BytePtr(), packet->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    bool ICETransport::handleSTUNRequesterResponse(
                                                   ISTUNRequesterPtr requester,
                                                   IPAddress fromIPAddress,
                                                   STUNPacketPtr response
                                                   )
    {
      ZS_EVENTING_3(
                    x, i, Trace, IceTransportInternalStunRequesterReceivedResponse, ol, IceTransport, Receive,
                    puid, id, mID,
                    puid, stunReuqesterId, requester->getID(),
                    string, fromIpAddress, fromIPAddress.string()
                    );
      response->trace(__func__);

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
          ZS_EVENTING_4(
                        x, w, Trace, IceTransportInternalStunRequesterReceivedResponseMismatch, ol, IceTransport, Receive,
                        puid, id, mID,
                        puid, stunReuqesterId, requester->getID(),
                        string, fromIpAddress, fromIPAddress.string(),
                        string, ip, ip.string()
                        );

          ZS_LOG_WARNING(Detail, log("response ip does not match sent ip") + ZS_PARAM("from ip", fromIPAddress.string()) + ZS_PARAM("sent ip", ip.string()))
          mOutgoingChecks[requester] = route; // put it back since it's not valid
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
            ZS_EVENTING_3(
                          x, w, Trace, IceTransportInternalRoleConflictDetectedEvent, ol, IceTransport, InternalEvent,
                          puid, id, mID,
                          string, role, IICETypes::toString(mOptions.mRole),
                          string, reason, "conflict error reported from report party"
                          );

            mOptionsHash = mOptions.hash();
            pruneAllCandidatePairs(true);
            wakeUp();
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
          if (IICETypes::Role_Controlling == mOptions.mRole) {
            auto previousValue = route->mLastRoundTripMeasurement;

            route->mLastRoundTripMeasurement = zsLib::toMicroseconds(getLatest(mLastReceivedPacket, route->mLastRoundTripCheck) - route->mLastRoundTripCheck);
            ZS_LOG_TRACE(log("updated route round trip time") + route->toDebug())

            if (Microseconds() != previousValue) {
              auto largest = previousValue > route->mLastRoundTripMeasurement ? previousValue : route->mLastRoundTripMeasurement;
              auto smallest = previousValue < route->mLastRoundTripMeasurement ? previousValue : route->mLastRoundTripMeasurement;
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

          removeOutgoingCheck(route);
        }
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
        route->mNextKeepWarm = ITimer::create(mThisWeak.lock(), zsLib::now() + mKeepWarmTimeBase + Milliseconds(IHelper::random(0, static_cast<size_t>(mKeepWarmTimeRandomizedAddTime.count()))));
        mNextKeepWarmTimers[route->mNextKeepWarm] = route;

        ZS_LOG_TRACE(log("installed keep warm timer") + route->toDebug())
      }

      updateAfterPacket(route);
      return true;
    }

    //-------------------------------------------------------------------------
    void ICETransport::onSTUNRequesterTimedOut(ISTUNRequesterPtr requester)
    {
      ZS_EVENTING_2(
                    x, i, Trace, IceTransportInternalStunRequesterTimedOut, ol, IceTransport, Receive,
                    puid, id, mID,
                    puid, stunRequesterId, requester->getID()
                    );

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
      IHelper::debugAppend(objectEl, "id", mID);
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
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::ICETransport");

      auto transportController = mTransportController.lock();
      auto rtpTransport = mRTPTransport.lock();

      IHelper::debugAppend(resultEl, "id", mID);


      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      IHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      IHelper::debugAppend(resultEl, "component", IICETypes::toString(mComponent));

      IHelper::debugAppend(resultEl, "state", IICETransport::toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      IHelper::debugAppend(resultEl, "gatherer", mGatherer ? mGatherer->getID() : 0);
      IHelper::debugAppend(resultEl, "gatherer subscription", mGathererSubscription ? mGathererSubscription->getID() : 0);
      IHelper::debugAppend(resultEl, "gatherer router", mGathererRouter ? mGathererRouter->getID() : 0);

      IHelper::debugAppend(resultEl, "transport controller", transportController ? transportController->getID() : 0);

      IHelper::debugAppend(resultEl, "rtp transport", rtpTransport ? rtpTransport->getID() : 0);
      IHelper::debugAppend(resultEl, "rtcp transport", mRTCPTransport ? mRTCPTransport->getID() : 0);

      IHelper::debugAppend(resultEl, "wake up", mWakeUp);
      IHelper::debugAppend(resultEl, "warm routes changed", mWarmRoutesChanged);
      IHelper::debugAppend(resultEl, "force pick route again", mForcePickRouteAgain);

      IHelper::debugAppend(resultEl, "options hash", mOptionsHash);
      IHelper::debugAppend(resultEl, "options", mOptions.toDebug());
      IHelper::debugAppend(resultEl, "conflict resolver", mConflictResolver);

      IHelper::debugAppend(resultEl, "remote parameters hash", mRemoteParametersHash);
      IHelper::debugAppend(resultEl, "remote parameters", mRemoteParameters.toDebug());

      IHelper::debugAppend(resultEl, "local candidates hash", mLocalCandidatesHash);
      IHelper::debugAppend(resultEl, "local candidates", mLocalCandidates.size());
      IHelper::debugAppend(resultEl, "end of local candidates", mLocalCandidatesComplete);

      IHelper::debugAppend(resultEl, "remote candidates hash", mRemoteCandidatesHash);
      IHelper::debugAppend(resultEl, "remote candidates", mRemoteCandidates.size());
      IHelper::debugAppend(resultEl, "end of remote candidates", mRemoteCandidatesComplete);

      IHelper::debugAppend(resultEl, "computed pairs hash", mComputedPairsHash);
      IHelper::debugAppend(resultEl, "legal routes", mLegalRoutes.size());
      IHelper::debugAppend(resultEl, "foundation routes", mFoundationRoutes.size());
      IHelper::debugAppend(resultEl, mRouteStateTracker->toDebug());

      IHelper::debugAppend(resultEl, "activation timer", mActivationTimer ? mActivationTimer->getID() : 0);
      IHelper::debugAppend(resultEl, "activate routes that received checks", mNextActivationCausesAllRoutesThatReceivedChecksToActivate);

      IHelper::debugAppend(resultEl, "pending activation", mPendingActivation.size());

      IHelper::debugAppend(resultEl, "frozen", mFrozen.size());

      IHelper::debugAppend(resultEl, "active route", mActiveRoute ? mActiveRoute->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "warm routes", mWarmRoutes.size());

      IHelper::debugAppend(resultEl, "gatherer routes", mGathererRoutes.size());

      IHelper::debugAppend(resultEl, "outgoing checks", mOutgoingChecks.size());
      IHelper::debugAppend(resultEl, "next keep warm timers", mNextKeepWarmTimers.size());

      IHelper::debugAppend(resultEl, "use candidate route", mUseCandidateRoute ? mUseCandidateRoute->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "use candidate request", mUseCandidateRequest ? mUseCandidateRequest->getID() : 0);
      IHelper::debugAppend(resultEl, "last received use candidate", mLastReceivedUseCandidate);

      IHelper::debugAppend(resultEl, "last received packet", mLastReceivedPacket);
      IHelper::debugAppend(resultEl, "last received packet timer", mLastReceivedPacketTimer ? mLastReceivedPacketTimer->getID() : 0);
      IHelper::debugAppend(resultEl, "no packets received recheck time", mNoPacketsReceivedRecheckTime);

      IHelper::debugAppend(resultEl, "expire route time", mExpireRouteTime);
      IHelper::debugAppend(resultEl, "expire route timer", mExpireRouteTimer ? mExpireRouteTimer->getID() : 0);
      IHelper::debugAppend(resultEl, "blacklist consent", mBlacklistConsent);

      IHelper::debugAppend(resultEl, "keep warm time base", mKeepWarmTimeBase);
      IHelper::debugAppend(resultEl, "keep warm randomized add time", mKeepWarmTimeRandomizedAddTime);

      IHelper::debugAppend(resultEl, "secure transport id", mSecureTransportID);
      IHelper::debugAppend(resultEl, "secure transport", (bool)(mSecureTransport.lock()));
      IHelper::debugAppend(resultEl, "secure transport (old)", (bool)(mSecureTransportOld.lock()));

      IHelper::debugAppend(resultEl, "max buffered packets", mMaxBufferedPackets);
      IHelper::debugAppend(resultEl, "must buffer packets", mMustBufferPackets);
      IHelper::debugAppend(resultEl, "buffered packets", mBufferedPackets.size());

      IHelper::debugAppend(resultEl, "received username on ICE response packet", mSTUNPacketOptions.mBindResponseRequiresUsernameAttribute);

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

      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

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
        if (!stepSetNeedsMoreCandidates()) return;
      }
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepCalculateLegalPairs()
    {
      typedef std::map<Hash, CandidatePairPtr> CandidatePairMap;

      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mComputedPairsHash.hasData()) {
        ZS_LOG_TRACE(log("already computed legal pairs"))
        return true;
      }

      CandidatePairMap pairings;

      for (auto iterLocal = mLocalCandidates.begin(); iterLocal != mLocalCandidates.end(); ++iterLocal) {
        auto localCandidate = (*iterLocal).second;

        bool isLocalIPv4 = (String::npos != localCandidate->mIP.find('.'));

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
              case IICETypes::TCPCandidateType_Active:  if (IICETypes::TCPCandidateType_Passive != remoteCandidate->mTCPType) continue; break;
              case IICETypes::TCPCandidateType_Passive: if (IICETypes::TCPCandidateType_Active != remoteCandidate->mTCPType) continue; break;
              case IICETypes::TCPCandidateType_SO:      if (IICETypes::TCPCandidateType_SO != remoteCandidate->mTCPType) continue; break;
            }
          }

          bool isRemoteIPv4 = (String::npos != remoteCandidate->mIP.find('.'));

          if (isLocalIPv4 != isRemoteIPv4) continue;  // cannot match unless they both are either IPv4 or IPv6

          CandidatePairPtr candidatePair(make_shared<CandidatePair>());
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

        if (IICETypes::CandidateType_Prflx != route->mCandidatePair->mRemote->mCandidateType) goto shutdown_route;

        goto check_remote_reflexive;

      check_remote_reflexive:
        {
          // make sure local is still valid first
          auto localCandidateHash = route->mCandidatePair->mLocal->hash();
          auto foundLocal = mLocalCandidates.find(localCandidateHash);

          if (foundLocal == mLocalCandidates.end()) {
            ZS_LOG_WARNING(Debug, log("local candidate is gone (thus pairing must be trimmed)"))
            goto shutdown_route;
          }

          goto check_route_state;
        }

      check_route_state:
        {
          // special situation where candidate is local
          switch (route->state()) {
            case Route::State_New:          goto keep_route;
            case Route::State_Pending:      goto keep_route;
            case Route::State_Frozen:       goto keep_route;
            case Route::State_InProgress:   goto keep_route;
            case Route::State_Succeeded:    goto keep_route;
            case Route::State_Ignored:      goto shutdown_route;
            case Route::State_Failed:       goto shutdown_route;
            case Route::State_Blacklisted:  goto shutdown_route;
          }
          goto shutdown_route;
        }

      keep_route:
        {
          ZS_LOG_TRACE(log("peer reflexive route still in valid state (thus still legal)") + route->toDebug())
          continue;
        }

      shutdown_route:
        {
          ZS_LOG_WARNING(Debug, log("route no longer valid (no candidate pairing found or peer reflexive no longer valid)") + route->toDebug())
          shutdown(route);
          continue;
        }
      }

      for (auto iter = pairings.begin(); iter != pairings.end(); ++iter)
      {
        auto hash = (*iter).first;
        auto candidatePair = (*iter).second;

        RoutePtr route(make_shared<Route>(mRouteStateTracker));
        route->mCandidatePair = candidatePair;
        route->mCandidatePairHash = hash;

        route->trace(__func__, "new legal route");

        ZS_LOG_DEBUG(log("found new legal route") + route->toDebug())
        mLegalRoutes[hash] = route;

        installFoundation(route);
      }

      auto hasher = IHasher::sha1();

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
        auto hash = (*iter).first;

        hasher->update(hash);
        hasher->update(":");
      }

      mComputedPairsHash = hasher->finalizeAsString();
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepPendingActivation()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (0 == mRouteStateTracker->count(Route::State_New)) {
        ZS_LOG_TRACE(log("no routes pending activation"))
        return true;
      }

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot activate any pending candidates as remote username is not set"))
        return true;
      }

      ZS_LOG_DEBUG(log("calculating pending activation"))

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter)
      {
        auto hash = (*iter).first;
        auto route = (*iter).second;

        if (route->state() != Route::State_New) continue;

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

      ZS_LOG_DEBUG(log("pending activation is now calculated") + ZS_PARAM("total pending", mRouteStateTracker->count(Route::State_Pending)))
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepActivationTimer()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      auto now = zsLib::now();

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot activate any candidates as remote username is not set"))
        goto no_activation_timer;
      }

      if (isContinousGathering()) {
        if (Time() != mLastReceivedPacket) {
          if (mLastReceivedPacket + mNoPacketsReceivedRecheckTime < now) {
            ZS_LOG_TRACE(log("needs activation timer as no remote packet has been received for a while"))
            goto need_activation_timer;
          }
        }
      } else {
        if ((isComplete()) ||
            (isDisconnected()) ||
            (isFailed())) {
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
          auto duration = Milliseconds(ISettings::getUInt(ORTC_SETTING_ICE_TRANSPORT_ACTIVATION_TIMER_IN_MILLISECONDS));
          ZS_LOG_DEBUG(log("creating activation timer") + ZS_PARAM("duration (ms)", duration))
          mActivationTimer = ITimer::create(mThisWeak.lock(), duration);
        }
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepPickRoute()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot pick a route as no remote username is set"))
        return true;
      }
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
      const char *reason = "choosing preferred route";

      Microseconds fastestRoutePicked {};

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
            ZS_LOG_WARNING(Trace, log("will not consider this route because of recent inactivity") + ZS_PARAM("difference", (now - lastRemoteActivity)) + route->toDebug())
            continue;
          }
        }

        if (Microseconds() != fastestRoutePicked) {
          if (Microseconds() != route->mLastRoundTripMeasurement) {
            if (isMagnitudeFaster(fastestRoutePicked, route->mLastRoundTripMeasurement)) {
              ZS_LOG_TRACE(log("route has faster round trip time (thus will pick)") + route->toDebug() + ZS_PARAM("previous speed", fastestRoutePicked))
              reason = "route is magnitude faster than previous route";
              goto chose_this_route;
            }

            ZS_LOG_INSANE(log("route is not a magnitude faster round trip time (thus will not pick)") + route->toDebug() + ZS_PARAM("previous speed", fastestRoutePicked))
          }
        }

        if (preference < chosenPreference) continue; // previous preference is preferred over current preference
        reason = "choosing preferred route";

      chose_this_route:
        {
          chosenRoute = route;
          chosenPreference = preference;
          fastestRoutePicked = route->mLastRoundTripMeasurement;
        }
      }

      if (mActiveRoute != chosenRoute) {
        if (!chosenRoute) {
          ZS_LOG_TRACE(log("no route to choose (thus cannot choose route at this time)"))
          return true;
        }

        mActiveRoute = chosenRoute;
        ZS_EVENTING_2(
                      x, i, Debug, IceTransportCandidatePairChangedEvent, ol, IceTransport, Event,
                      puid, id, mID,
                      puid, activeRouteId, mActiveRoute->mID
                      );

        mActiveRoute->trace(__func__, reason);
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
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot use candidate as remote username is not set"))
        return true;
      }

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

      if (State_Completed != mCurrentState) {
        ZS_LOG_TRACE(log("cannot nominate candidate because state is not completed yet") + ZS_PARAM("state", toString(mCurrentState)))
        return true;
      }

      if (mUseCandidateRequest) {
        ZS_LOG_DEBUG(log("removing previous use candidate"))
        mUseCandidateRequest->cancel();
        mUseCandidateRequest.reset();
      }

      ZS_LOG_DEBUG(log("nominating candidate") + mActiveRoute->toDebug())

      mUseCandidateRoute = mActiveRoute;
      mUseCandidateRoute->trace(__func__, "nominate candidate");

      mUseCandidateRequest = createBindRequest(mUseCandidateRoute, true);

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepDewarmRoutes()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot dewarm as remote username is not set"))
        return true;
      }

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

        if (Time() != route->mLastReceivedCheck) {
          if (route->mLastReceivedCheck + mExpireRouteTime > now) {
            ZS_LOG_INSANE(log("have received check (thus do not expire)") + route->toDebug() + ZS_PARAM("expire (s)", mExpireRouteTime))
            continue;
          }
        }

        if (route->mLastReceivedResponse + mExpireRouteTime > now) {
          ZS_LOG_INSANE(log("have received responce to check (thus do not expire)") + route->toDebug() + ZS_PARAM("expire (s)", mExpireRouteTime))
          continue;
        }

        route->trace(__func__, "dewarm");

        ZS_LOG_DEBUG(log("route cannot be kept warm any longer (expired)") + ZS_PARAM("difference", (now - route->mLastReceivedResponse)) + route->toDebug() + ZS_PARAM("expires (s)", mExpireRouteTime))

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
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot warm routes as remote username is not set"))
        return true;
      }

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
        if (Microseconds() == route->mLastRoundTripMeasurement) goto must_keep_warm;  // need a round trip time measurement to happen

        goto do_not_keep_warm;

      must_keep_warm:
        {
          ZS_LOG_INSANE(log("route must be kept warm") + route->toDebug())

          if (route->mOutgoingCheck) {
            ZS_LOG_INSANE(log("already have outgoing check (thus do not need timer)") + route->toDebug())
            continue;
          }
          if (route->mNextKeepWarm) {
            ZS_LOG_INSANE(log("already have next keep warm timer (thus do not need timer)") + route->toDebug())
            continue;
          }

          route->trace(__func__, "keep warm");

          ZS_LOG_DEBUG(log("installing keep warm timer") + route->toDebug())

          route->mNextKeepWarm = ITimer::create(mThisWeak.lock(), zsLib::now() + mKeepWarmTimeBase + Milliseconds(IHelper::random(0, static_cast<size_t>(mKeepWarmTimeRandomizedAddTime.count()))));
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
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot expire routes as remote username is not set"))
        return true;
      }

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

      mExpireRouteTimer = ITimer::create(mThisWeak.lock(), mExpireRouteTime);
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::stepLastReceivedPacketTimer()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if (mRemoteParameters.mUsernameFragment.isEmpty()) {
        ZS_LOG_TRACE(log("cannot have last packet received timer as remote username is not set"))
        return true;
      }

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
        mLastReceivedPacketTimer = ITimer::create(mThisWeak.lock(), mNoPacketsReceivedRecheckTime);
      }
      
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::stepSetCurrentState()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      if ((mRemoteCandidates.size() < 1) &&
          (!mRemoteCandidatesComplete)) {
        ZS_LOG_INSANE(log("state is new (no remote candidates found)") + ZS_PARAM("total remote candidates", mRemoteCandidates.size()))
        setState(IICETransport::State_New);
        return true;
      }

      bool pendingChecks = ((0 != mRouteStateTracker->count(Route::State_New)) ||
                            (0 != mRouteStateTracker->count(Route::State_Pending)) ||
                            (0 != mRouteStateTracker->count(Route::State_Frozen)) ||
                            (0 != mRouteStateTracker->count(Route::State_InProgress)));
      bool localAndRemoteComplete = mLocalCandidatesComplete && mRemoteCandidatesComplete;

      if (localAndRemoteComplete) {
        if (pendingChecks) {
          if (mActiveRoute) {
            ZS_LOG_INSANE(debug("state is connected (local and remote complete, pending checks, and active route found)"))
            setState(IICETransport::State_Connected);
            return true;
          }
          ZS_LOG_INSANE(debug("state is disconnected (local and remote complete, pending checks, no active route found)"))
          setState(IICETransport::State_Disconnected);
          return true;
        }

        if (mActiveRoute) {
          ZS_LOG_INSANE(debug("state is complete (local and remote complete, no pending checks, active route found)"))
          setState(IICETransport::State_Completed);
        } else {
          ZS_LOG_INSANE(debug("state is failed (local and remote complete, no pending checks, no active route found)"))
          setState(IICETransport::State_Failed);
        }
        return true;
      }

      if (mActiveRoute) {
        ZS_LOG_INSANE(debug("state is connected (not complete, may have pending checks, active route found)"))
        setState(IICETransport::State_Connected);
        return true;
      }

      if (pendingChecks) {
        ZS_LOG_INSANE(debug("state is checking (not complete, has pending checks, no active route found)"))
        setState(IICETransport::State_Checking);
        return true;
      }

      if (mLocalCandidates.size() < 1) {
        ZS_LOG_INSANE(debug("state is checking (not complete, has pending checks, no active route found)"))
        setState(IICETransport::State_Checking);
        return true;
      }

      ZS_LOG_INSANE(debug("state is disconnected (not complete, no pending checks, no active route found)"))
      setState(IICETransport::State_Disconnected);
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepSetNeedsMoreCandidates()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportStep, ol, IceTransport, Step, puid, id, mID);

      mNeedsMoreCandidates = getNeedsMoreCandidates();
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
      auto pThis = mThisWeak.lock();
      if (pThis) {
        IICETransportAsyncDelegateProxy::create(mThisWeak.lock())->onWarmRoutesChanged();
      }
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::hasWarmRoutesChanged()
    {
      return 0 != mWarmRoutesChanged;
    }

    //-------------------------------------------------------------------------
    void ICETransport::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportCancel, ol, IceTransport, Cancel, puid, id, mID);

      //.......................................................................
      // start the shutdown process
      if (isShutdown()) {
        ZS_LOG_WARNING(Trace, log("already shutdown"))
        return;
      }

      // try to gracefully shutdown
      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      stepSetCurrentState();
      stepSetNeedsMoreCandidates();

      if (mGracefulShutdownReference) {
        // no async shutdown required (at this time)
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

      if (mGathererSubscription) {
        mGathererSubscription->cancel();
        mGathererSubscription.reset();
      }

      mTransportController.reset();

      mGathererRouter.reset();

      pruneAllCandidatePairs(false);

      setState(IICETransportTypes::State_Closed);

      mSubscriptions.clear();

      mLocalCandidates.clear();
      mLocalCandidatesComplete = true;

      mRemoteCandidates.clear();
      mRemoteCandidatesComplete = true;

      mLegalRoutes.clear();
      mFoundationRoutes.clear();

      if (mActivationTimer) {
        mActivationTimer->cancel();
        mActivationTimer.reset();
      }

      mPendingActivation.clear();

      for (auto iter_doNotUse = mFrozen.begin(); iter_doNotUse != mFrozen.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto promise = (*current).first;
        promise->reject(make_shared<ReasonNoMoreRelationship>());
      }

      mActiveRoute.reset();

      mWarmRoutes.clear();

      mGathererRoutes.clear();

      mOutgoingChecks.clear();
      mNextKeepWarmTimers.clear();

      mUseCandidateRoute.reset();
      if (mUseCandidateRequest) {
        mUseCandidateRequest->cancel();
        mUseCandidateRequest.reset();
      }

      if (mLastReceivedPacketTimer) {
        mLastReceivedPacketTimer->cancel();
        mLastReceivedPacketTimer.reset();
      }

      if (mExpireRouteTimer) {
        mExpireRouteTimer->cancel();
        mExpireRouteTimer.reset();
      }

      mSecureTransportID = 0;
      mSecureTransport.reset();
      mSecureTransportOld.reset();
    }

    //-------------------------------------------------------------------------
    void ICETransport::setState(IICETransport::States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", IICETransport::toString(state)) + ZS_PARAM("old state", IICETransport::toString(mCurrentState)))

      mCurrentState = state;
      ZS_EVENTING_2(x, i, Debug, IceTransportStateChangedEvent, ol, IceTransport, StateEvent, puid, id, mID, string, state, IICETransport::toString(state));

      ICETransportPtr pThis = mThisWeak.lock();
      if (pThis) {
        if (State_Completed == mCurrentState) {
          if ((!mUseCandidateRoute) &&
              (!mOptions.mAggressiveICE) &&
              (IICETypes::Role_Controlling == mOptions.mRole)) {
            ZS_LOG_TRACE(log("route nomination is required"))
            IWakeDelegateProxy::create(pThis)->onWake();
          }
        }

        mSubscriptions.delegate()->onICETransportStateChange(pThis, mCurrentState);
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
      ZS_EVENTING_3(
                    x, e, Debug, IceTransportErrorEvent, ol, IceTransport, ErrorEvent,
                    puid, id, mID,
                    word, errorCode, errorCode,
                    string, reason, reason
                    );

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
    bool ICETransport::isFailed() const
    {
      return IICETransport::State_Failed == mCurrentState;
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
            mActiveRoute->trace(__func__, "expire route");
            if (mBlacklistConsent) {
              ZS_LOG_WARNING(Detail, log("blacklisting consent on active route") + ZS_PARAM("difference", (now - latest)) + mActiveRoute->toDebug())
              setBlacklisted(mActiveRoute);
            } else {
              ZS_LOG_WARNING(Detail, log("active route has now failed") + ZS_PARAM("difference", (now - latest)) + mActiveRoute->toDebug())
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

          route->trace(__func__, "warm route expired");

          expired = true;
          mWarmRoutes.erase(current);
          warmRoutesChanged();

          ZS_LOG_WARNING(Debug, log("warm route no longer is warm") + ZS_PARAM("difference", (now - latest)) + route->toDebug())

          if (mBlacklistConsent) {
            ZS_LOG_WARNING(Detail, log("warm route is blacklisted due to consent removal") + route->toDebug())
            setBlacklisted(route);
          } else {
            setFailed(route);
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

      for (auto iter = mWarmRoutes.begin(); iter != mWarmRoutes.end(); ++iter)
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

      if (mNextActivationCausesAllRoutesThatReceivedChecksToActivate) {
        bool didActivate = false;

        for (auto iter_doNotUse = mPendingActivation.begin(); iter_doNotUse != mPendingActivation.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto route = (*current).second;

          if (IICETypes::Protocol_TCP == route->mCandidatePair->mLocal->mProtocol) {
            if (IICETypes::TCPCandidateType_Passive == route->mCandidatePair->mLocal->mTCPType) {
              ZS_LOG_TRACE(log("never activate a TCP passive route (since only active connections can open outgoing TCP socket)") + route->toDebug())
              continue;
            }
          }

          if (Time() == route->mLastReceivedCheck) continue;

          ZS_LOG_DEBUG(log("activating route") + route->toDebug())

          didActivate = true;

          setInProgress(route);
          if (mPendingActivation.size() < 1) {
            ZS_LOG_DEBUG(log("might not need activation timer anymore (thus waking up)"))
            wakeUp();
          }
        }

        mNextActivationCausesAllRoutesThatReceivedChecksToActivate = false;

        if (didActivate) {
          ZS_LOG_TRACE(log("did activate at least one route (thus going to check activation timer)"))
          goto check_activation_timer;
        }
      }

      for (auto iter_doNotUse = mPendingActivation.begin(); iter_doNotUse != mPendingActivation.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;

        if (!mTestLowerPreferenceCandidatePairs) {
          if ((mActiveRoute) &&
              (route != mActiveRoute)) {
            auto activePreference = mActiveRoute->getPreference(Role_Controlling == mOptions.mRole);
            auto preference = route->getPreference(Role_Controlling == mOptions.mRole);

            if (activePreference > preference) {
              ZS_LOG_TRACE(log("no need to activate this route (since it cannot be used due to lower preference)") + ZS_PARAM("active preference", activePreference) + ZS_PARAM("preference", preference) + ZS_PARAM("route", mActiveRoute->toDebug()) + ZS_PARAM("route", route->toDebug()))
              goto set_route_ignored;
            }
          }
        }

        if (IICETypes::Protocol_TCP == route->mCandidatePair->mLocal->mProtocol) {
          if (IICETypes::TCPCandidateType_Passive == route->mCandidatePair->mLocal->mTCPType) {
            ZS_LOG_TRACE(log("never activate a TCP passive route (since only active connections can open outgoing TCP socket)") + route->toDebug())
            goto set_route_ignored;
          }
        }

        if (IICETypes::Component_RTCP == mComponent) {

          auto rtpTransport = mRTPTransport.lock();

          // freeze upon RTP component
          if (!rtpTransport) {
            ZS_LOG_TRACE(log("no ice transport to freeze upon (thus activating now)") + route->toDebug())
            goto activate_now;
          }

          PromisePtr promise = Promise::create(getAssociatedMessageQueue());
          bool found = rtpTransport->hasCandidatePairFoundation(route->mCandidatePair->mLocal->mFoundation, route->mCandidatePair->mRemote->mFoundation, promise);
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

      set_route_ignored:
        {
          setIgnored(route);
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
        ZS_LOG_TRACE(log("packet received recently (thus no need to re-activate anything)") + ZS_PARAM("difference", (now - mLastReceivedPacket)))
        goto check_activation_timer;
      }

      ZS_LOG_WARNING(Debug, log("packet was not received recently (thus need to re-activate something)") + ZS_PARAM("difference", (now - mLastReceivedPacket)))

      // scope: try to activate an old route just in case something can be resolved
      {
        // try to activate previously successful routes
        for (int loop = 0; loop < 2; ++loop) {
          for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
            auto route = (*iter).second;

            if (IICETypes::Protocol_TCP == route->mCandidatePair->mLocal->mProtocol) {
              if (IICETypes::TCPCandidateType_Passive == route->mCandidatePair->mLocal->mTCPType) {
                ZS_LOG_TRACE(log("never activate a TCP passive route (since only active connections can open outgoing TCP socket)") + route->toDebug())
                continue;
              }
            }

            if (0 == loop) {
              if ((route->isSucceeded()) ||
                  (route->isIgnored())) goto activate_previous_route;
            } else {
              if (route->isFailed()) goto activate_previous_route;
            }

            // do not activate this route (not in a state that can be activated)
            continue;

          activate_previous_route:
            {
              ZS_LOG_DEBUG(log("activating route due to possible failure situation about to happen") + route->toDebug())
              route->trace(__func__, "activate route (due to potential failure condition as no remote activity detected)");
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
      route->mLastRoundTripCheck = zsLib::now();
      mOutgoingChecks[route->mOutgoingCheck] = route;

      ZS_LOG_INSANE(log("installed outgoing stun binding keep alive") + route->toDebug())
    }

    //-----------------------------------------------------------------------
    void ICETransport::forceActive(RoutePtr route)
    {
      if (!route) return;
      if (route->mPrune) return;

      if (route->mNextKeepWarm) return;

      route->trace(__func__, "forced active");

      // install a temporary keep warm timer (to force route activate sooner)
      route->mNextKeepWarm = ITimer::create(mThisWeak.lock(), zsLib::now() + Milliseconds(IHelper::random(0, static_cast<size_t>(mKeepWarmTimeRandomizedAddTime.count()))));
      mNextKeepWarmTimers[route->mNextKeepWarm] = route;

      ZS_LOG_TRACE(log("forcing route to generate activity") + route->toDebug())
    }

    //-----------------------------------------------------------------------
    void ICETransport::shutdown(RoutePtr route)
    {
      if (!route) return;

      route->trace(__func__, "shutdown");

      ZS_LOG_TRACE(log("shutting down route") + route->toDebug())

      removeLegal(route);
      removeFoundation(route);
      removePendingActivation(route);
      removeFrozen(route);
      removeFrozenDependencies(route, false, make_shared<ReasonNoMoreRelationship>());
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

        route->mGathererRoute.reset();  // reset the route since it's gone

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

      CandidatePairPtr tempPair(make_shared<CandidatePair>());
      tempPair->mLocal = make_shared<Candidate>(*(route->mCandidatePair->mLocal));
      tempPair->mRemote = make_shared<Candidate>(*(route->mCandidatePair->mRemote));
      return tempPair;
    }
    
    //-------------------------------------------------------------------------
    void ICETransport::setPending(RoutePtr route)
    {
      switch (route->state()) {
        case Route::State_New: {
          ZS_LOG_DEBUG(log("route can go into pending state") + route->toDebug())
          goto insert_pending;
        }
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
        case Route::State_Ignored:    {
          ZS_LOG_DEBUG(log("route previously ignored but is being rechecked again") + route->toDebug())
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
        route->trace(__func__, "pending");

        ZS_LOG_DEBUG(log("route is now being set to the pending state") + route->toDebug())

        removeFrozen(route);
        removeOutgoingCheck(route);
        removeKeepWarmTimer(route);
        removeWarm(route);
        route->mLastReceivedResponse = Time();  // need to recheck thus no response receieved

        // NOTE: reversing priority value so largest value becomes smallest value
        auto priority = (getMaxQWORD() ^ route->getActivationPriority(IICETypes::Role_Controlling == mOptions.mRole, mRemoteParameters.mUseUnfreezePriority));

        auto found = mPendingActivation.find(priority);
        while (found != mPendingActivation.end()) {
          auto comparePriority = (*found).first;
          if (priority != comparePriority) break;
          ++priority;
          ++found;
        }

        route->mPendingPriority = priority;
        mPendingActivation[priority] = route;

        route->state(Route::State_Pending);
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setFrozen(
                                 RoutePtr route,
                                 PromisePtr promise
                                 )
    {
      switch (route->state()) {
        case Route::State_New:        {
          ZS_LOG_DEBUG(log("route is going from new to frozen") + route->toDebug())
          goto freeze;
        }
        case Route::State_Pending:    {
          ZS_LOG_DEBUG(log("route is going from pending to frozen") + route->toDebug())
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
        case Route::State_Ignored:    {
          ZS_LOG_DEBUG(log("route previously ignored (no need to freeze)") + route->toDebug())
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
        route->trace(__func__, "frozen");

        ZS_LOG_DEBUG(log("route is now being frozen") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);

        route->state(Route::State_Frozen);
        route->mFrozenPromise = promise;
        mFrozen[promise] = route;

        promise->thenWeak(mThisWeak.lock());
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setInProgress(RoutePtr route)
    {
      switch (route->state()) {
        case Route::State_New:        goto in_progress;
        case Route::State_Pending:    goto in_progress;
        case Route::State_Frozen:     goto in_progress;
        case Route::State_InProgress: {
          ZS_LOG_WARNING(Detail, log("route is already in progress (no need to activate again)") + route->toDebug())
          return;
        }
        case Route::State_Succeeded: {
          ZS_LOG_DEBUG(log("route was succeeded (but going to activate now again)") + route->toDebug())
          goto in_progress;
        }
        case Route::State_Ignored: {
          ZS_LOG_DEBUG(log("route was ignored (but going to activate again)") + route->toDebug())
          goto in_progress;
        }
        case Route::State_Failed:    {
          ZS_LOG_DEBUG(log("route previously failed (but going to activate again)") + route->toDebug())
          goto in_progress;
        }
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted") + route->toDebug())
          return;
        }
      }

    in_progress:
      {
        route->trace(__func__, "in progress");

        ZS_LOG_DEBUG(log("route is now being set in progress") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeOutgoingCheck(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->state(Route::State_InProgress);
        route->mLastReceivedResponse = Time();

        route->mOutgoingCheck = createBindRequest(route);
        route->mLastRoundTripCheck = zsLib::now();
        mOutgoingChecks[route->mOutgoingCheck] = route;

        ZS_LOG_TRACE(log("route set in progress") + route->toDebug())
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setSucceeded(RoutePtr route)
    {
      bool wasSuccessful = route->isSucceeded();

      switch (route->state()) {
        case Route::State_New:        goto succeeded;
        case Route::State_Pending:    goto succeeded;
        case Route::State_Frozen:     goto succeeded;
        case Route::State_InProgress: goto succeeded;
        case Route::State_Succeeded:  goto succeeded;
        case Route::State_Ignored:    goto succeeded;
        case Route::State_Failed:     goto succeeded;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot succeed)") + route->toDebug())
          return;
        }
      }

    succeeded:
      {
        route->trace(__func__, "succeeded");

        ZS_LOG_DEBUG(log("route is now successful") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, true);
        if (!wasSuccessful) {
          removeOutgoingCheck(route);
        }

        route->state(Route::State_Succeeded);

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
      switch (route->state()) {
        case Route::State_New:        goto failed;
        case Route::State_Pending:    goto failed;
        case Route::State_Frozen:     goto failed;
        case Route::State_InProgress: goto failed;
        case Route::State_Succeeded:  goto failed;
        case Route::State_Ignored:    goto failed;
        case Route::State_Failed:     goto failed;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot fail)") + route->toDebug())
          return;
        }
      }

    failed:
      {
        route->trace(__func__, "failed");

        ZS_LOG_DEBUG(log("route is now a failure") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, false);
        removeOutgoingCheck(route);
        removeGathererRoute(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->state(Route::State_Failed);
        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setIgnored(RoutePtr route)
    {
      switch (route->state()) {
        case Route::State_New:        goto ignored;
        case Route::State_Pending:    goto ignored;
        case Route::State_Frozen:     goto ignored;
        case Route::State_InProgress: goto ignored;
        case Route::State_Succeeded:  goto ignored;
        case Route::State_Ignored:    goto ignored;
        case Route::State_Failed:     goto ignored;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot be ignored)") + route->toDebug())
          return;
        }
      }

    ignored:
      {
        route->trace(__func__, "ignored");

        ZS_LOG_DEBUG(log("route is now being ignored") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, false);
        removeOutgoingCheck(route);
        removeGathererRoute(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->state(Route::State_Ignored);
        wakeUp();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransport::setBlacklisted(RoutePtr route)
    {
      switch (route->state()) {
        case Route::State_New:        goto blacklist;
        case Route::State_Pending:    goto blacklist;
        case Route::State_Frozen:     goto blacklist;
        case Route::State_InProgress: goto blacklist;
        case Route::State_Succeeded:  goto blacklist;
        case Route::State_Ignored:    goto blacklist;
        case Route::State_Failed:     goto blacklist;
        case Route::State_Blacklisted: {
          ZS_LOG_WARNING(Detail, log("route is blacklisted (and cannot blacklist again)") + route->toDebug())
          return;
        }
      }

    blacklist:
      {
        route->trace(__func__, "blacklisted");

        ZS_LOG_DEBUG(log("route is now blacklisted") + route->toDebug())

        removePendingActivation(route);
        removeFrozen(route);
        removeFrozenDependencies(route, false);
        removeOutgoingCheck(route);
        removeGathererRoute(route);
        removeKeepWarmTimer(route);
        removeWarm(route);

        route->mKeepWarm = false;

        route->state(Route::State_Blacklisted);
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

      if (IICETypes::Protocol_UDP == route->mCandidatePair->mLocal->mProtocol) {
        if (IICETypes::CandidateType_Host == route->mCandidatePair->mLocal->mCandidateType) {
          switch (route->mCandidatePair->mRemote->mCandidateType) {
            case IICETypes::CandidateType_Host:   break;
            case IICETypes::CandidateType_Prflx:
            case IICETypes::CandidateType_Srflex:
            case IICETypes::CandidateType_Relay:  {
              // NOTE: If the remote candidate is seen as non-host related
              // then likely the remote party is sending to the local party's
              // reflexive candidate if the local candidate sees the packet
              // arrive on the local host. Thus we must inform the gatherer
              // that activity is likely occuring on its reflexive candidates
              // associated with the local candidate in order that the gather
              // does not prune a potential candidate where activity is
              // occuring (where the gatherer would not likely know otherwise).
              if ((mGatherer) &&
                  (route->mGathererRoute)) {
                ZS_LOG_INSANE(log("notify gatherer of likely reflexive activity") + route->toDebug())
                mGatherer->notifyLikelyReflexiveActivity(route->mGathererRoute);
              }
              break;
            }
          }
        }
      }

      RoutePtr oldActiveRoute = mActiveRoute;

      if (!mActiveRoute) {
        if (Time() != route->mLastReceivedResponse) {
          ZS_LOG_DEBUG(log("setting route to active since received a response and no other route is available") + route->toDebug())
          mActiveRoute = route;
          mActiveRoute->trace(__func__, "choosing as active route (as response was received and no other route is available)");
        } else if (Time() != route->mLastReceivedCheck) {
          ZS_LOG_DEBUG(log("setting route to active since received a validated incoming request") + route->toDebug())
          mActiveRoute = route;
          mActiveRoute->trace(__func__, "choosing as active route (as incoming check was received on this route)");
        }
      }

      auto latest = getLatest(route->mLastReceivedResponse, route->mLastReceivedCheck);
      if (Time() != latest) {
        auto now = zsLib::now();
        if (latest + mExpireRouteTime > now) {
          if (!route->isBlacklisted()) {
            ZS_LOG_TRACE(log("consent was granted") + route->toDebug())
            if (route->isSucceeded()) {
              if (mWarmRoutes.end() == mWarmRoutes.find(route->mCandidatePairHash)) {
                ZS_LOG_TRACE(log("need to keep candiate in the warm table since it's been woken up again"))
                setSucceeded(route);
              }
            } else {
              if (((Time() != route->mLastReceivedCheck) ||
                   (mRemoteParameters.mICELite)) &&
                  (Time() != route->mLastReceivedResponse)) {
                ZS_LOG_DEBUG(log("route is now a success") + route->toDebug())
                setSucceeded(route);
              }
            }
          } else {
            ZS_LOG_WARNING(Trace, log("consent cannot be granted because route is blacklisted") + route->toDebug())
          }
        }
      }

      if (oldActiveRoute != mActiveRoute) {
        ZS_LOG_DETAIL(log("new route chosen") + mActiveRoute->toDebug());
        ZS_EVENTING_2(
                      x, i, Debug, IceTransportCandidatePairChangedEvent, ol, IceTransport, Event,
                      puid, id, mID,
                      puid, activeRouteId, mActiveRoute->mID
                      );

        mSubscriptions.delegate()->onICETransportCandidatePairChanged(mThisWeak.lock(), cloneCandidatePair(mActiveRoute));

        wakeUp();
      }

      if (mUseCandidateRoute != mActiveRoute) {
        if (!mOptions.mAggressiveICE) {
          if (IICETypes::Role_Controlling == mOptions.mRole) {
            if (Time() != route->mLastReceivedResponse) {
              if (IICETransport::State_Completed == mCurrentState) {
                ZS_LOG_DEBUG(log("need to wake up and check if this route can be nominated"))
                wakeUp();
              }
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
      if (!mGathererRouter) {
        ZS_LOG_WARNING(Trace, log("cannot install gatherer route as gatherer router is not present") + route->toDebug())
        return false;
      }
      if (route->mGathererRoute) {
        ZS_LOG_INSANE(log("gatherer route already installed") + route->toDebug())
        return true;
      }

      auto ip = route->mCandidatePair->mRemote->ip();
      route->mGathererRoute = mGathererRouter->findRoute(route->mCandidatePair->mLocal, ip, true);
      if (!route->mGathererRoute) {
        ZS_LOG_WARNING(Debug, log("failed to install gatherer route") + route->toDebug())
        return false;
      }

      route->trace(__func__, "gatherer route installed");

      mGathererRoutes[route->mGathererRoute->mID] = route;
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

        ZS_EVENTING_4(
                      x, i, Debug, IceTransportInstallFoundation, ol, IceTransport, Info,
                      puid, id, mID,
                      string, localFoundation, route->mCandidatePair->mLocal->mFoundation,
                      string, remoteFoundation, route->mCandidatePair->mRemote->mFoundation,
                      size_t, totalRoutes, routes.size()
                      );

        route->trace(__func__, "installed foundation");

        mFoundationRoutes[foundation] = routes;
        return;
      }

      {
        RouteMap &routes = (*found).second;

        auto foundRoute = routes.find(route->mCandidatePairHash);
        if (foundRoute != routes.end()) return; // already installed

        routes[route->mCandidatePairHash] = route;

        ZS_EVENTING_4(
                      x, i, Debug, IceTransportInstallFoundation, ol, IceTransport, Info,
                      puid, id, mID,
                      string, localFoundation, route->mCandidatePair->mLocal->mFoundation,
                      string, remoteFoundation, route->mCandidatePair->mRemote->mFoundation,
                      size_t, totalRoutes, routes.size()
                      );

        route->trace(__func__, "installed foundation");
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

      ZS_EVENTING_4(
                    x, i, Debug, IceTransportRemoveFoundation, ol, IceTransport, Info,
                    puid, id, mID,
                    string, localFoundation, route->mCandidatePair->mLocal->mFoundation,
                    string, remoteFoundation, route->mCandidatePair->mRemote->mFoundation,
                    size_t, totalRoutes, routes.size()
                    );

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
      if (!route->mGathererRoute) return;

      auto found = mGathererRoutes.find(route->mGathererRoute->mID);
      if (found != mGathererRoutes.end()) {
        mGathererRoutes.erase(found);
      }
      if (mGatherer) {
        mGatherer->removeRoute(route->mGathererRoute);
      }

      route->trace(__func__, "removing fatherer route");
      route->mGathererRoute.reset();
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

      String hash = localCandidate->hash();

      RoutePtr closeEnoughRoute;

      for (auto iter = mLegalRoutes.begin(); iter != mLegalRoutes.end(); ++iter) {
        auto route = (*iter).second;

        String localCandidateHash = route->mCandidatePair->mLocal->hash(false);

        if (localCandidateHash != hash) continue;

        ZS_LOG_INSANE(log("found local candidate match (but not remote yet)") + route->toDebug())

        auto remoteIP = route->mCandidatePair->mRemote->ip();

        if (inRemoteIP.isAddressEqualIgnoringIPv4Format(remoteIP)) {
          if (remotePriority == route->mCandidatePair->mRemote->mPriority) {
            ZS_LOG_INSANE(log("found local + remote candidate match") + route->toDebug())
            return route;
          }
          if (0 == remotePriority) {
            ZS_LOG_INSANE(log("found local + remote candidate match (since remote priority is 0)") + route->toDebug())
            return route;
          }
          ZS_LOG_INSANE(log("found local + remote close match") + route->toDebug())
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
                                                      bool useCandidate,
                                                      IBackOffTimerPatternPtr pattern
                                                      ) const
    {
      ZS_THROW_BAD_STATE_IF(!mGatherer)

      if (IICETypes::Role_Controlling == mOptions.mRole) {
        useCandidate = mOptions.mAggressiveICE ? true : useCandidate; // force this flag if aggressive mode
      }

      STUNPacketPtr stunPacket = STUNPacket::createRequest(STUNPacket::Method_Binding);
      stunPacket->mOptions = mSTUNPacketOptions;
      stunPacket->mFingerprintIncluded = true;
      stunPacket->mPriorityIncluded = true;
      stunPacket->mPriority = route->mCandidatePair->mLocal->mPriority;
      setRole(stunPacket);
      if ((useCandidate) ||
          (route == mUseCandidateRoute)) {
        stunPacket->mUseCandidateIncluded = true;
      }

      stunPacket->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
      stunPacket->mUsername = mRemoteParameters.mUsernameFragment + ":" + mGatherer->getUsernameFrag();
      stunPacket->mPassword = mRemoteParameters.mPassword;

      fix(stunPacket);

      auto remoteIP = route->mCandidatePair->mRemote->ip();
      auto result = ISTUNRequester::create(IHelper::getServicePoolQueue(), mThisWeak.lock(), remoteIP, stunPacket, STUNPacket::RFC_5245_ICE, pattern);
      ZS_EVENTING_2(
                    x, i, Debug, IceTransportInternalStunRequesterCreate, ol, IceTransport, Info,
                    puid, id, mID,
                    puid, stunRequesterId, ((bool)result) ? result->getID() : 0
                    );

      route->trace(__func__, "create binding request");
      return result;
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
        stunPacket->mUsername = request->mUsername;
        stunPacket->mPassword = mGatherer->getPassword();
      }

      fix(stunPacket);

      route->trace(__func__, "create binding response");
      stunPacket->trace(__func__);

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
      stun->mLogObject = "ortc::ICETransport";
      stun->mLogObjectID = mID;
    }

    //-------------------------------------------------------------------------
    void ICETransport::sendPacket(
                                  RouterRoutePtr routerRoute,
                                  STUNPacketPtr packet
                                  )
    {
      if (!mGatherer) {
        ZS_LOG_WARNING(Detail, log("cannot respond as no gatherer is attached") + packet->toDebug())
        return;
      }

      SecureByteBlockPtr packetized = packet->packetize(STUNPacket::RFC_5245_ICE);
      ZS_EVENTING_4(
                    x, i, Trace, IceTransportSendStunPacket, ol, IceTransport, Send,
                    puid, id, mID,
                    puid, iceGathererId, mGatherer->getID(),
                    buffer, packet, packetized->BytePtr(),
                    size, size, packetized->SizeInBytes()
                    );
      packet->trace(__func__);
      mGatherer->sendPacket(*this, routerRoute, packetized->BytePtr(), packetized->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    bool ICETransport::handleSwitchRolesAndConflict(
                                                    RouterRoutePtr routerRoute,
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
        auto response = createErrorResponse(packet, STUNPacket::ErrorCode_RoleConflict);
        response->mMappedAddress = routerRoute->mRemoteIP;
        setRole(response);

        routerRoute->trace(__func__, "role conflict detected");
        response->trace(__func__);

        ZS_LOG_WARNING(Detail, log("returning conflict error to remote party") + routerRoute->toDebug() + response->toDebug())
        sendPacket(routerRoute, response);
        return true;
      }

    switch_roles:
      {
        ZS_LOG_WARNING(Detail, log("ice role conflict detected (switching roles)") + mOptions.toDebug() + packet->toDebug() + ZS_PARAM("conflict resolver", mConflictResolver));

        mOptions.mRole = (IICETypes::Role_Controlling == mOptions.mRole ? IICETypes::Role_Controlled : IICETypes::Role_Controlling);

        ZS_EVENTING_3(
                      x, w, Trace, IceTransportInternalRoleConflictDetectedEvent, ol, IceTransport, InternalEvent,
                      puid, id, mID,
                      string, role, IICETypes::toString(mOptions.mRole),
                      string, reason, "conflict with remote party role (thus will switch role)"
                      );

        mOptionsHash = mOptions.hash();
        pruneAllCandidatePairs(true);
        wakeUp();

        ZS_LOG_WARNING(Debug, log("will retry conflicting packet again after recomputing candidate pairings") + routerRoute->toDebug() + packet->toDebug())
        IICETransportAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyPacketRetried(routerRoute->mLocalCandidate, routerRoute->mRemoteIP, packet);
      }
      return true;
    }

    //-------------------------------------------------------------------------
    ICETransport::RoutePtr ICETransport::findOrCreateMissingRoute(
                                                                  RouterRoutePtr routerRoute,
                                                                  STUNPacketPtr packet
                                                                  )
    {
      RoutePtr route;

      // scope: first check if route already exists
      {
        auto found = mGathererRoutes.find(routerRoute->mID);

        if (found == mGathererRoutes.end()) goto existing_route_not_found;

        route = (*found).second;
        ZS_LOG_TRACE(log("found existing route to use") + route->toDebug())
        return route;
      }

    existing_route_not_found:
      {
        // this route probably does not exist
        route = findRoute(routerRoute->mLocalCandidate, routerRoute->mRemoteIP);
        if (route) {
          ZS_LOG_TRACE(log("adding gatherer route mapping") + route->toDebug() + routerRoute->toDebug())

          removeGathererRoute(route);

          // add to gathering routes
          route->mGathererRoute = routerRoute;

          route->trace(__func__, "added gatherer route");

          mGathererRoutes[route->mGathererRoute->mID] = route;
          return route;
        }

        CandidatePtr remoteCandidate(make_shared<Candidate>());
        remoteCandidate->mCandidateType = IICETypes::CandidateType_Prflx;
        remoteCandidate->mPriority = packet->mPriority;
        remoteCandidate->mIP = routerRoute->mRemoteIP.string(false);
        remoteCandidate->mPort = routerRoute->mRemoteIP.getPort();
        remoteCandidate->mProtocol = routerRoute->mLocalCandidate->mProtocol;
        if (IICETypes::Protocol_TCP == routerRoute->mLocalCandidate->mProtocol) {
          switch (routerRoute->mLocalCandidate->mTCPType) {
            case IICETypes::TCPCandidateType_Active:  remoteCandidate->mTCPType = IICETypes::TCPCandidateType_Passive; break;
            case IICETypes::TCPCandidateType_Passive: remoteCandidate->mTCPType = IICETypes::TCPCandidateType_Active; break;
            case IICETypes::TCPCandidateType_SO:      remoteCandidate->mTCPType = IICETypes::TCPCandidateType_SO; break;
          }
        }

        RoutePtr closestRoute = findClosestRoute(routerRoute->mLocalCandidate, routerRoute->mRemoteIP, packet->mPriority);
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
        route = make_shared<Route>(mRouteStateTracker);
        route->mCandidatePair = make_shared<CandidatePair>();
        route->mCandidatePair->mLocal = make_shared<Candidate>(*(routerRoute->mLocalCandidate));
        route->mCandidatePair->mRemote = remoteCandidate;
        route->mCandidatePairHash = route->mCandidatePair->hash();

        route->trace("added missing route (because of incoming stun packet)");
        packet->trace(__func__);

        // add as legal routes
        mLegalRoutes[route->mCandidatePairHash] = route;

        // add to gathering routes
        route->mGathererRoute = routerRoute;
        mGathererRoutes[route->mGathererRoute->mID] = route;
        
        setPending(route);  // this is now a pending candidate
      }
      
      return route;
    }

    //-------------------------------------------------------------------------
    void ICETransport::handlePassThroughSTUNPacket(
                                                   RouterRoutePtr routerRoute,
                                                   STUNPacketPtr packet
                                                   )
    {
      UseSecureTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);
        auto found = mGathererRoutes.find(routerRoute->mID);
        if (found == mGathererRoutes.end()) {
          ZS_LOG_WARNING(Detail, log("route was not found") + routerRoute->toDebug() + packet->toDebug())
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

        transport = mSecureTransport.lock();
        goto forward_attached_listener;
      }

    forward_attached_listener:
      {
        if (!transport) {
          ZS_LOG_WARNING(Debug, log("no secure transport attached (thus stun packet is being discarded)"))
          return;
        }

        ZS_EVENTING_2(
                      x, i, Trace, IceTransportDeliveringIncomingStunPacketToSecureTransport, ol, IceTransport, Deliver,
                      puid, id, mID,
                      puid, secureTransportId, transport->getID()
                      );

        packet->trace(__func__);
        transport->handleReceivedSTUNPacket(mComponent, packet);
      }
    }

    //-------------------------------------------------------------------------
    bool ICETransport::getNeedsMoreCandidates() const
    {
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Debug, log("do not need more candidate if shutting down"))
        return false;
      }

      if (isComplete()) {
        ZS_LOG_TRACE(log("no need for more candidates as transport is complete"))
        return false;
      }

      if (isFailed()) {
        ZS_LOG_TRACE(log("no need for more candidates as transport is failed"))
        return false;
      }

      ZS_LOG_TRACE(log("more candidates are required as transport is not completed or failed"))
      return true;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport::Route
    #pragma mark

    //-------------------------------------------------------------------------
    ICETransport::Route::Route(RouteStateTrackerPtr tracker) :
      mTracker(tracker)
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!tracker)
      tracker->inState(mState);
    }

    //-------------------------------------------------------------------------
    ICETransport::Route::~Route()
    {
      mTracker->outState(mState);
    }

    //-------------------------------------------------------------------------
    const char *ICETransport::Route::toString(States state)
    {
      switch (state) {
        case State_New:         return "New";
        case State_Pending:     return "Pending";
        case State_Frozen:      return "Frozen";
        case State_InProgress:  return "In progress";
        case State_Succeeded:   return "Succeeded";
        case State_Ignored:     return "Ignored";
        case State_Failed:      return "Failed";
        case State_Blacklisted: return "Blacklisted";
      }
      return "Unknown";
    }

    //-------------------------------------------------------------------------
    ElementPtr ICETransport::Route::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICETransport::Route");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, mCandidatePair ? mCandidatePair->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "candidate pair hash", mCandidatePairHash);

      IHelper::debugAppend(resultEl, "state", toString(mState));

      IHelper::debugAppend(resultEl, "gatherer route", mGathererRoute ? mGathererRoute->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "last received check", mLastReceivedCheck);
      IHelper::debugAppend(resultEl, "last sent check", mLastSentCheck);

      IHelper::debugAppend(resultEl, "last received media", mLastReceivedMedia);
      IHelper::debugAppend(resultEl, "last received response", mLastReceivedResponse);

      IHelper::debugAppend(resultEl, "prune", mPrune);
      IHelper::debugAppend(resultEl, "keep warm", mKeepWarm);
      IHelper::debugAppend(resultEl, "outgoing check", mOutgoingCheck ? mOutgoingCheck->getID() : 0);
      IHelper::debugAppend(resultEl, "keep warm timer", mNextKeepWarm ? mNextKeepWarm->getID() : 0);

      IHelper::debugAppend(resultEl, "last round trip check", mLastRoundTripCheck);
      IHelper::debugAppend(resultEl, "last round trip measurement", mLastRoundTripMeasurement);

      IHelper::debugAppend(resultEl, "frozen promise", (bool)mFrozenPromise);
      IHelper::debugAppend(resultEl, "dependent promises", mDependentPromises.size());

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
      return priority;
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
      return priority;
    }

    //-------------------------------------------------------------------------
    ICETransport::Route::States ICETransport::Route::state() const
    {
      return mState;
    }

    //-------------------------------------------------------------------------
    void ICETransport::Route::state(States state)
    {
      if (state == mState) return;

      ZS_LOG_INSANE(log("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mState)))

      mTracker->outState(mState);
      mState = state;
      mTracker->inState(mState);
    }

    //-------------------------------------------------------------------------
    void ICETransport::Route::trace(const char *function, const char *message) const
    {
      ZS_EVENTING_COMPACT_41(
                             x, i, Trace, IceTransportRouteTrace, ol, IceTransport, Info,
                             puid/routeId, mID,
                             string/callingMethod, function,
                             string/message, message,
                             puid/outerObjectId, ((bool)mTracker) ? mTracker->mOuterObjectID : static_cast<PUID>(0),
                             string/localCandidatePairHash, mCandidatePairHash,
                             string/localInterfaceType, mCandidatePair->mLocal->mInterfaceType,
                             string/localFoundation, mCandidatePair->mLocal->mFoundation,
                             dword/localPriority, mCandidatePair->mLocal->mPriority,
                             dword/localUnfreezePriority, mCandidatePair->mLocal->mUnfreezePriority,
                             string/localProtocol, IICETypes::toString(mCandidatePair->mLocal->mProtocol),
                             string/localIp, mCandidatePair->mLocal->mIP,
                             word/localPort, mCandidatePair->mLocal->mPort,
                             string/localCandidateType, IICETypes::toString(mCandidatePair->mLocal->mCandidateType),
                             string/localTcpTypes, IICETypes::toString(mCandidatePair->mLocal->mTCPType),
                             string/localRelatedAddress, mCandidatePair->mLocal->mRelatedAddress,
                             word/localRelatedPort, mCandidatePair->mLocal->mRelatedPort,
                             string/remoteInterfaceType, mCandidatePair->mRemote->mInterfaceType,
                             string/remoteFoundation, mCandidatePair->mRemote->mFoundation,
                             dword/remotePriority, mCandidatePair->mRemote->mPriority,
                             dword/remoteUnfreezePriority, mCandidatePair->mRemote->mUnfreezePriority,
                             string/remoteProtocol, IICETypes::toString(mCandidatePair->mRemote->mProtocol),
                             string/remoteIp, mCandidatePair->mRemote->mIP,
                             word/remotePort, mCandidatePair->mRemote->mPort,
                             string/remoteCandidateType, IICETypes::toString(mCandidatePair->mRemote->mCandidateType),
                             string/remoteTcpType, IICETypes::toString(mCandidatePair->mRemote->mTCPType),
                             string/remoteRelatedAddress, mCandidatePair->mRemote->mRelatedAddress,
                             word/remoteRelatedPort, mCandidatePair->mRemote->mRelatedPort,
                             puid/gathererRouteId, ((bool)mGathererRoute) ? mGathererRoute->mID : static_cast<PUID>(0),
                             qword/pendingPriority, mPendingPriority,
                             duration/lastReceivedCheck, zsLib::timeSinceEpoch<Milliseconds>(mLastReceivedCheck).count(),
                             duration/lastSentCheck, zsLib::timeSinceEpoch<Milliseconds>(mLastSentCheck).count(),
                             duration/lastReceivedMedia, zsLib::timeSinceEpoch<Milliseconds>(mLastReceivedMedia).count(),
                             duration/lastReceivedResponse, zsLib::timeSinceEpoch<Milliseconds>(mLastReceivedResponse).count(),
                             bool/prune, mPrune,
                             bool/keepWarm, mKeepWarm,
                             puid/outgoingCheckStunRequeter, ((bool)mOutgoingCheck) ? mOutgoingCheck->getID() : static_cast<PUID>(0),
                             puid/nextKeepWarmStunRequester, ((bool)mNextKeepWarm) ? mNextKeepWarm->getID() : static_cast<PUID>(0),
                             bool/frozenPromise, (bool)mFrozenPromise,
                             size_t/totalDependentPromises, mDependentPromises.size(),
                             duration/lastRecievedCheck, zsLib::timeSinceEpoch<Milliseconds>(mLastReceivedCheck).count(),
                             duration/lastRoundTripMeasurement, mLastRoundTripMeasurement.count()
                             );
    }

    //-------------------------------------------------------------------------
    Log::Params ICETransport::Route::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICETransport::Route");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport::RouteStateTracker
    #pragma mark

    //-------------------------------------------------------------------------
    ICETransport::RouteStateTracker::RouteStateTracker(PUID outerObjectID) :
      mOuterObjectID(outerObjectID)
    {
      mStates[Route::State_New] = 0;
      mStates[Route::State_Pending] = 0;
      mStates[Route::State_Frozen] = 0;
      mStates[Route::State_InProgress] = 0;
      mStates[Route::State_Succeeded] = 0;
      mStates[Route::State_Failed] = 0;
      mStates[Route::State_Blacklisted] = 0;
    }

    //-------------------------------------------------------------------------
    ElementPtr ICETransport::RouteStateTracker::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICETransport::RouteStateTracker");

      IHelper::debugAppend(resultEl, "new", mStates[Route::State_New]);
      IHelper::debugAppend(resultEl, "pending", mStates[Route::State_Pending]);
      IHelper::debugAppend(resultEl, "frozen", mStates[Route::State_Frozen]);
      IHelper::debugAppend(resultEl, "in progress", mStates[Route::State_InProgress]);
      IHelper::debugAppend(resultEl, "succeeded", mStates[Route::State_Succeeded]);
      IHelper::debugAppend(resultEl, "ignored", mStates[Route::State_Ignored]);
      IHelper::debugAppend(resultEl, "failed", mStates[Route::State_Failed]);
      IHelper::debugAppend(resultEl, "blacklisted", mStates[Route::State_Blacklisted]);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void ICETransport::RouteStateTracker::inState(States state)
    {
      ++(mStates[state]);
      ZS_EVENTING_3(
                    x, i, Trace, IceTransportRouteStateTrackerStateCountChange, ol, IceTransport, Info,
                    puid, transportObjectId, mOuterObjectID,
                    string, state, Route::toString(state),
                    size_t, value, mStates[state]
                    );
    }

    //-------------------------------------------------------------------------
    void ICETransport::RouteStateTracker::outState(States state)
    {
      --(mStates[state]);
      ZS_EVENTING_3(
                    x, i, Trace, IceTransportRouteStateTrackerStateCountChange, ol, IceTransport, Info,
                    puid, transportObjectId, mOuterObjectID,
                    string, state, Route::toString(state),
                    size_t, value, mStates[state]
                    );
    }

    //-------------------------------------------------------------------------
    size_t ICETransport::RouteStateTracker::count(States state)
    {
      return (mStates[state]);
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
  #pragma mark (helpers)
  #pragma mark

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IICETransportTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes::States
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
      case State_Failed:        return "failed";
      case State_Closed:        return "closed";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IICETransportTypes::States IICETransportTypes::toState(const char *state) throw (InvalidParameters)
  {
    String stateStr(state);
    for (IICETransportTypes::States index = IICETransportTypes::State_First; index <= IICETransportTypes::State_Last; index = static_cast<IICETransportTypes::States>(static_cast<std::underlying_type<IICETransportTypes::States>::type>(index) + 1)) {
      if (0 == stateStr.compareNoCase(IICETransportTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + stateStr)
    return IICETransportTypes::State_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes::CandidatePair
  #pragma mark


  //---------------------------------------------------------------------------
  IICETransportTypes::CandidatePair::CandidatePair(ElementPtr elem)
  {
    if (!elem) return;

//    CandidatePtr mLocal;
//    CandidatePtr mRemote;

    {
      ElementPtr local = elem->findFirstChildElement("local");
      if (local) {
        mLocal = make_shared<Candidate>(local);
      }
    }

    {
      ElementPtr remote = elem->findFirstChildElement("local");
      if (remote) {
        mRemote = make_shared<Candidate>(remote);
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransportTypes::CandidatePair::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mLocal) {
      elem->adoptAsLastChild(mLocal->createElement("local"));
    }
    if (mRemote) {
      elem->adoptAsLastChild(mRemote->createElement("remote"));
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransportTypes::CandidatePair::toDebug() const
  {
    return createElement("ortc::IICETransport::CandidatePair");
  }

  //---------------------------------------------------------------------------
  String IICETransportTypes::CandidatePair::hash(bool includePriorities) const
  {
    auto hasher = IHasher::sha1();

    String localHash = mLocal ? mLocal->hash(includePriorities) : String();
    String remoteHash = mRemote ? mRemote->hash(includePriorities) : String();

    hasher->update("candidate-pair:");
    hasher->update(localHash);
    hasher->update(":");
    hasher->update(remoteHash);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes::Options
  #pragma mark


  //---------------------------------------------------------------------------
  IICETransportTypes::Options::Options(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IICETransportTypes::Options", "aggressiveIce", mAggressiveICE);

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("role"));
      if (str.hasData()) {
        try {
          mRole = toRole(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("role not valid") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransportTypes::Options::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "aggressiveIce", mAggressiveICE);
    IHelper::adoptElementValue(elem, "role", IICETypes::toString(mRole), false);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransportTypes::Options::toDebug() const
  {
    return createElement("ortc::IICETransport::Options");
  }

  //---------------------------------------------------------------------------
  String IICETransportTypes::Options::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IICETransport::Options:");
    hasher->update(mAggressiveICE);
    hasher->update(":");
    hasher->update(IICETypes::toString(mRole));

    return hasher->finalizeAsString();
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
  IICETransportPtr IICETransport::convert(IRTCPTransportPtr object)
  {
    return internal::ICETransport::convert(object);
  }

  //---------------------------------------------------------------------------
  IICETransportPtr IICETransport::create(
                                         IICETransportDelegatePtr delegate,
                                         IICEGathererPtr gatherer
                                         )
  {
    return internal::IICETransportFactory::singleton().create(delegate, gatherer);
  }

} // namespace ortc
