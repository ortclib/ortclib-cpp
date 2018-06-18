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
      //
      // IPeerConnectionAsyncDelegate
      //

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
      //
      // PeerConnection
      //

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

        static const char *toString(InternalStates state) noexcept;

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

        static const char *toString(NegotiationStates state) noexcept;

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

        static const char *toString(PendingMethods method) noexcept;

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

        typedef std::set<String> HashSet;

        struct TransportInfo
        {
          struct Details
          {
            IICEGathererPtr mGatherer;
            IICETransportPtr mTransport;
            HashSet mProcessedICECandidates;
            bool mICEEndOfCandidates {false};

            IDTLSTransportPtr mDTLSTransport;
            ISRTPSDESTransportPtr mSRTPSDESTransport;
            ISRTPSDESTransportTypes::ParametersPtr mSRTPSDESParameters;

            ElementPtr toDebug() const noexcept;
          };

          TransportID mID;

          NegotiationStates mNegotiationState {NegotiationState_First};

          Optional<IDTLSTransportTypes::Roles> mLocalDTLSRole;
          Details mRTP;
          Details mRTCP;
          CertificateList mCertificates;

          ElementPtr toDebug() const noexcept;
        };

        struct MediaLineInfo
        {
          MediaLineID mID;
          Optional<size_t> mLineIndex;

          String mBundledTransportID;
          String mPrivateTransportID;

          NegotiationStates mNegotiationState {NegotiationState_First};

          ElementPtr toDebug() const noexcept;
        };

        struct RTPMediaLineInfo : public MediaLineInfo
        {
          String mMediaType;

          ISessionDescriptionTypes::MediaDirections mDirection {ISessionDescriptionTypes::MediaDirection_SendReceive};
           
          IHelper::IDPreferences mIDPreference {IHelper::IDPreference_First};
          IRTPTypes::CapabilitiesPtr mLocalSenderCapabilities;
          IRTPTypes::CapabilitiesPtr mLocalReceiverCapabilities;
          IRTPTypes::CapabilitiesPtr mRemoteSenderCapabilities;
          IRTPTypes::CapabilitiesPtr mRemoteReceiverCapabilities;

          ElementPtr toDebug() const noexcept;
        };

        struct SCTPMediaLineInfo : public MediaLineInfo
        {
          ISCTPTransportTypes::CapabilitiesPtr mRemoteCapabilities;

          Optional<WORD> mLocalPort;
          Optional<WORD> mRemotePort;
          ISCTPTransportPtr mSCTPTransport;
          DataChannelMap mDataChannels;

          ElementPtr toDebug() const noexcept;
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

          ElementPtr toDebug() const noexcept;
        };

        struct ReceiverInfo
        {
          ReceiverID mID;
          MediaLineID mMediaLineID;
          String mMediaStreamTrackID;

          NegotiationStates mNegotiationState {NegotiationState_First};

          IRTPReceiverPtr mReceiver;
          UseMediaStreamMap mMediaStreams;

          ElementPtr toDebug() const noexcept;
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
                        ) noexcept :
            mMethod(method),
            mPromise(promise) {}

          ElementPtr toDebug() const noexcept;
        };

        struct PendingAddTrack
        {
          PromiseWithSenderPtr mPromise;
          IMediaStreamTrackPtr mTrack;
          UseMediaStreamMap mMediaStreams;
          MediaStreamTrackConfigurationPtr mConfiguration;

          ElementPtr toDebug() const noexcept;
        };

        struct PendingAddDataChannel
        {
          PromiseWithDataChannelPtr mPromise;
          IDataChannelTypes::ParametersPtr mParameters;

          ElementPtr toDebug() const noexcept;
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
                       ) noexcept;

        ~PeerConnection() noexcept;

      public:
        PeerConnection(
                       const Noop &,
                       IMessageQueuePtr queue
                       ) noexcept :
          MessageQueueAssociator(queue),
          SharedRecursiveLock(SharedRecursiveLock::create()) {}

        void init() noexcept;

        static PeerConnectionPtr convert(IPeerConnectionPtr object) noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // PeerConnection => IPeerConnection
        //

        static PeerConnectionPtr create(
                                        IPeerConnectionDelegatePtr delegate,
                                        const Optional<Configuration> &configuration = Optional<Configuration>()
                                        ) noexcept;

        virtual PUID getID() const noexcept override { return mID; }

        IPeerConnectionSubscriptionPtr subscribe(IPeerConnectionDelegatePtr delegate) noexcept override;

        PromiseWithDescriptionPtr createOffer(const Optional<OfferOptions> &configuration = Optional<OfferOptions>()) noexcept override;
        PromiseWithDescriptionPtr createAnswer(const Optional<AnswerOptions> &configuration = Optional<AnswerOptions>()) noexcept override;

        PromiseWithDescriptionPtr createCapabilities(const Optional<CapabilityOptions> &configuration = Optional<CapabilityOptions>()) noexcept override;

        PromisePtr setLocalDescription(ISessionDescriptionPtr description) noexcept override;

        ISessionDescriptionPtr localDescription() const noexcept override;
        ISessionDescriptionPtr currentDescription() const noexcept override;
        ISessionDescriptionPtr pendingDescription() const noexcept override;

        PromisePtr setRemoteDescription(ISessionDescriptionPtr description) noexcept override;
        ISessionDescriptionPtr remoteDescription() const noexcept override;
        ISessionDescriptionPtr currentRemoteDescription() const noexcept override;
        ISessionDescriptionPtr pendingRemoteDescription() const noexcept override;
        void addICECandidate(const ICECandidate &candidate) noexcept override;

        SignalingStates signalingState() const noexcept override;
        ICEGatheringStates iceGatheringState() const noexcept override;
        ICEConnectionStates iceConnectionState() const noexcept override;
        PeerConnectionStates connectionState() const noexcept override;
        bool canTrickleCandidates() const noexcept override;

        static ServerListPtr getDefaultIceServers() noexcept;

        ConfigurationPtr getConfiguration() const noexcept override;
        void setConfiguration(const Configuration &configuration) noexcept override;

        void close() noexcept override;

        SenderListPtr getSenders() const noexcept override;
        ReceiverListPtr getReceivers() const noexcept override;
        PromiseWithSenderPtr addTrack(
                                      IMediaStreamTrackPtr track,
                                      const MediaStreamTrackConfiguration &configuration = MediaStreamTrackConfiguration()
                                      ) noexcept override;
        PromiseWithSenderPtr addTrack(
                                      IMediaStreamTrackPtr track,
                                      const MediaStreamList &mediaStreams,
                                      const MediaStreamTrackConfiguration &configuration = MediaStreamTrackConfiguration()
                                      ) noexcept override;

        void removeTrack(IRTPSenderPtr sender) noexcept override;

        PromiseWithDataChannelPtr createDataChannel(const IDataChannelTypes::Parameters &parameters) noexcept override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IStatsProvider
        //

        PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IPeerConnectionAsyncDelegate
        //

        void onProvideStats(PromiseWithStatsReportPtr promise, StatsTypeSet stats) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IICEGathererDelegate
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

        //---------------------------------------------------------------------
        //
        // PeerConnection => IICETransportDelegate
        //

        void onICETransportStateChange(
                                       IICETransportPtr transport,
                                       IICETransport::States state
                                       ) override;

        void onICETransportCandidatePairAvailable(
                                                  IICETransportPtr transport,
                                                  CandidatePairPtr candidatePair
                                                  ) override;
        void onICETransportCandidatePairGone(
                                             IICETransportPtr transport,
                                             CandidatePairPtr candidatePair
                                             ) override;

        void onICETransportCandidatePairChanged(
                                                IICETransportPtr transport,
                                                CandidatePairPtr candidatePair
                                                ) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IDTLSTransportDelegate
        //

        void onDTLSTransportStateChange(
                                        IDTLSTransportPtr transport,
                                        IDTLSTransport::States state
                                        ) override;

        void onDTLSTransportError(
                                  IDTLSTransportPtr transport,
                                  ErrorAnyPtr error
                                  ) override;


        //---------------------------------------------------------------------
        //
        // PeerConnection => ISRTPSDESTransportDelegate
        //

        void onSRTPSDESTransportLifetimeRemaining(
                                                  ISRTPSDESTransportPtr transport,
                                                  ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                  ULONG overallLifetimeRemainingPercentage
                                                  ) override;

        void onSRTPSDESTransportError(
                                      ISRTPSDESTransportPtr transport,
                                      ErrorAnyPtr errorCode
                                      ) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IRTPListenerDelegate
        //

        void onRTPListenerUnhandledRTP(
                                       IRTPListenerPtr listener,
                                       SSRCType ssrc,
                                       PayloadType payloadType,
                                       const char *mid,
                                       const char *rid
                                       ) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IRTPSenderDelegate
        //

        void onRTPSenderSSRCConflict(
                                     IRTPSenderPtr sender,
                                     SSRCType ssrc
                                     ) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IRTPReceiverDelegate
        //

        //---------------------------------------------------------------------
        //
        // PeerConnection => ISCTPTransportDelegate
        //

        void onSCTPTransportStateChange(
                                        ISCTPTransportPtr transport,
                                        ISCTPTransportTypes::States state
                                        ) override;
        void onSCTPTransportDataChannel(
                                        ISCTPTransportPtr transport,
                                        IDataChannelPtr channel
                                        ) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => ISCTPTransportListenerDelegate
        //

        void onSCTPTransport(ISCTPTransportPtr transport) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => ISCTPTransportListenerDelegate
        //

        void onDataChannelStateChange(
                                      IDataChannelPtr channel,
                                      IDataChannelTypes::States state
                                      ) override;

        void onDataChannelError(
                                IDataChannelPtr channel,
                                ErrorAnyPtr error
                                ) override;

        void onDataChannelBufferedAmountLow(IDataChannelPtr channel) override;

        void onDataChannelMessage(
                                  IDataChannelPtr channel,
                                  MessageEventDataPtr data
                                  ) override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IWakeDelegate
        //

        void onWake() override;

        //---------------------------------------------------------------------
        //
        // PeerConnection => IPromiseSettledDelegate
        //

        void onPromiseSettled(PromisePtr promise) override;

      protected:
        //---------------------------------------------------------------------
        //
        // PeerConnection => (internal)
        //

        Log::Params log(const char *message) const noexcept;
        Log::Params debug(const char *message) const noexcept;
        virtual ElementPtr toDebug() const noexcept;

        bool isShuttingDown() const noexcept { return InternalState_ShuttingDown == mState; }
        bool isShutdown() const noexcept { return InternalState_Shutdown == mState; }
        bool isStopped() const noexcept { return isShuttingDown() || isShutdown(); }

        void wake() noexcept;
        void notifyNegotiationNeeded() noexcept;

        void cancel() noexcept;
        void setError(WORD errorCode, const char *errorReason = NULL) noexcept;

        void step() noexcept;
        bool stepCertificates() noexcept;
        bool stepProcessRemote() noexcept;
        bool stepProcessRemoteTransport(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessRemoteRTPMediaLines(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessRemoteRTPSenders(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessRemoteSCTPTransport(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessLocal() noexcept;
        bool stepProcessLocalTransport(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessLocalRTPMediaLines(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessLocalRTPSenders(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepProcessLocalSCTPTransport(ISessionDescriptionTypes::DescriptionPtr description) noexcept;
        bool stepCreateOfferOrAnswer() noexcept;
        bool stepProcessPendingRemoteCandidates() noexcept;
        bool stepAddTracks() noexcept;
        bool stepAddSCTPTransport() noexcept;
        bool stepFinalizeSenders() noexcept;
        bool stepFinalizeDataChannels() noexcept;
        bool stepFixGathererState() noexcept;
        bool stepFixTransportState() noexcept;

        void setState(InternalStates state) noexcept;
        void setState(SignalingStates state) noexcept;
        void setState(ICEGatheringStates state) noexcept;
        void setState(ICEConnectionStates state) noexcept;
        void setState(PeerConnectionStates state) noexcept;

        void addCandidateToTransport(
                                     TransportInfo &transport,
                                     ICECandidatePtr candidate
                                     ) noexcept;

        TransportInfoPtr getTransportFromPool(const char *useID = NULL) noexcept;
        void addToTransportPool() noexcept;

        String registerNewID(size_t length = 3) noexcept;
        String registerIDUsage(const char *idStr) noexcept;
        void unregisterID(const char *idStr) noexcept;
        WORD registerNewLocalPort() noexcept;

        void flushLocalPending(ISessionDescriptionPtr description) noexcept;
        void flushRemotePending(ISessionDescriptionPtr description) noexcept;
        void close(TransportInfo &transportInfo) noexcept;
        void close(TransportInfo::Details &details) noexcept;
        void close(RTPMediaLineInfo &mediaLineInfo) noexcept;
        void close(SCTPMediaLineInfo &mediaLineInfo) noexcept;
        void close(SenderInfo &senderInfo) noexcept;
        void close(ReceiverInfo &receiverInfo) noexcept;

        void insertSSRCs(SenderInfo &senderInfo) noexcept;
        void clearSSRCs(SenderInfo &senderInfo) noexcept;
        void clearSSRC(SSRCType ssrc) noexcept;
        void fillRTCPSSRC(IRTPTypes::Parameters &ioReceiverParameters) noexcept;

        void close(PendingMethod &pending) noexcept;
        void close(PendingAddTrack &pending) noexcept;
        void close(PendingAddDataChannel &pending) noexcept;

        void purgeNonReferencedAndEmptyStreams() noexcept;
        Optional<size_t> getNextHighestMLineIndex() const noexcept;

        void moveAddedTracksToPending() noexcept;
        void processStats(
                          PromiseWithStatsReportPtr collectionPromise,
                          PromiseWithStatsReportPtr resolvePromise
                          ) noexcept;

        static MediaStreamListPtr convertToList(const UseMediaStreamMap &useStreams) noexcept;
        static UseMediaStreamMapPtr convertToMap(const MediaStreamList &mediaStreams) noexcept;
        static MediaStreamSetPtr convertToSet(const UseMediaStreamMap &useStreams) noexcept;
        static ISessionDescriptionTypes::ICECandidateListPtr convertCandidateList(IICETypes::CandidateList &source) noexcept;
        static void calculateDelta(
                                   const MediaStreamSet &existingSet,
                                   const MediaStreamSet &newSet,
                                   MediaStreamSet &outAdded,
                                   MediaStreamSet &outRemoved
                                   ) noexcept;

        static IDTLSTransportTypes::ParametersPtr getDTLSParameters(
                                                                    const TransportInfo &transportInfo,
                                                                    IICETypes::Components component
                                                                    ) noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // PeerConnection => (data)
        //

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
      //
      // IPeerConnectionFactory
      //

      interaction IPeerConnectionFactory
      {
        typedef IPeerConnectionTypes::Configuration Configuration;
        typedef IPeerConnectionTypes::ServerListPtr ServerListPtr;

        static IPeerConnectionFactory &singleton() noexcept;

        virtual PeerConnectionPtr create(
                                         IPeerConnectionDelegatePtr delegate,
                                         const Optional<Configuration> &configuration = Optional<Configuration>()
                                         ) noexcept;

        virtual ServerListPtr getDefaultIceServers() noexcept;
      };

      class PeerConnectionFactory : public IFactory<IPeerConnectionFactory> {};

    }
  }
}


ZS_DECLARE_PROXY_BEGIN(ortc::adapter::internal::IPeerConnectionAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProviderTypes::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProviderTypes::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_PROXY_METHOD(onProvideStats, PromiseWithStatsReportPtr, StatsTypeSet)
ZS_DECLARE_PROXY_END()

