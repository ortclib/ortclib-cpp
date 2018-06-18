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
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSettings);
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICEGatherer);
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICETransportContoller);
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSecureTransport);
    ZS_DECLARE_INTERACTION_PTR(IICETransportForDataTransport);


    ZS_DECLARE_INTERACTION_PROXY(IICETransportAsyncDelegate);

    ZS_DECLARE_INTERACTION_PTR(IICEGathererForICETransport);
    ZS_DECLARE_INTERACTION_PTR(IICETransportControllerForICETransport);


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IICETransportForICEGatherer
    //

    interaction IICETransportForICEGatherer
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, ForICEGatherer);

    ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute);

      virtual PUID getID() const noexcept = 0;

      virtual ForICEGathererPtr getForGatherer() const noexcept = 0;

      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
                                STUNPacketPtr packet
                                ) noexcept = 0;
      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) noexcept = 0;

      virtual bool needsMoreCandidates() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IICETransportForICETransportContoller
    //

    interaction IICETransportForICETransportContoller
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICETransportContoller, ForTransportContoller);

      static ElementPtr toDebug(ForTransportContollerPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual void notifyControllerAttached(ICETransportControllerPtr controller) noexcept = 0;
      virtual void notifyControllerDetached(ICETransportController &controller) noexcept = 0;

      virtual bool hasCandidatePairFoundation(
                                              const String &localFoundation,
                                              const String &remoteFoundation,
                                              PromisePtr promise
                                              ) noexcept = 0;

      virtual IICETypes::Components component() const noexcept = 0;
      virtual IICETransport::States state() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IICETransportForSecureTransport
    //

    interaction IICETransportForSecureTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, ForSecureTransport);

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForICETransport, UseSecureTransport);

      virtual PUID getID() const noexcept = 0;

      virtual void notifyAttached(
                                  PUID secureTransportID,
                                  UseSecureTransportPtr transport
                                  ) noexcept = 0;
      virtual void notifyDetached(PUID secureTransportID) noexcept = 0;

      virtual IICETypes::Components component() const noexcept = 0;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) noexcept = 0;

      virtual IICETransport::States state() const noexcept = 0;

      virtual IICETypes::Roles getRole() const noexcept = 0;

      virtual ICETransportPtr getRTPTransport() const noexcept = 0;
      virtual ICETransportPtr getRTCPTransport() const noexcept = 0;

      virtual UseSecureTransportPtr getSecureTransport() const noexcept = 0;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) noexcept = 0;
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IICETransportForDataTransport
    //

    interaction IICETransportForDataTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDataTransport, ForDataTransport);

      virtual PUID getID() const noexcept = 0;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) noexcept = 0;

      virtual IICETransport::States state() const noexcept = 0;
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IICETransportAsyncDelegate
    //

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
    //
    // ICETransport
    //

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

      ZS_DECLARE_STRUCT_PTR(RouteStateTracker);
      ZS_DECLARE_STRUCT_PTR(Route);
      ZS_DECLARE_STRUCT_PTR(ReasonNoMoreRelationship);

      ZS_DECLARE_TYPEDEF_PTR(ortc::services::ISTUNRequester, ISTUNRequester);
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, UseICEGatherer);
      ZS_DECLARE_TYPEDEF_PTR(IICETransportControllerForICETransport, UseICETransportController);
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate);
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateComplete, CandidateComplete);
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList);
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForICETransport, UseSecureTransport);
      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute);

      typedef String Hash;
      typedef std::map<Hash, CandidatePtr> CandidateMap;
      typedef std::map<Hash, CandidatePairPtr> CandidatePairMap;

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
                   ) noexcept;

    protected:
      ICETransport(
                   Noop,
                   IMessageQueuePtr queue = IMessageQueuePtr(),
                   const SharedRecursiveLock &lock = SharedRecursiveLock::create()
                   ) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(lock) {}

      void init() noexcept;

    public:
      virtual ~ICETransport() noexcept;

      static ICETransportPtr convert(IICETransportPtr object) noexcept;
      static ICETransportPtr convert(IRTCPTransportPtr object) noexcept;
      static ICETransportPtr convert(ForICEGathererPtr object) noexcept;
      static ICETransportPtr convert(ForTransportContollerPtr object) noexcept;
      static ICETransportPtr convert(ForSecureTransportPtr object) noexcept;
      static ICETransportPtr convert(ForDataTransportPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // ICETransport => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICETransport
      //

      static ElementPtr toDebug(ICETransportPtr transport) noexcept;

      static ICETransportPtr create(
                                    IICETransportDelegatePtr delegate,
                                    IICEGathererPtr gatherer
                                    ) noexcept(false); // throws InvalidParmaters

      PUID getID() const noexcept override {return mID;}

      IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) noexcept override;

      IICEGathererPtr iceGatherer() const noexcept override;

      Roles role() const noexcept override;
      Components component() const noexcept override;
      States state() const noexcept override;

      CandidateListPtr getRemoteCandidates() const noexcept override;

      CandidatePairPtr getSelectedCandidatePair() const noexcept override;

      void start(
                 IICEGathererPtr gatherer,
                 const Parameters &remoteParameters,
                 Optional<Options> options = Optional<Options>()
                 ) noexcept(false) override; // throws InvalidParameters

      void stop() noexcept override;

      ParametersPtr getRemoteParameters() const noexcept override;

      IICETransportPtr createAssociatedTransport(IICETransportDelegatePtr delegate) noexcept(false) override; // throws InvalidStateError

      void addRemoteCandidate(const GatherCandidate &remoteCandidate) noexcept(false) override; // throws InvalidStateError, InvalidParameters
      void setRemoteCandidates(const CandidateList &remoteCandidates) noexcept(false) override; // throws InvalidStateError, InvalidParameters
      void removeRemoteCandidate(const GatherCandidate &remoteCandidate) noexcept(false) override; // throws InvalidStateError, InvalidParameters

      virtual void keepWarm(
                            const CandidatePair &candidatePair,
                            bool keepWarm = true
                            ) noexcept(false) override; // throws InvalidStateError

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICETransportForICEGatherer
      //

      virtual ForICEGathererPtr getForGatherer() const noexcept override;

      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
                                STUNPacketPtr packet
                                ) noexcept override;
      virtual void notifyPacket(
                                RouterRoutePtr routerRoute,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) noexcept override;

      virtual bool needsMoreCandidates() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICETransportForICETransportContoller
      //

      // (duplicate) virtual PUID getID() const noexcept = 0;

      virtual void notifyControllerAttached(ICETransportControllerPtr controller) noexcept override;
      virtual void notifyControllerDetached(ICETransportController &controller) noexcept override;

      virtual bool hasCandidatePairFoundation(
                                              const String &localFoundation,
                                              const String &remoteFoundation,
                                              PromisePtr promise
                                              ) noexcept override;

      // (duplicate) virtual IICETypes::Components component() const noexcept = 0;
      // (duplicate) virtual IICETransport::States state() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICETransportForSecureTransport
      //

      // (duplicate) virtual PUID getID() const noexcept = 0;

      void notifyAttached(
                          PUID secureTransportID,
                          UseSecureTransportPtr transport
                          ) noexcept override;
      void notifyDetached(PUID secureTransportID) noexcept override;

      // (duplicate) virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) noexcept = 0;

      // (duplicate) virtual IICETransport::States state() const noexcept = 0;

      IICETypes::Roles getRole() const noexcept override;

      ICETransportPtr getRTPTransport() const noexcept override;
      ICETransportPtr getRTCPTransport() const noexcept override;

      UseSecureTransportPtr getSecureTransport() const noexcept override;

      bool sendPacket(
                      const BYTE *buffer,
                      size_t bufferSizeInBytes
                      ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICETransportForDataTransport
      //

      // (duplicate) virtual PUID getID() const noexcept = 0;

      // (duplicate) virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) noexcept = 0;

      // (duplciate) virtual IICETransport::States state() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICETransportAsyncDelegate
      //

      void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;
      void onNotifyPacketRetried(
                                 IICETypes::CandidatePtr localCandidate,
                                 IPAddress remoteIP,
                                 STUNPacketPtr stunPacket
                                 ) override;
      void onWarmRoutesChanged() override;

      void onNotifyAttached(PUID secureTransportID) override;
      void onNotifyDetached(PUID secureTransportID) override;

      void onDeliverPendingPackets() override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => IICEGathererDelegate
      //

      void onICEGathererStateChange(
                                    IICEGathererPtr gatherer,
                                    IICEGatherer::States state
                                    ) override;

      void onICEGathererLocalCandidate(
                                       IICEGathererPtr gatherer,
                                       CandidatePtr candidate
                                       ) override;

      void onICEGathererLocalCandidateComplete(
                                               IICEGathererPtr gatherer,
                                               CandidateCompletePtr candidate
                                               ) override;

      void onICEGathererLocalCandidateGone(
                                           IICEGathererPtr gatherer,
                                           CandidatePtr candidate
                                           ) override;

      void onICEGathererError(
                              IICEGathererPtr gatherer,
                              ErrorEventPtr errorEvent
                              ) override;

      //-----------------------------------------------------------------------
      //
      // ICETransport => ISTUNRequesterDelegate
      //

      void onSTUNRequesterSendPacket(
                                     ISTUNRequesterPtr requester,
                                     IPAddress destination,
                                     SecureByteBlockPtr packet
                                     ) override;

      bool handleSTUNRequesterResponse(
                                       ISTUNRequesterPtr requester,
                                       IPAddress fromIPAddress,
                                       STUNPacketPtr response
                                       ) noexcept override;

      void onSTUNRequesterTimedOut(ISTUNRequesterPtr requester) override;

    public:

      //-----------------------------------------------------------------------
      //
      // ICETransport::Route
      //

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
        static const char *toString(States state) noexcept;

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

        Route(RouteStateTrackerPtr tracker) noexcept;
        ~Route() noexcept;

        ElementPtr toDebug() const noexcept;

        QWORD getPreference(bool localIsControlling) const noexcept;
        QWORD getActivationPriority(
                                    bool localIsControlling,
                                    bool useUnfreezePreference
                                    ) const noexcept;

        States state() const noexcept;
        void state(States state) noexcept;

        bool isNew() const noexcept {return State_New == mState;}
        bool isPending() const noexcept {return State_Pending == mState;}
        bool isFrozen() const noexcept {return State_Frozen == mState;}
        bool isInProgress() const noexcept {return State_InProgress == mState;}
        bool isSucceeded() const noexcept {return State_Succeeded == mState;}
        bool isIgnored() const noexcept {return State_Ignored == mState;}
        bool isFailed() const noexcept {return State_Failed == mState;}
        bool isBlacklisted() const noexcept {return State_Blacklisted == mState;}

        void trace(
                   const char *function = NULL,
                   const char *message = NULL
                   ) const noexcept;

      protected:
        Log::Params log(const char *message) const noexcept;

        States mState {State_New};
      };

      struct RouteStateTracker
      {
        typedef Route::States States;

        PUID mOuterObjectID {};

        size_t mStates[Route::State_Last+1];

        RouteStateTracker(PUID outerObjectID) noexcept;

        ElementPtr toDebug() const noexcept;

        void inState(States state) noexcept;
        void outState(States state) noexcept;

        size_t count(States state) noexcept;
      };

      struct ReasonNoMoreRelationship : public Any
      {
      };

    protected:
      //-----------------------------------------------------------------------
      //
      // ICETransport => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isConnected() const noexcept;
      bool isComplete() const noexcept;
      bool isDisconnected() const noexcept;
      bool isFailed() const noexcept;
      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;
      bool isContinousGathering() const noexcept;

      void step() noexcept;
      bool stepCalculateLegalPairs() noexcept;
      bool stepProcessLegalPairs(CandidatePairMap &pairings) noexcept;
      bool stepPendingActivation() noexcept;
      bool stepActivationTimer() noexcept;
      bool stepPickRoute() noexcept;
      bool stepUseCandidate() noexcept;
      bool stepDewarmRoutes() noexcept;
      bool stepKeepWarmRoutes() noexcept;
      bool stepExpireRouteTimer() noexcept;
      bool stepLastReceivedPacketTimer() noexcept;
      bool stepSetCurrentState() noexcept;
      bool stepSetNeedsMoreCandidates() noexcept;

      void wakeUp() noexcept;
      void warmRoutesChanged() noexcept;
      bool hasWarmRoutesChanged() noexcept;
      void cancel() noexcept;

      void setState(IICETransportTypes::States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

      void handleExpireRouteTimer() noexcept;
      void handleLastReceivedPacket() noexcept;
      void handleActivationTimer() noexcept;
      void handleNextKeepWarmTimer(RoutePtr route) noexcept;

      void forceActive(RoutePtr route) noexcept;
      void shutdown(RoutePtr route) noexcept;

      void pruneAllCandidatePairs(bool keepActiveAlive) noexcept;
      CandidatePairPtr cloneCandidatePair(RoutePtr route) const noexcept;

      void setPending(RoutePtr route) noexcept;
      void setFrozen(
                     RoutePtr route,
                     PromisePtr promise
                     ) noexcept;
      void setInProgress(RoutePtr route) noexcept;
      void setSucceeded(RoutePtr route) noexcept;
      void setIgnored(RoutePtr route) noexcept;
      void setFailed(RoutePtr route) noexcept;
      void setBlacklisted(RoutePtr route) noexcept;

      void updateAfterPacket(RoutePtr route) noexcept;

      bool installGathererRoute(RoutePtr route) noexcept;

      void installFoundation(RoutePtr route) noexcept;

      void removeLegal(RoutePtr route) noexcept;
      void removeFoundation(RoutePtr route) noexcept;
      void removeFrozen(RoutePtr route) noexcept;
      void removeFrozenDependencies(
                                    RoutePtr route,
                                    bool succeeded,
                                    AnyPtr reason = AnyPtr()
                                    ) noexcept;
      void removeActive(RoutePtr route) noexcept;
      void removePendingActivation(RoutePtr route) noexcept;
      void removeOutgoingCheck(RoutePtr route) noexcept;
      void removeGathererRoute(RoutePtr route) noexcept;
      void removeKeepWarmTimer(RoutePtr route) noexcept;
      void removeWarm(RoutePtr route) noexcept;

      RoutePtr findRoute(
                         IICETypes::CandidatePtr localCandidate,
                         const IPAddress &remoteIP
                         ) noexcept;
      RoutePtr findClosestRoute(
                                IICETypes::CandidatePtr localCandidate,
                                const IPAddress &remoteIP,
                                DWORD remotePriority
                                ) noexcept;
      Time getLastRemoteActivity(RoutePtr route) const noexcept;

      ISTUNRequesterPtr createBindRequest(
                                          RoutePtr route,
                                          bool useCandidate = false,
                                          IBackOffTimerPatternPtr pattern = IBackOffTimerPatternPtr()
                                          ) const noexcept;
      STUNPacketPtr createBindResponse(
                                       STUNPacketPtr request,
                                       RoutePtr route
                                       ) const noexcept;
      STUNPacketPtr createErrorResponse(
                                        STUNPacketPtr request,
                                        STUNPacket::ErrorCodes error
                                        ) noexcept;
      void setRole(STUNPacketPtr packet) const noexcept;
      void fix(STUNPacketPtr stun) const noexcept;

      void sendPacket(
                      RouterRoutePtr routerRoute,
                      STUNPacketPtr packet
                      ) noexcept;

      bool handleSwitchRolesAndConflict(
                                        RouterRoutePtr routerRoute,
                                        STUNPacketPtr packet
                                        ) noexcept;

      RoutePtr findOrCreateMissingRoute(
                                        RouterRoutePtr routerRoute,
                                        STUNPacketPtr packet
                                        ) noexcept;

      void handlePassThroughSTUNPacket(                       // do not call within a lock
                                       RouterRoutePtr routerRoute,
                                       STUNPacketPtr packet
                                       ) noexcept;

      bool getNeedsMoreCandidates() const noexcept;

    private:
      //-----------------------------------------------------------------------
      //
      // ICEGatherer => (data)
      //
      
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

      CandidateMap mLocalCandidates;
      bool mLocalCandidatesComplete {false};

      String mRemoteCandidatesHash;
      CandidateMap mRemoteCandidates;
      bool mRemoteCandidatesComplete {false};

      IMessageQueuePtr mLegalRoutesComputeQueue;
      bool mLegalRoutesDirty {true};
      bool mComputedLegalRoutes {false};
      bool mComputingLegalRoutes {false};
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
    //
    // IICETransportFactory
    //

    interaction IICETransportFactory
    {
      static IICETransportFactory &singleton() noexcept;

      virtual ICETransportPtr create(
                                     IICETransportDelegatePtr delegate,
                                     IICEGathererPtr gatherer
                                     ) noexcept(false); // throws InvalidParameters
    };

    class ICETransportFactory : public IFactory<IICETransportFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETypes::CandidatePtr, CandidatePtr)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::IPAddress, IPAddress)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::STUNPacketPtr, STUNPacketPtr)
ZS_DECLARE_PROXY_METHOD(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD(onNotifyPacketRetried, CandidatePtr, IPAddress, STUNPacketPtr)
ZS_DECLARE_PROXY_METHOD(onWarmRoutesChanged)
ZS_DECLARE_PROXY_METHOD(onNotifyAttached, PUID)
ZS_DECLARE_PROXY_METHOD(onNotifyDetached, PUID)
ZS_DECLARE_PROXY_METHOD(onDeliverPendingPackets)
ZS_DECLARE_PROXY_END()

