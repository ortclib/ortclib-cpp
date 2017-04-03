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

#pragma once


#include <ortc/adapter/internal/types.h>

#include <ortc/internal/ortc_StatsReport.h>

#include <ortc/adapter/IPeerConnection.h>
#include <ortc/adapter/IHelper.h>

#include <ortc/IRTPListener.h>
#include <ortc/IRTPSender.h>
#include <ortc/IRTPReceiver.h>
#include <ortc/IDataChannel.h>
#include <ortc/ISCTPTransport.h>

#include <queue>

namespace ortc
{
  namespace adapter
  {
    namespace internal
    {
      ZS_DECLARE_INTERACTION_PTR(IMediaStreamForPeerConnection);

      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark IPeerConnectionAsyncDelegate
      #pragma mark

      interaction IPeerConnectionAsyncDelegate
      {
        ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr);
        ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::StatsTypeSet, StatsTypeSet);

        virtual void onProvideStats(PromiseWithStatsReportPtr promise, StatsTypeSet stats) = 0;
      };

      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerConnection
      #pragma mark

      class PeerConnection : public Noop,
                             public MessageQueueAssociator,
                             public SharedRecursiveLock,
                             public IPeerConnection,
                             public IPeerConnectionAsyncDelegate,
                             public IICEGathererDelegate,
                             public IICETransportDelegate,
                             public IDTLSTransportDelegate,
                             public ISRTPSDESTransportDelegate,
                             public IRTPListenerDelegate,
                             public IRTPSenderDelegate,
                             public IRTPReceiverDelegate,
                             public ISCTPTransportDelegate,
                             public ISCTPTransportListenerDelegate,
                             public IDataChannelDelegate,
                             public IWakeDelegate,
                             public IPromiseSettledDelegate
      {
      protected:
        struct make_private {};

      public:
        friend interaction IPeerConnectionFactory;

        typedef IRTPTypes::SSRCType SSRCType;

        enum InternalStates
        {
          InternalState_First,

          InternalState_Pending = InternalState_First,
          InternalState_Ready,
          InternalState_ShuttingDown,
          InternalState_Shutdown,

          InternalState_Last = InternalState_Shutdown
        };

        static const char *toString(InternalStates state);

        enum NegotiationStates
        {
          NegotiationState_First,

          NegotiationState_PendingOffer = NegotiationState_First,
          NegotiationState_Agreed,
          NegotiationState_LocalOffered,
          NegotiationState_RemoteOffered,
          NegotiationState_Rejected,

          NegotiationState_Last = NegotiationState_Rejected,
        };

        static const char *toString(NegotiationStates state);

        enum PendingMethods
        {
          PendingMethod_First,

          PendingMethod_CreateOffer = PendingMethod_First,
          PendingMethod_CreateAnswer,
          PendingMethod_CreateCapabilities,
          PendingMethod_SetLocalDescription,
          PendingMethod_SetRemoteDescription,

          PendingMethod_Last = PendingMethod_SetRemoteDescription,
        };

        static const char *toString(PendingMethods method);

        typedef String TransportID;
        typedef String MediaLineID;
        typedef String SenderID;
        typedef String ReceiverID;
        typedef String MediaStreamID;

        ZS_DECLARE_STRUCT_PTR(TransportInfo);
        ZS_DECLARE_STRUCT_PTR(MediaLineInfo);
        ZS_DECLARE_STRUCT_PTR(RTPMediaLineInfo);
        ZS_DECLARE_STRUCT_PTR(SCTPMediaLineInfo);
        ZS_DECLARE_STRUCT_PTR(SenderInfo);
        ZS_DECLARE_STRUCT_PTR(ReceiverInfo);
        ZS_DECLARE_STRUCT_PTR(DataChannelInfo);
        ZS_DECLARE_STRUCT_PTR(PendingMethod);
        ZS_DECLARE_STRUCT_PTR(PendingAddTrack);
        ZS_DECLARE_STRUCT_PTR(PendingAddDataChannel);

        ZS_DECLARE_TYPEDEF_PTR(IMediaStreamForPeerConnection, UseMediaStream);
        ZS_DECLARE_TYPEDEF_PTR(std::list<UseMediaStreamPtr>, UseMediaStreamList);
        typedef std::map<MediaStreamID, UseMediaStreamPtr> UseMediaStreamMap;
        ZS_DECLARE_PTR(UseMediaStreamMap);
        ZS_DECLARE_TYPEDEF_PTR(ISessionDescriptionTypes::MediaStreamSet, MediaStreamSet);

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IStatsReportForInternal, UseStatsReport);
        ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr);
        ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::StatsTypeSet, StatsTypeSet);
        
        typedef std::map<PUID, DataChannelInfoPtr> DataChannelMap;
        ZS_DECLARE_PTR(DataChannelMap);

        struct TransportInfo
        {
          struct Details
          {
            IICEGathererPtr mGatherer;
            IICETransportPtr mTransport;
            bool mRTPEndOfCandidates {false};

            IDTLSTransportPtr mDTLSTransport;
            ISRTPSDESTransportPtr mSRTPSDESTransport;
            ISRTPSDESTransportTypes::ParametersPtr mSRTPSDESParameters;

            ElementPtr toDebug() const;
          };

          TransportID mID;

          NegotiationStates mNegotiationState {NegotiationState_First};

          Optional<IDTLSTransportTypes::Roles> mLocalDTLSRole;
          Details mRTP;
          Details mRTCP;
          CertificateList mCertificates;

          ElementPtr toDebug() const;
        };

        struct MediaLineInfo
        {
          MediaLineID mID;
          Optional<size_t> mLineIndex;

          String mBundledTransportID;
          String mPrivateTransportID;

          NegotiationStates mNegotiationState {NegotiationState_First};

          ElementPtr toDebug() const;
        };

        struct RTPMediaLineInfo : public MediaLineInfo
        {
          String mMediaType;

          IHelper::IDPreferences mIDPreference {IHelper::IDPreference_First};
          IRTPTypes::CapabilitiesPtr mLocalSenderCapabilities;
          IRTPTypes::CapabilitiesPtr mLocalReceiverCapabilities;
          IRTPTypes::CapabilitiesPtr mRemoteSenderCapabilities;
          IRTPTypes::CapabilitiesPtr mRemoteReceiverCapabilities;

          ElementPtr toDebug() const;
        };

        struct SCTPMediaLineInfo : public MediaLineInfo
        {
          ISCTPTransportTypes::CapabilitiesPtr mRemoteCapabilities;

          Optional<WORD> mLocalPort;
          Optional<WORD> mRemotePort;
          ISCTPTransportPtr mSCTPTransport;
          DataChannelMap mDataChannels;

          ElementPtr toDebug() const;
        };

        struct SenderInfo
        {
          SenderID mID;
          MediaLineID mMediaLineID;

          MediaStreamTrackConfigurationPtr mConfiguration;
          IMediaStreamTrackPtr mTrack;
          UseMediaStreamMap mMediaStreams;

          NegotiationStates mNegotiationState {NegotiationState_First};
          PromiseWithSenderPtr mPromise;

          IRTPSenderPtr mSender;
          IRTPTypes::ParametersPtr mParameters;

          ElementPtr toDebug() const;
        };

        struct ReceiverInfo
        {
          ReceiverID mID;
          MediaLineID mMediaLineID;
          String mMediaStreamTrackID;

          NegotiationStates mNegotiationState {NegotiationState_First};

          IRTPReceiverPtr mReceiver;
          UseMediaStreamMap mMediaStreams;

          ElementPtr toDebug() const;
        };

        struct DataChannelInfo
        {
          IDataChannelPtr mDataChannel;
          IDataChannelSubscriptionPtr mSubscription;
        };

        struct PendingMethod
        {
          PendingMethods mMethod {PendingMethod_First};
          PromisePtr mPromise;

          Optional<OfferOptions> mOfferOptions;
          Optional<AnswerOptions> mAnswerOptions;
          Optional<CapabilityOptions> mCapabilityOptions;

          ISessionDescriptionPtr mSessionDescription;

          PendingMethod(
                        PendingMethods method,
                        PromisePtr promise
                        ) :
            mMethod(method),
            mPromise(promise) {}

          ElementPtr toDebug() const;
        };

        struct PendingAddTrack
        {
          PromiseWithSenderPtr mPromise;
          IMediaStreamTrackPtr mTrack;
          UseMediaStreamMap mMediaStreams;
          MediaStreamTrackConfigurationPtr mConfiguration;

          ElementPtr toDebug() const;
        };

        struct PendingAddDataChannel
        {
          PromiseWithDataChannelPtr mPromise;
          IDataChannelTypes::ParametersPtr mParameters;

          ElementPtr toDebug() const;
        };

        typedef std::map<TransportID, TransportInfoPtr> TransportInfoMap;
        typedef std::list<TransportInfoPtr> TransportList;
        typedef std::map<MediaLineID, RTPMediaLineInfoPtr> RTPMediaLineInfoMap;
        typedef std::map<MediaLineID, SCTPMediaLineInfoPtr> SCTPMediaLineInfoMap;
        typedef std::map<SenderID, SenderInfoPtr> SenderInfoMap;
        typedef std::map<ReceiverID, ReceiverInfoPtr> ReceiverInfoMap;
        typedef std::list<ICECandidatePtr> CandidateList;
        typedef std::list<PendingMethodPtr> PendingMethodList;
        typedef std::list<PendingAddTrackPtr> PendingAddTrackList;
        typedef std::list<IRTPSenderPtr> SenderList;
        typedef std::list<PendingAddDataChannelPtr> PendingAddDataChannelList;
        typedef std::map<String, size_t> IDMap;
        typedef PUID PromiseID;
        typedef PromiseWithStatsReportPtr StatsCollectionPromisePtr;
        typedef PromiseWithStatsReportPtr ReolveStatsPromisePtr;
        typedef std::pair<StatsCollectionPromisePtr, ReolveStatsPromisePtr> CollectionPromisePair;
        typedef std::map<PromiseID, CollectionPromisePair> StatsPromiseMap;
        typedef std::set<SSRCType> SSRCSet;
        typedef std::queue<SSRCType> SSRCQueue;
        typedef std::set<WORD> PortSet;

      public:
        PeerConnection(
                       const make_private &,
                       IMessageQueuePtr queue,
                       IPeerConnectionDelegatePtr delegate,
                       const Optional<Configuration> &configuration
                       );

        ~PeerConnection();

      public:
        PeerConnection(
                       const Noop &,
                       IMessageQueuePtr queue
                       ) :
          SharedRecursiveLock(SharedRecursiveLock::create()),
          MessageQueueAssociator(queue) {}

        void init();

        static PeerConnectionPtr convert(IPeerConnectionPtr object);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IPeerConnection
        #pragma mark

        static PeerConnectionPtr create(
                                        IPeerConnectionDelegatePtr delegate,
                                        const Optional<Configuration> &configuration = Optional<Configuration>()
                                        );

        virtual PUID getID() const override { return mID; }

        virtual IPeerConnectionSubscriptionPtr subscribe(IPeerConnectionDelegatePtr delegate) override;

        virtual PromiseWithDescriptionPtr createOffer(const Optional<OfferOptions> &configuration = Optional<OfferOptions>()) override;
        virtual PromiseWithDescriptionPtr createAnswer(const Optional<AnswerOptions> &configuration = Optional<AnswerOptions>()) override;

        virtual PromiseWithDescriptionPtr createCapabilities(const Optional<CapabilityOptions> &configuration = Optional<CapabilityOptions>()) override;

        virtual PromisePtr setLocalDescription(ISessionDescriptionPtr description) override;

        virtual ISessionDescriptionPtr localDescription() const override;
        virtual ISessionDescriptionPtr currentDescription() const override;
        virtual ISessionDescriptionPtr pendingDescription() const override;

        virtual PromisePtr setRemoteDescription(ISessionDescriptionPtr description) override;
        virtual ISessionDescriptionPtr remoteDescription() const override;
        virtual ISessionDescriptionPtr currentRemoteDescription() const override;
        virtual ISessionDescriptionPtr pendingRemoteDescription() const override;
        virtual void addICECandidate(const ICECandidate &candidate) override;

        virtual SignalingStates signalingState() const override;
        virtual ICEGatheringStates iceGatheringState() const override;
        virtual ICEConnectionStates iceConnectionState() const override;
        virtual PeerConnectionStates connectionState() const override;
        virtual bool canTrickleCandidates() const override;

        static ServerListPtr getDefaultIceServers();

        virtual ConfigurationPtr getConfiguration() const override;
        virtual void setConfiguration(const Configuration &configuration) override;

        virtual void close() override;

        virtual SenderListPtr getSenders() const override;
        virtual ReceiverListPtr getReceivers() const override;
        virtual PromiseWithSenderPtr addTrack(
                                              IMediaStreamTrackPtr track,
                                              const MediaStreamTrackConfiguration &configuration = MediaStreamTrackConfiguration()
                                              ) override;
        virtual PromiseWithSenderPtr addTrack(
                                              IMediaStreamTrackPtr track,
                                              const MediaStreamList &mediaStreams,
                                              const MediaStreamTrackConfiguration &configuration = MediaStreamTrackConfiguration()
                                              ) override;

        virtual void removeTrack(IRTPSenderPtr sender) override;

        virtual PromiseWithDataChannelPtr createDataChannel(const IDataChannelTypes::Parameters &parameters) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IStatsProvider
        #pragma mark

        virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IPeerConnectionAsyncDelegate
        #pragma mark

        virtual void onProvideStats(PromiseWithStatsReportPtr promise, StatsTypeSet stats) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IICEGathererDelegate
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

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IICETransportDelegate
        #pragma mark

        virtual void onICETransportStateChange(
                                               IICETransportPtr transport,
                                               IICETransport::States state
                                               ) override;

        virtual void onICETransportCandidatePairAvailable(
                                                          IICETransportPtr transport,
                                                          CandidatePairPtr candidatePair
                                                          ) override;
        virtual void onICETransportCandidatePairGone(
                                                     IICETransportPtr transport,
                                                     CandidatePairPtr candidatePair
                                                     ) override;

        virtual void onICETransportCandidatePairChanged(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IDTLSTransportDelegate
        #pragma mark

        virtual void onDTLSTransportStateChange(
                                                IDTLSTransportPtr transport,
                                                IDTLSTransport::States state
                                                ) override;

        virtual void onDTLSTransportError(
                                          IDTLSTransportPtr transport,
                                          ErrorAnyPtr error
                                          ) override;


        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => ISRTPSDESTransportDelegate
        #pragma mark

        virtual void onSRTPSDESTransportLifetimeRemaining(
                                                          ISRTPSDESTransportPtr transport,
                                                          ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                          ULONG overallLifetimeRemainingPercentage
                                                          ) override;

        virtual void onSRTPSDESTransportError(
                                              ISRTPSDESTransportPtr transport,
                                              ErrorAnyPtr errorCode
                                              ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IRTPListenerDelegate
        #pragma mark

        virtual void onRTPListenerUnhandledRTP(
                                               IRTPListenerPtr listener,
                                               SSRCType ssrc,
                                               PayloadType payloadType,
                                               const char *mid,
                                               const char *rid
                                               ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IRTPSenderDelegate
        #pragma mark

        virtual void onRTPSenderSSRCConflict(
                                             IRTPSenderPtr sender,
                                             SSRCType ssrc
                                             ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IRTPReceiverDelegate
        #pragma mark

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => ISCTPTransportDelegate
        #pragma mark

        virtual void onSCTPTransportStateChange(
                                                ISCTPTransportPtr transport,
                                                ISCTPTransportTypes::States state
                                                ) override;
        virtual void onSCTPTransportDataChannel(
                                                ISCTPTransportPtr transport,
                                                IDataChannelPtr channel
                                                ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => ISCTPTransportListenerDelegate
        #pragma mark

        virtual void onSCTPTransport(ISCTPTransportPtr transport) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => ISCTPTransportListenerDelegate
        #pragma mark

        virtual void onDataChannelStateChange(
                                              IDataChannelPtr channel,
                                              IDataChannelTypes::States state
                                              ) override;

        virtual void onDataChannelError(
                                        IDataChannelPtr channel,
                                        ErrorAnyPtr error
                                        ) override;

        virtual void onDataChannelBufferedAmountLow(IDataChannelPtr channel) override;

        virtual void onDataChannelMessage(
                                          IDataChannelPtr channel,
                                          MessageEventDataPtr data
                                          ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IWakeDelegate
        #pragma mark

        virtual void onWake() override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => IPromiseSettledDelegate
        #pragma mark

        virtual void onPromiseSettled(PromisePtr promise) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => (internal)
        #pragma mark

        Log::Params log(const char *message) const;
        Log::Params debug(const char *message) const;
        virtual ElementPtr toDebug() const;

        bool isShuttingDown() const { return InternalState_ShuttingDown == mState; }
        bool isShutdown() const { return InternalState_Shutdown == mState; }
        bool isStopped() const { return isShuttingDown() || isShutdown(); }

        void wake();
        void notifyNegotiationNeeded();

        void cancel();
        void setError(WORD errorCode, const char *errorReason = NULL);

        void step();
        bool stepCertificates();
        bool stepProcessRemote();
        bool stepProcessRemoteTransport(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessRemoteRTPMediaLines(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessRemoteRTPSenders(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessRemoteSCTPTransport(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessLocal();
        bool stepProcessLocalTransport(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessLocalRTPMediaLines(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessLocalRTPSenders(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepProcessLocalSCTPTransport(ISessionDescriptionTypes::DescriptionPtr description);
        bool stepCreateOfferOrAnswer();
        bool stepProcessPendingRemoteCandidates();
        bool stepAddTracks();
        bool stepAddSCTPTransport();
        bool stepFinalizeSenders();
        bool stepFinalizeDataChannels();
        bool stepFixGathererState();
        bool stepFixTransportState();

        void setState(InternalStates state);
        void setState(SignalingStates state);
        void setState(ICEGatheringStates state);
        void setState(ICEConnectionStates state);
        void setState(PeerConnectionStates state);

        void addCandidateToTransport(
                                     TransportInfo &transport,
                                     ICECandidatePtr candidate
                                     );

        TransportInfoPtr getTransportFromPool(const char *useID = NULL);
        void addToTransportPool();

        String registerNewID(size_t length = 3);
        String registerIDUsage(const char *idStr);
        void unregisterID(const char *idStr);
        WORD registerNewLocalPort();

        void flushLocalPending(ISessionDescriptionPtr description);
        void flushRemotePending(ISessionDescriptionPtr description);
        void close(TransportInfo &transportInfo);
        void close(TransportInfo::Details &details);
        void close(RTPMediaLineInfo &mediaLineInfo);
        void close(SCTPMediaLineInfo &mediaLineInfo);
        void close(SenderInfo &senderInfo);
        void close(ReceiverInfo &receiverInfo);

        void insertSSRCs(SenderInfo &senderInfo);
        void clearSSRCs(SenderInfo &senderInfo);
        void clearSSRC(SSRCType ssrc);
        void fillRTCPSSRC(IRTPTypes::Parameters &ioReceiverParameters);

        void close(PendingMethod &pending);
        void close(PendingAddTrack &pending);
        void close(PendingAddDataChannel &pending);

        void purgeNonReferencedAndEmptyStreams();
        Optional<size_t> getNextHighestMLineIndex() const;

        void moveAddedTracksToPending();
        void processStats(
                          PromiseWithStatsReportPtr collectionPromise,
                          PromiseWithStatsReportPtr resolvePromise
                          );

        static MediaStreamListPtr convertToList(const UseMediaStreamMap &useStreams);
        static UseMediaStreamMapPtr convertToMap(const MediaStreamList &mediaStreams);
        static MediaStreamSetPtr convertToSet(const UseMediaStreamMap &useStreams);
        static ISessionDescriptionTypes::ICECandidateListPtr convertCandidateList(IICETypes::CandidateList &source);
        static void calculateDelta(
                                   const MediaStreamSet &existingSet,
                                   const MediaStreamSet &newSet,
                                   MediaStreamSet &outAdded,
                                   MediaStreamSet &outRemoved
                                   );

        static IDTLSTransportTypes::ParametersPtr getDTLSParameters(
                                                                    const TransportInfo &transportInfo,
                                                                    IICETypes::Components component
                                                                    );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerConnection => (data)
        #pragma mark

        AutoPUID mID;
        PeerConnectionWeakPtr mThisWeak;
        PeerConnectionPtr mGracefulShutdownReference;

        IPeerConnectionDelegateSubscriptions mSubscriptions;
        IPeerConnectionSubscriptionPtr mDefaultSubscription;

        Configuration mConfiguration;

        InternalStates mState {InternalState_Pending};
        WORD mErrorCode {};
        String mErrorReason;

        SignalingStates mLastSignalingState {SignalingState_First};
        ICEGatheringStates mLastICEGatheringStates {IICEGathererTypes::State_First};
        ICEConnectionStates mLastICEConnectionState {IICETransport::State_First};
        PeerConnectionStates mLastPeerConnectionState {PeerConnectionState_First};

        std::atomic<bool> mWakeCalled {false};
        bool mNegotiationNeeded {false};

        ISessionDescriptionPtr mLocalDescription;
        ISessionDescriptionPtr mPendingLocalDescription;

        ISessionDescriptionPtr mRemoteDescription;
        ISessionDescriptionPtr mPendingRemoteDescription;

        PendingMethodList mPendingMethods;
        PendingAddTrackList mAddedPendingAddTracks;
        PendingAddTrackList mPendingAddTracks;
        SenderList mPendingRemoveTracks;
        PendingAddDataChannelList mPendingAddDataChannels;

        TransportInfoMap mTransports;
        RTPMediaLineInfoMap mRTPMedias;
        SCTPMediaLineInfoMap mSCTPMedias;
        SenderInfoMap mSenders;
        ReceiverInfoMap mReceivers;

        UseMediaStreamMap mMediaStreams;
        CandidateList mPendingRemoteCandidates;

        TransportList mTransportPool;

        IDMap mExistingIDs;
        PortSet mExistingLocalPorts;

        // step certificates
        ICertificateTypes::PromiseWithCertificatePtr mCertificatePromise;

        // post process the stats
        StatsPromiseMap mPendingStatPromises;

        SSRCSet mAudioSenderSSRCs;
        SSRCSet mVideoSenderSSRCs;

        SSRCQueue mFutureAudioSenderSSRCs;
        SSRCQueue mFutureVideoSenderSSRCs;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IPeerConnectionFactory
      #pragma mark

      interaction IPeerConnectionFactory
      {
        typedef IPeerConnectionTypes::Configuration Configuration;
        typedef IPeerConnectionTypes::ServerListPtr ServerListPtr;

        static IPeerConnectionFactory &singleton();

        virtual PeerConnectionPtr create(
                                         IPeerConnectionDelegatePtr delegate,
                                         const Optional<Configuration> &configuration = Optional<Configuration>()
                                         );

        virtual ServerListPtr getDefaultIceServers();
      };

      class PeerConnectionFactory : public IFactory<IPeerConnectionFactory> {};

    }
  }
}


ZS_DECLARE_PROXY_BEGIN(ortc::adapter::internal::IPeerConnectionAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProviderTypes::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProviderTypes::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_PROXY_METHOD_2(onProvideStats, PromiseWithStatsReportPtr, StatsTypeSet)
ZS_DECLARE_PROXY_END()

