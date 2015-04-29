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

#pragma once

#include <ortc/internal/types.h>

#include <ortc/IICETransport.h>
#include <ortc/IICEGatherer.h>

#include <openpeer/services/ISTUNRequester.h>
#include <openpeer/services/IWakeDelegate.h>

#include <zsLib/Timer.h>

#define ORTC_SETTING_ICE_TRANSPORT_MAX_CANDIDATE_PAIRS_TO_TEST  "ortc/ice-transport/max-candidate-pairs-to-test"

#define ORTC_SETTING_ICE_TRANSPORT_ACTIVATION_TIMER_IN_MILLISECONDS  "ortc/ice-transport/activation-timer-in-milliseconds"

#define ORTC_SETTING_ICE_TRANSPORT_NO_PACKETS_RECEVIED_RECHECK_CANDIDATES_IN_SECONDS  "ortc/ice-transport/no-packets-received-recheck-candidates-in-seconds"

#define ORTC_SETTING_ICE_TRANSPORT_EXPIRE_ROUTE_IN_SECONDS "ortc/ice-transport/expire-route-not-active-in-seconds"

#define ORTC_SETTING_ICE_TRANSPORT_BLACKLIST_AFTER_CONSENT_REMOVAL "ortc/ice-transport/blacklist-after-consent-removal"

#define ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_BASE_IN_MILLISECONDS "ortc/ice-transport/keep-warm-time-base-in-milliseconds"
#define ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_RANDOMIZED_ADD_TIME_IN_MILLISECONDS "ortc/ice-transport/keep-warm-time-randomized-add-time-in-milliseconds"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSettings)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICEGatherer)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICETransportContoller)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForRTPTransport)

    ZS_DECLARE_INTERACTION_PROXY(ITransportAsyncDelegate)

    ZS_DECLARE_INTERACTION_PTR(IICEGathererForICETransport)
    ZS_DECLARE_INTERACTION_PTR(IICETransportControllerForICETransport)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    interaction IICETransportForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IICETransportForSettings() {}
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForICEGatherer
    #pragma mark

    interaction IICETransportForICEGatherer
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, ForICEGatherer)

      virtual PUID getID() const = 0;

      virtual void notifyRouteAdded(
                                    PUID routeID,
                                    IICETypes::CandidatePtr localCandidate,
                                    const IPAddress &fromIP
                                    ) = 0;
      virtual void notifyRouteRemoved(PUID routeID) = 0;

      virtual void notifyPacket(
                                PUID routeID,
                                STUNPacketPtr packet
                                ) = 0;
      virtual void notifyPacket(
                                PUID routeID,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) = 0;

      virtual bool needsMoreCandidates() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForICETransportContoller
    #pragma mark

    interaction IICETransportForICETransportContoller
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICETransportContoller, ForTransportContoller)

      static ElementPtr toDebug(ForTransportContollerPtr transport);

      virtual PUID getID() const = 0;

      virtual void notifyControllerAttached(ICETransportControllerPtr controller) = 0;
      virtual void notifyControllerDetached() = 0;

      virtual bool hasCandidatePairFoundation(
                                              const String &localFoundation,
                                              const String &remoteFoundation,
                                              PromisePtr promise
                                              ) = 0;

      virtual IICETypes::Components component() const = 0;
      virtual IICETransport::States state() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForRTPTransport
    #pragma mark

    interaction IICETransportForRTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForRTPTransport, ForRTPTransport)

      virtual PUID getID() const = 0;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) = 0;
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportAsyncDelegate
    #pragma mark

    interaction ITransportAsyncDelegate
    {
      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport
    #pragma mark

    class ICETransport : public Noop,
                         public MessageQueueAssociator,
                         public SharedRecursiveLock,
                         public IICETransport,
                         public IICETransportForSettings,
                         public IICETransportForICEGatherer,
                         public IICETransportForICETransportContoller,
                         public IICETransportForRTPTransport,
                         public ITransportAsyncDelegate,
                         public IWakeDelegate,
                         public zsLib::ITimerDelegate,
                         public IICEGathererDelegate,
                         public openpeer::services::ISTUNRequesterDelegate
    {
    public:
      friend interaction IICETransport;
      friend interaction IICETransportFactory;
      friend interaction IICETransportForSettings;
      friend interaction IICETransportForICEGatherer;
      friend interaction IICETransportForICETransportContoller;
      friend interaction IICETransportForRTPTransport;

      ZS_DECLARE_STRUCT_PTR(Route)

      ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISTUNRequester, ISTUNRequester)
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, UseICEGatherer)
      ZS_DECLARE_TYPEDEF_PTR(IICETransportControllerForICETransport, UseICETransportController)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateComplete, CandidateComplete)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList)

      typedef String Hash;
      typedef std::map<Hash, CandidatePtr> CandidateMap;

      typedef std::map<Hash, RoutePtr> RouteMap;

      typedef PUID RouteID;
      typedef QWORD SortPriority;
      typedef std::map<SortPriority, RoutePtr> SortedRouteMap;

      typedef std::map<RouteID, RoutePtr> RouteIDMap;
      typedef std::map<ISTUNRequesterPtr, RoutePtr> STUNCheckMap;
      typedef std::map<TimerPtr, RoutePtr> TimerRouteMap;
      typedef std::map<PromisePtr, RoutePtr> PromiseRouteMap;

      typedef std::list<PromisePtr> PromiseList;

    protected:
      ICETransport(
                   IMessageQueuePtr queue,
                   IICETransportDelegatePtr delegate,
                   UseICEGathererPtr gatherer
                   );

      ICETransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create()) {}

      void init();

    public:
      virtual ~ICETransport();

      static ICETransportPtr convert(IICETransportPtr object);
      static ICETransportPtr convert(ForSettingsPtr object);
      static ICETransportPtr convert(ForICEGathererPtr object);
      static ICETransportPtr convert(ForTransportContollerPtr object);
      static ICETransportPtr convert(ForRTPTransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransport
      #pragma mark

      static ElementPtr toDebug(ICETransportPtr transport);

      static ICETransportPtr create(
                                    IICETransportDelegatePtr delegate,
                                    IICEGathererPtr gatherer
                                    );

      virtual PUID getID() const override {return mID;}

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) override;

      virtual IICEGathererPtr iceGatherer() const override;

      virtual Roles role() const override;
      virtual Components component() const override;
      virtual States state() const override;

      virtual CandidateListPtr getRemoteCandidates() const override;

      virtual CandidatePairPtr getNominatedCandidatePair() const override;

      virtual void start(
                         IICEGathererPtr gatherer,
                         Parameters remoteParameters,
                         Optional<Options> options = Optional<Options>()
                         ) throw (InvalidParameters) override;

      virtual void stop() override;

      virtual ParametersPtr getRemoteParameters() const override;

      virtual IICETransportPtr createAssociatedTransport() throw (InvalidStateError) override;

      virtual void addRemoteCandidate(const GatherCandidate &remoteCandidate) override;
      virtual void setRemoteCandidates(const CandidateList &remoteCandidates) override;
      virtual void removeRemoteCandidate(const GatherCandidate &remoteCandidate) override;

      virtual void keepWarm(
                            const CandidatePair &candidatePair,
                            bool keepWarm = true
                            ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForICEGatherer
      #pragma mark

      virtual void notifyRouteAdded(
                                    PUID routeID,
                                    IICETypes::CandidatePtr localCandidate,
                                    const IPAddress &fromIP
                                    ) override;
      virtual void notifyRouteRemoved(PUID routeID) override;

      virtual void notifyPacket(
                                PUID routeID,
                                STUNPacketPtr packet
                                ) override;
      virtual void notifyPacket(
                                PUID routeID,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) override;

      virtual bool needsMoreCandidates() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForICETransportContoller
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      virtual void notifyControllerAttached(ICETransportControllerPtr controller) override;
      virtual void notifyControllerDetached() override;

      virtual bool hasCandidatePairFoundation(
                                              const String &localFoundation,
                                              const String &remoteFoundation,
                                              PromisePtr promise
                                              ) override;

      // (duplicate) virtual IICETypes::Components component() const = 0;
      // (duplicate) virtual IICETransport::States state() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForRTPTransport
      #pragma mark

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => ITransportAsyncDelegate
      #pragma mark

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICEGathererDelegate
      #pragma mark

      virtual void onICEGathererStateChanged(
                                             IICEGathererPtr gatherer,
                                             IICEGatherer::States state
                                             ) override;

      virtual void onICEGathererLocalCandidate(
                                               IICEGathererPtr gatherer,
                                               CandidatePtr candidate
                                               ) override;

      virtual void onICEGathererLocalCandidateComplete(
                                                       IICEGathererPtr gatherer,
                                                       CandidateCompletePtr candidate
                                                       ) override;

      virtual void onICEGathererLocalCandidateGone(
                                                   IICEGathererPtr gatherer,
                                                   CandidatePtr candidate
                                                   ) override;

      virtual void onICEGathererError(
                                      IICEGathererPtr gatherer,
                                      ErrorCode errorCode,
                                      String errorReason
                                      ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => ISTUNRequesterDelegate
      #pragma mark

      virtual void onSTUNRequesterSendPacket(
                                             ISTUNRequesterPtr requester,
                                             IPAddress destination,
                                             SecureByteBlockPtr packet
                                             ) override;

      virtual bool handleSTUNRequesterResponse(
                                               ISTUNRequesterPtr requester,
                                               IPAddress fromIPAddress,
                                               STUNPacketPtr response
                                               ) override;

      virtual void onSTUNRequesterTimedOut(ISTUNRequesterPtr requester) override;

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport::Route
      #pragma mark

      struct Route
      {
        enum States
        {
          State_Pending,
          State_Frozen,
          State_InProgress,
          State_Succeeded,
          State_Failed,
          State_Blacklisted,
        };
        static const char *toString(States state);

        CandidatePairPtr mCandidatePair;
        String mCandidatePairHash;

        States mState {State_Pending};

        PUID mGathererRouteID {0};
        QWORD mPendingPriority {};

        Time mLastReceivedCheck;
        Time mLastSentCheck;

        Time mLastReceivedMedia;
        Time mLastReceivedResponse;

        bool mPrune {false};
        bool mKeepWarm {false};
        ISTUNRequesterPtr mOutgoingCheck;
        TimerPtr mNextKeepWarm;

        PromisePtr mFrozenPromise;
        PromiseList mDependentPromises;

        ElementPtr toDebug() const;

        QWORD getPreference(bool localIsControlling) const;
        QWORD getActivationPriority(
                                    bool localIsControlling,
                                    bool useUnfreezePreference
                                    ) const;

        bool isPending() const {return State_Pending == mState;}
        bool isFrozen() const {return State_Frozen == mState;}
        bool isInProgress() const {return State_InProgress == mState;}
        bool isSucceeded() const {return State_Succeeded == mState;}
        bool isFailed() const {return State_Failed == mState;}
        bool isBlacklisted() const {return State_Blacklisted == mState;}
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isConnected() const;
      bool isComplete() const;
      bool isDisconnected() const;
      bool isShuttingDown() const;
      bool isShutdown() const;
      bool isContinousGathering() const;

      void step();
      bool stepCalculateLegalPairs();
      bool stepPendingActivation();
      bool stepActivationTimer();
      bool stepLastReceivedPacketTimer();
      bool stepExpireRouteTimer();
      bool stepPickRoute();
      bool stepUseCandidate();
      bool stepDewarmRoutes();
      bool stepKeepWarmRoutes();

      void wakeUp();
      void cancel();

      void setState(IICETransportTypes::States state);
      void setError(WORD error, const char *reason = NULL);

      void shutdown(RoutePtr route);

      void pruneAllCandidatePairs(bool keepActiveAlive);
      CandidatePairPtr cloneCandidatePair(RoutePtr route);

      void setPending(RoutePtr route);
      void setFrozen(
                     RoutePtr route,
                     PromisePtr promise
                     );
      void setInProgress(RoutePtr route);
      void setSucceeded(RoutePtr route);
      void setFailed(RoutePtr route);
      void setBlacklisted(RoutePtr route);

      void updateAfterPacket(RoutePtr route);

      void removeLegal(RoutePtr route);
      void removeFrozen(RoutePtr route);
      void removeFrozenDependencies(
                                    RoutePtr route,
                                    bool succeeded
                                    );
      void removeActive(RoutePtr route);
      void removePendingActivation(RoutePtr route);
      void removeOutgoingCheck(RoutePtr route);
      void removeGathererRoute(RoutePtr route);
      void removeKeepWarmTimer(RoutePtr route);
      void removeWarm(RoutePtr route);

      ISTUNRequesterPtr createBindRequest(
                                          RoutePtr route,
                                          bool useCandidate = false
                                          ) const;
      void fix(STUNPacketPtr stun) const;

    private:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => (data)
      #pragma mark
      
      AutoPUID mID;

      ICETransportWeakPtr mThisWeak;
      ICETransportPtr mGracefulShutdownReference;

      IICETransportDelegateSubscriptions mSubscriptions;
      IICETransportSubscriptionPtr mDefaultSubscription;

      Components mComponent {Component_RTP};

      IICETransportTypes::States mCurrentState {IICETransportTypes::State_New};

      WORD mLastError {};
      String mLastErrorReason;

      UseICEGathererPtr mGatherer;
      IICEGathererSubscriptionPtr mGathererSubscription;

      UseICETransportControllerWeakPtr mTransportController;

      ICETransportPtr mRTPTransport;
      ICETransportWeakPtr mRTCPTransport;

      std::atomic<bool> mWakeUp {false};

      String mOptionsHash;
      Options mOptions;
      QWORD mConflictResolver {};

      String mRemoteParametersHash;
      Parameters mRemoteParameters;

      String mLocalCandidatesHash;
      CandidateMap mLocalCandidates;
      bool mEndOfLocalCandidates {false};

      String mRemoteCandidatesHash;
      CandidateMap mRemoteCandidates;
      bool mEndOfRemoteCandidates {false};

      String mComputedPairsHash;
      RouteMap mLegalRoutes;

      TimerPtr mActivationTimer;

      String mPendingActivationPairsHash;
      SortedRouteMap mPendingActivation;

      PromiseRouteMap mFrozen;

      RoutePtr mActiveRoute;

      bool mWarmRoutesChanged {false};
      RouteMap mWarmRoutes;   // these are reported as available (and gone when removed)

      RouteIDMap mGathererRoutes;

      STUNCheckMap mOutgoingChecks;
      TimerRouteMap mNextKeepWarmTimers;

      RoutePtr mUseCandidateRoute;
      ISTUNRequesterPtr mUseCandidateRequest;
      Time mLastReceivedUseCandidate;

      Time mLastReceivedPacket;
      TimerPtr mLastReceivedPacketTimer;
      Seconds mNoPacketsReceivedRecheckTime;

      Seconds mExpireRouteTime;
      TimerPtr mExpireRouteTimer;
      bool mBlacklistConsent {false};

      Milliseconds mKeepWarmTimeBase;
      Milliseconds mKeepWarmTimeRandomizedAddTime;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportFactory
    #pragma mark

    interaction IICETransportFactory
    {
      static IICETransportFactory &singleton();

      virtual ICETransportPtr create(
                                     IICETransportDelegatePtr delegate,
                                     IICEGathererPtr gatherer
                                     );
    };

    class ICETransportFactory : public IFactory<IICETransportFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ITransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD_1(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_END()
