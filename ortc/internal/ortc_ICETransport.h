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

#include <ortc/internal/ortc_ICEGathererRouter.h>

#include <ortc/IICETransport.h>
#include <ortc/IICEGatherer.h>

#include <ortc/services/ISTUNRequester.h>
#include <ortc/services/STUNPacket.h>

#include <zsLib/ITimer.h>

#include <queue>

#define ORTC_SETTING_ICE_TRANSPORT_MAX_CANDIDATE_PAIRS_TO_TEST  "ortc/ice-transport/max-candidate-pairs-to-test"

#define ORTC_SETTING_ICE_TRANSPORT_ACTIVATION_TIMER_IN_MILLISECONDS  "ortc/ice-transport/activation-timer-in-milliseconds"

#define ORTC_SETTING_ICE_TRANSPORT_NO_PACKETS_RECEVIED_RECHECK_CANDIDATES_IN_SECONDS  "ortc/ice-transport/no-packets-received-recheck-candidates-in-seconds"

#define ORTC_SETTING_ICE_TRANSPORT_EXPIRE_ROUTE_IN_SECONDS "ortc/ice-transport/expire-route-not-active-in-seconds"

#define ORTC_SETTING_ICE_TRANSPORT_BLACKLIST_AFTER_CONSENT_REMOVAL "ortc/ice-transport/blacklist-after-consent-removal"

#define ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_BASE_IN_MILLISECONDS "ortc/ice-transport/keep-warm-time-base-in-milliseconds"
#define ORTC_SETTING_ICE_TRANSPORT_KEEP_WARM_TIME_RANDOMIZED_ADD_TIME_IN_MILLISECONDS "ortc/ice-transport/keep-warm-time-randomized-add-time-in-milliseconds"

#define ORTC_SETTING_ICE_TRANSPORT_TEST_CANDIDATE_PAIRS_OF_LOWER_PREFERENCE "ortc/ice-transport/test-candidate-pairs-of-lower-preference"

#define ORTC_SETTING_ICE_TRANSPORT_MAX_BUFFERED_FOR_SECURE_TRANSPORT "ortc/ice-transport/max-buffered-packets-for-secure-transport"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSettings)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICEGatherer)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICETransportContoller)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForDataTransport)


    ZS_DECLARE_INTERACTION_PROXY(IICETransportAsyncDelegate)

    ZS_DECLARE_INTERACTION_PTR(IICEGathererForICETransport)
    ZS_DECLARE_INTERACTION_PTR(IICETransportControllerForICETransport)


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

      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute)

      virtual PUID getID() const = 0;

      virtual ForICEGathererPtr getForGatherer() const = 0;

      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
                                STUNPacketPtr packet
                                ) = 0;
      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
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
      virtual void notifyControllerDetached(ICETransportController &controller) = 0;

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
    #pragma mark IICETransportForSecureTransport
    #pragma mark

    interaction IICETransportForSecureTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, ForSecureTransport)

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForICETransport, UseSecureTransport)

      virtual PUID getID() const = 0;

      virtual void notifyAttached(
                                  PUID secureTransportID,
                                  UseSecureTransportPtr transport
                                  ) = 0;
      virtual void notifyDetached(PUID secureTransportID) = 0;

      virtual IICETypes::Components component() const = 0;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) = 0;

      virtual IICETransport::States state() const = 0;

      virtual IICETypes::Roles getRole() const = 0;

      virtual ICETransportPtr getRTPTransport() const = 0;
      virtual ICETransportPtr getRTCPTransport() const = 0;

      virtual UseSecureTransportPtr getSecureTransport() const = 0;

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
    #pragma mark IICETransportForDataTransport
    #pragma mark

    interaction IICETransportForDataTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDataTransport, ForDataTransport)

      virtual PUID getID() const = 0;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) = 0;

      virtual IICETransport::States state() const = 0;
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportAsyncDelegate
    #pragma mark

    interaction IICETransportAsyncDelegate
    {
      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) = 0;
      virtual void onNotifyPacketRetried(
                                         IICETypes::CandidatePtr localCandidate,
                                         IPAddress remoteIP,
                                         STUNPacketPtr stunPacket
                                         ) = 0;
      virtual void onWarmRoutesChanged() = 0;

      virtual void onNotifyAttached(PUID secureTransportID) = 0;
      virtual void onNotifyDetached(PUID secureTransportID) = 0;

      virtual void onDeliverPendingPackets() = 0;
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
                         public IICETransportForICEGatherer,
                         public IICETransportForICETransportContoller,
                         public IICETransportForSecureTransport,
                         public IICETransportForDataTransport,
                         public IICETransportAsyncDelegate,
                         public IWakeDelegate,
                         public zsLib::ITimerDelegate,
                         public zsLib::IPromiseSettledDelegate,
                         public IICEGathererDelegate,
                         public ortc::services::ISTUNRequesterDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IICETransport;
      friend interaction IICETransportFactory;
      friend interaction IICETransportForICEGatherer;
      friend interaction IICETransportForICETransportContoller;
      friend interaction IICETransportForSecureTransport;
      friend interaction IICETransportForDataTransport;

      ZS_DECLARE_STRUCT_PTR(RouteStateTracker)
      ZS_DECLARE_STRUCT_PTR(Route)
      ZS_DECLARE_STRUCT_PTR(ReasonNoMoreRelationship)

      ZS_DECLARE_TYPEDEF_PTR(ortc::services::ISTUNRequester, ISTUNRequester)
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, UseICEGatherer)
      ZS_DECLARE_TYPEDEF_PTR(IICETransportControllerForICETransport, UseICETransportController)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateComplete, CandidateComplete)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForICETransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute)

      typedef String Hash;
      typedef std::map<Hash, CandidatePtr> CandidateMap;

      typedef std::map<Hash, RoutePtr> RouteMap;

      typedef PUID RouteID;
      typedef QWORD SortPriority;
      typedef std::map<SortPriority, RoutePtr> SortedRouteMap;

      typedef std::map<RouteID, RoutePtr> RouteIDMap;
      typedef std::map<ISTUNRequesterPtr, RoutePtr> STUNCheckMap;
      typedef std::map<ITimerPtr, RoutePtr> TimerRouteMap;
      typedef std::map<PromisePtr, RoutePtr> PromiseRouteMap;

      typedef std::list<PromisePtr> PromiseList;

      typedef String LocalFoundation;
      typedef String RemoteFoundation;
      typedef std::pair<LocalFoundation, RemoteFoundation> LocalRemoteFoundationPair;
      typedef std::map<LocalRemoteFoundationPair, RouteMap> FoundationRouteMap;

      typedef CandidatePtr LocalCandidatePtr;
      typedef IPAddress FromIP;
      typedef std::pair<LocalCandidatePtr, FromIP> LocalCandidateFromIPPair;

      typedef std::map<RouteID, LocalCandidateFromIPPair> RouteIDLocalCandidateFromIPMap;

      typedef std::queue<SecureByteBlockPtr> PacketQueue;

    public:
      ICETransport(
                   const make_private &,
                   IMessageQueuePtr queue,
                   IICETransportDelegatePtr delegate,
                   UseICEGathererPtr gatherer
                   );

    protected:
      ICETransport(
                   Noop,
                   IMessageQueuePtr queue = IMessageQueuePtr(),
                   const SharedRecursiveLock &lock = SharedRecursiveLock::create()
                   ) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(lock) {}

      void init();

    public:
      virtual ~ICETransport();

      static ICETransportPtr convert(IICETransportPtr object);
      static ICETransportPtr convert(IRTCPTransportPtr object);
      static ICETransportPtr convert(ForICEGathererPtr object);
      static ICETransportPtr convert(ForTransportContollerPtr object);
      static ICETransportPtr convert(ForSecureTransportPtr object);
      static ICETransportPtr convert(ForDataTransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

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

      virtual CandidatePairPtr getSelectedCandidatePair() const override;

      virtual void start(
                         IICEGathererPtr gatherer,
                         const Parameters &remoteParameters,
                         Optional<Options> options = Optional<Options>()
                         ) throw (InvalidParameters) override;

      virtual void stop() override;

      virtual ParametersPtr getRemoteParameters() const override;

      virtual IICETransportPtr createAssociatedTransport(IICETransportDelegatePtr delegate) throw (InvalidStateError) override;

      virtual void addRemoteCandidate(const GatherCandidate &remoteCandidate) throw (InvalidStateError, InvalidParameters) override;
      virtual void setRemoteCandidates(const CandidateList &remoteCandidates) throw (InvalidStateError, InvalidParameters) override;
      virtual void removeRemoteCandidate(const GatherCandidate &remoteCandidate) throw (InvalidStateError, InvalidParameters) override;

      virtual void keepWarm(
                            const CandidatePair &candidatePair,
                            bool keepWarm = true
                            ) throw (InvalidStateError) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForICEGatherer
      #pragma mark

      virtual ForICEGathererPtr getForGatherer() const override;

      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
                                STUNPacketPtr packet
                                ) override;
      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
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
      virtual void notifyControllerDetached(ICETransportController &controller) override;

      virtual bool hasCandidatePairFoundation(
                                              const String &localFoundation,
                                              const String &remoteFoundation,
                                              PromisePtr promise
                                              ) override;

      // (duplicate) virtual IICETypes::Components component() const = 0;
      // (duplicate) virtual IICETransport::States state() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForSecureTransport
      #pragma mark

      // (duplicate) virtual PUID getID() const override;

      virtual void notifyAttached(
                                  PUID secureTransportID,
                                  UseSecureTransportPtr transport
                                  ) override;
      virtual void notifyDetached(PUID secureTransportID) override;

      // (duplicate) virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate);

      // (duplicate) virtual IICETransport::States state() const override;

      virtual IICETypes::Roles getRole() const override;

      virtual ICETransportPtr getRTPTransport() const override;
      virtual ICETransportPtr getRTCPTransport() const override;

      virtual UseSecureTransportPtr getSecureTransport() const override;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForDataTransport
      #pragma mark

      // (duplicate) virtual PUID getID() const override;

      // (duplicate) virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate);

      // (duplciate) virtual IICETransport::States state() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportAsyncDelegate
      #pragma mark

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;
      virtual void onNotifyPacketRetried(
                                         IICETypes::CandidatePtr localCandidate,
                                         IPAddress remoteIP,
                                         STUNPacketPtr stunPacket
                                         ) override;
      virtual void onWarmRoutesChanged() override;

      virtual void onNotifyAttached(PUID secureTransportID) override;
      virtual void onNotifyDetached(PUID secureTransportID) override;

      virtual void onDeliverPendingPackets() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICEGathererDelegate
      #pragma mark

      virtual void onICEGathererStateChange(
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
                                      ErrorEventPtr errorEvent
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
          State_First,

          State_New = State_First,
          State_Pending,
          State_Frozen,
          State_InProgress,
          State_Succeeded,
          State_Ignored,
          State_Failed,
          State_Blacklisted,

          State_Last = State_Blacklisted,
        };
        static const char *toString(States state);

        AutoPUID mID;

        CandidatePairPtr mCandidatePair;
        String mCandidatePairHash;

        RouteStateTrackerPtr mTracker;

        RouterRoutePtr mGathererRoute;

        QWORD mPendingPriority {};

        Time mLastReceivedCheck;
        Time mLastSentCheck;

        Time mLastReceivedMedia;
        Time mLastReceivedResponse;

        bool mPrune {false};
        bool mKeepWarm {false};
        ISTUNRequesterPtr mOutgoingCheck;
        ITimerPtr mNextKeepWarm;

        PromisePtr mFrozenPromise;
        PromiseList mDependentPromises;

        Time mLastRoundTripCheck;
        Microseconds mLastRoundTripMeasurement {};

        Route(RouteStateTrackerPtr tracker);
        ~Route();

        ElementPtr toDebug() const;

        QWORD getPreference(bool localIsControlling) const;
        QWORD getActivationPriority(
                                    bool localIsControlling,
                                    bool useUnfreezePreference
                                    ) const;

        States state() const;
        void state(States state);

        bool isNew() const {return State_New == mState;}
        bool isPending() const {return State_Pending == mState;}
        bool isFrozen() const {return State_Frozen == mState;}
        bool isInProgress() const {return State_InProgress == mState;}
        bool isSucceeded() const {return State_Succeeded == mState;}
        bool isIgnored() const {return State_Ignored == mState;}
        bool isFailed() const {return State_Failed == mState;}
        bool isBlacklisted() const {return State_Blacklisted == mState;}

        void trace(const char *function, const char *message = NULL) const;

      protected:
        Log::Params log(const char *message) const;

        States mState {State_New};
      };

      struct RouteStateTracker
      {
        typedef Route::States States;

        PUID mOuterObjectID {};

        size_t mStates[Route::State_Last+1];

        RouteStateTracker(PUID outerObjectID);

        ElementPtr toDebug() const;

        void inState(States state);
        void outState(States state);

        size_t count(States state);
      };

      struct ReasonNoMoreRelationship : public Any
      {
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isConnected() const;
      bool isComplete() const;
      bool isDisconnected() const;
      bool isFailed() const;
      bool isShuttingDown() const;
      bool isShutdown() const;
      bool isContinousGathering() const;

      void step();
      bool stepCalculateLegalPairs();
      bool stepPendingActivation();
      bool stepActivationTimer();
      bool stepPickRoute();
      bool stepUseCandidate();
      bool stepDewarmRoutes();
      bool stepKeepWarmRoutes();
      bool stepExpireRouteTimer();
      bool stepLastReceivedPacketTimer();
      bool stepSetCurrentState();
      bool stepSetNeedsMoreCandidates();

      void wakeUp();
      void warmRoutesChanged();
      bool hasWarmRoutesChanged();
      void cancel();

      void setState(IICETransportTypes::States state);
      void setError(WORD error, const char *reason = NULL);

      void handleExpireRouteTimer();
      void handleLastReceivedPacket();
      void handleActivationTimer();
      void handleNextKeepWarmTimer(RoutePtr route);

      void forceActive(RoutePtr route);
      void shutdown(RoutePtr route);

      void pruneAllCandidatePairs(bool keepActiveAlive);
      CandidatePairPtr cloneCandidatePair(RoutePtr route) const;

      void setPending(RoutePtr route);
      void setFrozen(
                     RoutePtr route,
                     PromisePtr promise
                     );
      void setInProgress(RoutePtr route);
      void setSucceeded(RoutePtr route);
      void setIgnored(RoutePtr route);
      void setFailed(RoutePtr route);
      void setBlacklisted(RoutePtr route);

      void updateAfterPacket(RoutePtr route);

      bool installGathererRoute(RoutePtr route);

      void installFoundation(RoutePtr route);

      void removeLegal(RoutePtr route);
      void removeFoundation(RoutePtr route);
      void removeFrozen(RoutePtr route);
      void removeFrozenDependencies(
                                    RoutePtr route,
                                    bool succeeded,
                                    AnyPtr reason = AnyPtr()
                                    );
      void removeActive(RoutePtr route);
      void removePendingActivation(RoutePtr route);
      void removeOutgoingCheck(RoutePtr route);
      void removeGathererRoute(RoutePtr route);
      void removeKeepWarmTimer(RoutePtr route);
      void removeWarm(RoutePtr route);

      RoutePtr findRoute(
                         IICETypes::CandidatePtr localCandidate,
                         const IPAddress &remoteIP
                         );
      RoutePtr findClosestRoute(
                                IICETypes::CandidatePtr localCandidate,
                                const IPAddress &remoteIP,
                                DWORD remotePriority
                                );
      Time getLastRemoteActivity(RoutePtr route) const;

      ISTUNRequesterPtr createBindRequest(
                                          RoutePtr route,
                                          bool useCandidate = false,
                                          IBackOffTimerPatternPtr pattern = IBackOffTimerPatternPtr()
                                          ) const;
      STUNPacketPtr createBindResponse(
                                       STUNPacketPtr request,
                                       RoutePtr route
                                       ) const;
      STUNPacketPtr createErrorResponse(
                                        STUNPacketPtr request,
                                        STUNPacket::ErrorCodes error
                                        );
      void setRole(STUNPacketPtr packet) const;
      void fix(STUNPacketPtr stun) const;

      void sendPacket(
                      RouterRoutePtr routerRoute,
                      STUNPacketPtr packet
                      );

      bool handleSwitchRolesAndConflict(
                                        RouterRoutePtr routerRoute,
                                        STUNPacketPtr packet
                                        );

      RoutePtr findOrCreateMissingRoute(
                                        RouterRoutePtr routerRoute,
                                        STUNPacketPtr packet
                                        );

      void handlePassThroughSTUNPacket(                       // do not call within a lock
                                       RouterRoutePtr routerRoute,
                                       STUNPacketPtr packet
                                       );

      bool getNeedsMoreCandidates() const;

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

      std::atomic<bool> mNeedsMoreCandidates {true};

      UseICEGathererPtr mGatherer;
      IICEGathererSubscriptionPtr mGathererSubscription;
      ICEGathererRouterPtr mGathererRouter;

      UseICETransportControllerWeakPtr mTransportController;

      ICETransportWeakPtr mRTPTransport;
      ICETransportPtr mRTCPTransport;

      std::atomic<bool> mWakeUp {false};
      int mWarmRoutesChanged {0};
      bool mForcePickRouteAgain {false};

      String mOptionsHash;
      Options mOptions;
      QWORD mConflictResolver {};

      String mRemoteParametersHash;
      Parameters mRemoteParameters;

      String mLocalCandidatesHash;
      CandidateMap mLocalCandidates;
      bool mLocalCandidatesComplete {false};

      String mRemoteCandidatesHash;
      CandidateMap mRemoteCandidates;
      bool mRemoteCandidatesComplete {false};

      String mComputedPairsHash;
      RouteMap mLegalRoutes;
      FoundationRouteMap mFoundationRoutes;
      RouteStateTrackerPtr mRouteStateTracker;

      ITimerPtr mActivationTimer;
      bool mNextActivationCausesAllRoutesThatReceivedChecksToActivate {false};

      SortedRouteMap mPendingActivation;

      PromiseRouteMap mFrozen;

      RoutePtr mActiveRoute;

      RouteMap mWarmRoutes;   // these are reported as available (and gone when removed)

      RouteIDMap mGathererRoutes;

      STUNCheckMap mOutgoingChecks;
      TimerRouteMap mNextKeepWarmTimers;

      RoutePtr mUseCandidateRoute;
      ISTUNRequesterPtr mUseCandidateRequest;
      Time mLastReceivedUseCandidate;

      Time mLastReceivedPacket;
      ITimerPtr mLastReceivedPacketTimer;
      Seconds mNoPacketsReceivedRecheckTime {};

      Seconds mExpireRouteTime {};
      ITimerPtr mExpireRouteTimer;
      bool mTestLowerPreferenceCandidatePairs {false};
      bool mBlacklistConsent {false};

      Milliseconds mKeepWarmTimeBase {};
      Milliseconds mKeepWarmTimeRandomizedAddTime {};

      PUID mSecureTransportID {0};
      UseSecureTransportWeakPtr mSecureTransport;
      UseSecureTransportWeakPtr mSecureTransportOld;

      size_t mMaxBufferedPackets {};
      bool mMustBufferPackets {true};
      PacketQueue mBufferedPackets;

      STUNPacket::Options mSTUNPacketOptions;
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

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETypes::CandidatePtr, CandidatePtr)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::IPAddress, IPAddress)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::STUNPacketPtr, STUNPacketPtr)
ZS_DECLARE_PROXY_METHOD_1(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD_3(onNotifyPacketRetried, CandidatePtr, IPAddress, STUNPacketPtr)
ZS_DECLARE_PROXY_METHOD_0(onWarmRoutesChanged)
ZS_DECLARE_PROXY_METHOD_1(onNotifyAttached, PUID)
ZS_DECLARE_PROXY_METHOD_1(onNotifyDetached, PUID)
ZS_DECLARE_PROXY_METHOD_0(onDeliverPendingPackets)
ZS_DECLARE_PROXY_END()

