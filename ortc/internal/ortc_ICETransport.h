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

namespace ortc
{
  namespace internal
  {
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
                         public IICETransportForICEGatherer,
                         public IICETransportForICETransportContoller,
                         public IICETransportForRTPTransport,
                         public ITransportAsyncDelegate,
                         public IWakeDelegate,
                         public IICEGathererDelegate
    {
    public:
      friend interaction IICETransport;
      friend interaction IICETransportFactory;
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

      typedef String FoundationID;
      typedef std::map<FoundationID, PromisePtr> FoundationPromiseMap;

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
      static ICETransportPtr convert(ForICEGathererPtr object);
      static ICETransportPtr convert(ForTransportContollerPtr object);
      static ICETransportPtr convert(ForRTPTransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransport
      #pragma mark

      static ElementPtr toDebug(ICETransportPtr transport);

      static ICETransportPtr create(
                                    IICETransportDelegatePtr delegate,
                                    IICEGathererPtr gatherer
                                    );

      virtual PUID getID() const;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate);

      virtual IICEGathererPtr iceGatherer() const;

      virtual Roles role() const;
      virtual Components component() const;
      virtual States state() const;

      virtual CandidateListPtr getRemoteCandidates() const;

      virtual CandidatePairPtr getNominatedCandidatePair() const;

      virtual void start(
                         IICEGathererPtr gatherer,
                         Parameters remoteParameters,
                         Optional<Options> options = Optional<Options>()
                         ) throw (InvalidParameters);

      virtual void stop();

      virtual ParametersPtr getRemoteParameters() const;

      virtual IICETransportPtr createAssociatedTransport() throw (InvalidStateError);

      virtual void addRemoteCandidate(const GatherCandidate &remoteCandidate);
      virtual void setRemoteCandidates(const CandidateList &remoteCandidates);
      virtual void removeRemoteCandidate(const GatherCandidate &remoteCandidate);

      virtual void keepWarm(const CandidatePair &candidatePair);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForICEGatherer
      #pragma mark

      virtual void notifyRouteAdded(
                                    PUID routeID,
                                    IICETypes::CandidatePtr localCandidate,
                                    const IPAddress &fromIP
                                    );
      virtual void notifyRouteRemoved(PUID routeID);

      virtual void notifyPacket(
                                PUID routeID,
                                STUNPacketPtr packet
                                );
      virtual void notifyPacket(
                                PUID routeID,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                );

      virtual bool needsMoreCandidates() const;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForICETransportContoller
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      virtual void notifyControllerAttached(ICETransportControllerPtr controller);
      virtual void notifyControllerDetached();

      virtual bool hasCandidatePairFoundation(
                                              const String &localFoundation,
                                              const String &remoteFoundation,
                                              PromisePtr promise
                                              );

      // (duplicate) virtual IICETypes::Components component() const = 0;
      // (duplicate) virtual IICETransport::States state() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForRTPTransport
      #pragma mark

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => ITransportAsyncDelegate
      #pragma mark

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IWakeDelegate
      #pragma mark

      virtual void onWake();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICEGathererDelegate
      #pragma mark

      virtual void onICEGathererStateChanged(
                                             IICEGathererPtr gatherer,
                                             IICEGatherer::States state
                                             );

      virtual void onICEGathererLocalCandidate(
                                               IICEGathererPtr gatherer,
                                               CandidatePtr candidate
                                               );

      virtual void onICEGathererLocalCandidateComplete(
                                                       IICEGathererPtr gatherer,
                                                       CandidateCompletePtr candidate
                                                       );

      virtual void onICEGathererLocalCandidateGone(
                                                   IICEGathererPtr gatherer,
                                                   CandidatePtr candidate
                                                   );

      virtual void onICEGathererError(
                                      IICEGathererPtr gatherer,
                                      ErrorCode errorCode,
                                      String errorReason
                                      );

    public:

      struct Route
      {
        PUID mGathererRouteID {0};
        CandidatePairPtr mCandidatePair;

        Time mLastReceived;
        Time mLastSent;

        bool mPrune {false};
        bool mKeepWarm {false};
        ISTUNRequesterPtr mOutgoingCheck;
        TimerPtr mNextKeepWarm;

        PromisePtr mFrozenPromise;

        ElementPtr toDebug() const;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;
      bool isContinousGathering() const;

      void step();

      void cancel();

      void setState(IICETransportTypes::States state);
      void setError(WORD error, const char *reason = NULL);

      void shutdown(RoutePtr route);

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

      String mOptionsHash;
      Options mOptions;

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
      SortedRouteMap mPendingActivation;
      RouteMap mSearching;
      STUNCheckMap mOutgoingChecks;
      RouteMap mFailed;
      RouteMap mSucceeded;

      FoundationPromiseMap mFrozenFoundations;

      RouteIDMap mRoutes;

      RoutePtr mActiveRoute;
      SortedRouteMap mWarmRoutes;
      STUNCheckMap mKeepWarmChecks;
      TimerRouteMap mNextKeepWarmTimers;
      RouteMap mBlacklisted;
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
