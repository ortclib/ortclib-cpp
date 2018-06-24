/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPTypes.h>

#include <ortc/IICETransport.h>
#include <ortc/IRTPReceiver.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IMediaStreamTrack.h>
#include <ortc/IStatsProvider.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

#define ORTC_SETTING_RTP_RECEIVER_SSRC_TIMEOUT_IN_SECONDS "ortc/rtp-receiver/ssrc-timeout-in-seconds"

#define ORTC_SETTING_RTP_RECEIVER_MAX_RTP_PACKETS_IN_BUFFER "ortc/rtp-receiver/max-rtp-packets-in-buffer"
#define ORTC_SETTING_RTP_RECEIVER_MAX_AGE_RTP_PACKETS_IN_SECONDS "ortc/rtp-receiver/max-age-rtp-packets-in-seconds"

#define ORTC_SETTING_RTP_RECEIVER_CSRC_EXPIRY_TIME_IN_SECONDS "ortc/rtp-receiver/csrc-expiry-time-in-seconds"

#define ORTC_SETTING_RTP_RECEIVER_ONLY_RESOLVE_AMBIGUOUS_PAYLOAD_MAPPING_IF_ACTIVITY_DIFFERS_IN_MILLISECONDS "ortc/rtp-receiver/only-resolve-ambiguous-payload-mapping-if-activity-differs-in-milliseconds"

#define ORTC_SETTING_RTP_RECEIVER_LOCK_TO_RECEIVER_CHANNEL_AFTER_SWITCH_EXCLUSIVELY_FOR_IN_MILLISECONDS "ortc/rtp-receiver/lock-to-receiver-channel-after-switch-in-milliseconds"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPListener);
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForMediaStreamTrack);

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForRTPReceiver);

    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForRTPReceiver);
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPReceiver);
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPReceiverChannel);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiver);

    ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverAsyncDelegate);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverForRTPListener
    //

    interaction IRTPReceiverForRTPListener
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, ForRTPListener);

      static ElementPtr toDebug(ForRTPListenerPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTPPacketPtr packet
                                ) noexcept = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTCPPacketPtr packet
                                ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverForRTPReceiverChannel
    //

    interaction IRTPReceiverForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPReceiverChannel, ForRTPReceiverChannel);

      static ElementPtr toDebug(ForRTPReceiverChannelPtr object) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual bool sendPacket(RTCPPacketPtr packet) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverForMediaStreamTrack
    //

    interaction IRTPReceiverForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForMediaStreamTrack, ForMediaStreamTrack);

      static ElementPtr toDebug(ForMediaStreamTrackPtr object) noexcept;

      virtual PUID getID() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverAsyncDelegate
    //

    interaction IRTPReceiverAsyncDelegate
    {
      virtual ~IRTPReceiverAsyncDelegate() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPReceiver
    //
    
    class RTPReceiver : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IRTPReceiver,
                        public IRTPReceiverForRTPListener,
                        public IRTPReceiverForRTPReceiverChannel,
                        public IRTPReceiverForMediaStreamTrack,
                        public ISecureTransportDelegate,
                        public IWakeDelegate,
                        public zsLib::ITimerDelegate,
                        public IRTPReceiverAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPReceiver;
      friend interaction IRTPReceiverFactory;
      friend interaction IRTPReceiverForRTPListener;
      friend interaction IRTPReceiverForRTPReceiverChannel;
      friend interaction IRTPReceiverForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPReceiver, UseSecureTransport);
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPReceiver, UseListener);
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiver, UseChannel);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, UseMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);

      ZS_DECLARE_STRUCT_PTR(RegisteredHeaderExtension);
      ZS_DECLARE_STRUCT_PTR(ChannelHolder);
      ZS_DECLARE_STRUCT_PTR(ChannelInfo);
      ZS_DECLARE_STRUCT_PTR(SSRCInfo);
      ZS_DECLARE_STRUCT_PTR(CodecInfo);

      friend ChannelHolder;

      enum CodecTypes
      {
        CodecType_First,

        CodecType_Normal = CodecType_First,
        CodecType_RED,
        CodecType_ULPFEC,
        CodecType_FlexFEC,
        CodecType_RTX,

        CodecType_Last = CodecType_RTX,
      };

      static const char *toString(CodecTypes codecType) noexcept;
      static CodecTypes toCodecType(const char *type) noexcept(false); // throws InvalidParameters

      typedef IRTPTypes::SSRCType SSRCType;
      typedef std::list<RTCPPacketPtr> RTCPPacketList;

      ZS_DECLARE_TYPEDEF_PTR(std::list<ParametersPtr>, ParametersPtrList)

      ZS_DECLARE_PTR(RTCPPacketList)

      typedef std::pair<Time, RTPPacketPtr> TimeRTPPacketPair;
      typedef std::list<TimeRTPPacketPair> BufferedRTPPacketList;

      typedef String RID;
      typedef PUID ChannelID;

      typedef std::map<ChannelID, ChannelHolderWeakPtr> ChannelWeakMap;
      ZS_DECLARE_PTR(ChannelWeakMap)

      typedef DWORD RoutingPayloadType;

      typedef std::pair<SSRCType, RoutingPayloadType> SSRCRoutingPair;
      typedef std::map<SSRCRoutingPair, SSRCInfoPtr> SSRCRoutingMap;
      typedef std::map<SSRCRoutingPair, SSRCInfoWeakPtr> SSRCRoutingWeakMap;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::RegisteredHeaderExtension
      //

      typedef USHORT LocalID;
      typedef size_t ReferenceCount;

      struct RegisteredHeaderExtension
      {
        HeaderExtensionURIs mHeaderExtensionURI {HeaderExtensionURI_Unknown};
        LocalID mLocalID {};
        bool mEncrypted {};

        ElementPtr toDebug() const noexcept;
      };

      typedef std::map<LocalID, RegisteredHeaderExtension> HeaderExtensionMap;

      typedef std::map<SSRCType, ContributingSource> ContributingSourceMap;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::ChannelHolder
      //

      struct ChannelHolder
      {
        RTPReceiverWeakPtr mHolder;
        UseChannelPtr mChannel;
        ChannelInfoPtr mChannelInfo;
        std::atomic<ISecureTransport::States> mLastReportedState {ISecureTransport::State_Pending};

        ChannelHolder() noexcept;
        ~ChannelHolder() noexcept;

        PUID getID() const noexcept;
        void notify(ISecureTransport::States state) noexcept;

        void notify(RTPPacketPtr packet) noexcept;
        void notify(RTCPPacketListPtr packets) noexcept;

        void update(const Parameters &params) noexcept;

        bool handle(RTPPacketPtr packet) noexcept;
        bool handle(RTCPPacketPtr packet) noexcept;

        void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) noexcept;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::ChannelInfo
      //

      struct ChannelInfo
      {
        AutoPUID mID;

        ParametersPtr mOriginalParameters;
        ParametersPtr mFilledParameters;

        ChannelHolderWeakPtr mChannelHolder;  // NOTE: might be null if channel is not created yet (or previously destoyed)
        SSRCRoutingMap mRegisteredSSRCs;

        bool shouldLatchAll() const noexcept;
        String rid() const noexcept;

        void registerHolder(ChannelHolderPtr channelHolder) noexcept;

        SSRCInfoPtr registerSSRCUsage(SSRCInfoPtr ssrcInfo) noexcept;
        void unregisterSSRCUsage(
                                 SSRCType ssrc,
                                 RoutingPayloadType routingPayload
                                 ) noexcept;

        ElementPtr toDebug() const noexcept;
      };

      typedef std::map<ParametersPtr, ChannelInfoPtr> ParametersToChannelInfoMap;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::RIDInfo
      //

      struct RIDInfo
      {
        String mRID;
        ChannelInfoPtr mChannelInfo;

        ElementPtr toDebug() const noexcept;
      };

      typedef std::map<RID, RIDInfo> RIDToChannelMap;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::SSRCInfo
      //

      struct SSRCInfo
      {
        SSRCType mSSRC {};
        RoutingPayloadType mRoutingPayload {};
        String mRID;
        Time mLastUsage;

        ChannelHolderPtr mChannelHolder;      // can be NULL

        SSRCInfo() noexcept;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::CodecInfo
      //

      struct CodecInfo
      {
        PayloadType mPayloadType {};
        CodecTypes mCodecType {CodecType_First};
        CodecInfo *mOriginalCodec {};  // used for rtx to point to original codec
        IRTPTypes::CodecParameters *mOriginalCodecParams {};

        ElementPtr toDebug() const noexcept;
      };

      typedef std::map<PayloadType, CodecInfo> CodecInfoMap;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver::States
      //

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state) noexcept;

    public:
      RTPReceiver(
                  const make_private &,
                  IMessageQueuePtr queue,
                  IRTPReceiverDelegatePtr delegate,
                  Kinds kind,
                  IRTPTransportPtr transport,
                  IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                  ) noexcept(false); // throws InvalidParameters

    protected:
      RTPReceiver(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~RTPReceiver() noexcept;

      static RTPReceiverPtr convert(IRTPReceiverPtr object) noexcept;
      static RTPReceiverPtr convert(ForRTPListenerPtr object) noexcept;
      static RTPReceiverPtr convert(ForRTPReceiverChannelPtr object) noexcept;
      static RTPReceiverPtr convert(ForMediaStreamTrackPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IRTPReceiver
      //

      static ElementPtr toDebug(RTPReceiverPtr receiver) noexcept;

      static RTPReceiverPtr create(
                                   IRTPReceiverDelegatePtr delegate,
                                   Kinds kind,
                                   IRTPTransportPtr transport,
                                   IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                   ) noexcept;

      PUID getID() const noexcept override {return mID;}

      IRTPReceiverSubscriptionPtr subscribe(IRTPReceiverDelegatePtr delegate) noexcept override;

      ortc::IMediaStreamTrackPtr track() const noexcept override;
      IRTPTransportPtr transport() const noexcept override;
      IRTCPTransportPtr rtcpTransport() const noexcept override;

      void setTransport(
                        IRTPTransportPtr transport,
                        IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                        ) noexcept(false) override;   // throws InvalidParameters

      static CapabilitiesPtr getCapabilities(Optional<Kinds> kind) noexcept;

      PromisePtr receive(const Parameters &parameters) noexcept(false) override; // throws InvalidParameters
      void stop() noexcept override;

      ContributingSourceList getContributingSources() const noexcept override;

      void requestSendCSRC(SSRCType csrc) noexcept(false) override; // throws NotImplemented

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IRTPReceiverForRTPListener
      //

      // (duplciate) static ElementPtr toDebug(ForRTPListenerPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      bool handlePacket(
                        IICETypes::Components viaTransport,
                        RTPPacketPtr packet
                        ) noexcept override;

      bool handlePacket(
                        IICETypes::Components viaTransport,
                        RTCPPacketPtr packet
                        ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IRTPReceiverForRTPReceiverChannel
      //

      // (duplicate) static ElementPtr toDebug(ForRTPReceiverChannelPtr object) noexcept;

      // (duplicate) virtual PUID getID() const noexcept = 0;

      bool sendPacket(RTCPPacketPtr packet) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IRTPReceiverForMediaStreamTrack
      //

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object) noexcept;

      // (duplicate) virtual PUID getID() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => ISecureTransportDelegate
      //

      void onSecureTransportStateChanged(
                                         ISecureTransportPtr transport,
                                         ISecureTransport::States state
                                         ) override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => IRTPReceiverAsyncDelegate
      //

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => (friend RTPReceiver::ChannelHolder)
      //

      void notifyChannelGone() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPReceiver => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      bool stepAttemptDelivery() noexcept;
      bool stepCleanChannels() noexcept;

      void cancel() noexcept;

      void setState(States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

      bool shouldLatchAll() noexcept;
      void notifyChannelsOfTransportState() noexcept;

      void flushAllAutoLatchedChannels() noexcept;

      void addChannel(ParametersPtr params) noexcept;
      void updateChannel(
                         ChannelInfoPtr channelInfo,
                         ParametersPtr newParams
                         ) noexcept;
      void removeChannel(const ChannelInfo &channelInfo) noexcept;

      void registerHeaderExtensions(const Parameters &params) noexcept;

      SSRCInfoPtr setSSRCUsage(
                               SSRCType ssrc,
                               RoutingPayloadType routingPayload,
                               String &ioRID,
                               ChannelHolderPtr &ioChannelHolder
                               ) noexcept;

      void setRIDUsage(
                       const String &rid,
                       ChannelInfoPtr &ioChannelInfo
                       ) noexcept;

      void registerSSRCUsage(SSRCInfoPtr ssrcInfo) noexcept;

      void registerUsage(
                         const IRTPTypes::EncodingParameters &encodingParams,
                         ChannelInfoPtr channelInfo,
                         ChannelHolderPtr &ioChannelHolder
                         ) noexcept;

      void reattemptDelivery() noexcept;
      void expireRTPPackets() noexcept;

      bool shouldCleanChannel(bool objectExists) noexcept;
      void cleanChannels() noexcept;

      bool findMapping(
                       const RTPPacket &rtpPacket,
                       ChannelHolderPtr &outChannelHolder,
                       String &outRID
                       ) noexcept;

      String extractRID(
                        RoutingPayloadType routingPayload,
                        const RTPPacket &rtpPacket,
                        ChannelHolderPtr &outChannelHolder
                        ) noexcept;

      String extractMuxID(const RTPPacket &rtpPacket) noexcept;

      bool findMappingUsingRID(
                               const String &ridID,
                               RoutingPayloadType routingPayload,
                               const RTPPacket &rtpPacket,
                               ChannelInfoPtr &outChannelInfo,
                               ChannelHolderPtr &outChannelHolder
                               ) noexcept;

      bool findMappingUsingSSRCInEncodingParams(
                                                const String &rid,
                                                RoutingPayloadType routingPayload,
                                                const RTPPacket &rtpPacket,
                                                ChannelInfoPtr &outChannelInfo,
                                                ChannelHolderPtr &outChannelHolder
                                                ) noexcept;

      bool findMappingUsingPayloadType(
                                       const String &rid,
                                       RoutingPayloadType routingPayload,
                                       const RTPPacket &rtpPacket,
                                       ChannelInfoPtr &outChannelInfo,
                                       ChannelHolderPtr &outChannelHolder
                                       ) noexcept;

      bool findBestExistingLatchAllOrCreateNew(
                                               const RTPTypesHelper::DecodedCodecInfo &decodedCodec,
                                               const String &rid,
                                               RoutingPayloadType routingPayload,
                                               const RTPPacket &rtpPacket,
                                               ChannelInfoPtr &outChannelInfo,
                                               ChannelHolderPtr &outChannelHolder
                                               ) noexcept;

      bool fillRIDParameters(
                             const String &rid,
                             ChannelInfoPtr &ioChannelInfo
                             ) noexcept;

      void createChannel(
                         SSRCType ssrc,
                         RoutingPayloadType routingPayload,
                         const String &rid,
                         ChannelInfoPtr channelInfo,
                         ChannelHolderPtr &ioChannelHolder
                         ) noexcept;

      void processUnhandled(
                            const String &muxID,
                            const String &rid,
                            IRTPTypes::SSRCType ssrc,
                            IRTPTypes::PayloadType payloadType
                            ) noexcept;

      void processByes(const RTCPPacket &rtcpPacket) noexcept;
      void processSenderReports(const RTCPPacket &rtcpPacket) noexcept;

      void extractCSRCs(const RTPPacket &rtpPacket) noexcept;
      void setContributingSource(
                                 SSRCType csrc,
                                 BYTE level,
                                 const Optional<bool> &voiceActivityFlag
                                 ) noexcept;

      void postFindMappingProcessPacket(
                                        const RTPPacket &rtpPacket,
                                        ChannelHolderPtr &channelHolder
                                        ) noexcept;

      void resetActiveReceiverChannel() noexcept;

      Optional<RoutingPayloadType> decodeREDRoutingPayloadType(
                                                               const BYTE *buffer,
                                                               size_t bufferSizeInBytes
                                                               ) noexcept;

      Optional<RoutingPayloadType> decodeFECPayloadType(
                                                        const BYTE *buffer,
                                                        size_t bufferSizeInBytes
                                                        ) noexcept;

      Optional<RoutingPayloadType> getRoutingPayloadType(const RTPPacket &rtpPacket) noexcept;

      Optional<RoutingPayloadType> getRoutingPayload(const RTPTypesHelper::DecodedCodecInfo &decodedCodec) noexcept;

      RoutingPayloadType getMediaCodecRoutingPayload(PayloadType originalPayload) noexcept;

      void getFECMediaCodecRoutingPayload(
                                          PayloadType originalPayload,
                                          IRTPTypes::KnownFECMechanisms mechanism,
                                          Optional<RoutingPayloadType> &resultFECv1,
                                          Optional<RoutingPayloadType> &resultFECv2,
                                          PayloadType *outPayloadTypeFECv1 = NULL,
                                          PayloadType *outPayloadTypeFECv2 = NULL
                                          ) noexcept;

      Optional<RoutingPayloadType> getRtxMediaCodecRoutingPayload(PayloadType originalPayload) noexcept;
      void getRtxFECMediaCodecRoutingPayload(
                                             PayloadType originalPayload,
                                             IRTPTypes::KnownFECMechanisms mechanism,
                                             Optional<RoutingPayloadType> &resultFECv1,
                                             Optional<RoutingPayloadType> &resultFECv2
                                             ) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPReceiver => (data)
      //

      AutoPUID mID;
      RTPReceiverWeakPtr mThisWeak;
      RTPReceiverPtr mGracefulShutdownReference;

      IRTPReceiverDelegateSubscriptions mSubscriptions;
      IRTPReceiverSubscriptionPtr mDefaultSubscription;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      IMediaStreamTrack::Kinds mKind {IMediaStreamTrack::Kind_First};
      UseMediaStreamTrackPtr mTrack;

      ParametersPtr mParameters;
      CodecInfoMap mCodecInfos;

      UseListenerPtr mListener;

      UseSecureTransportPtr mRTPTransport;
      UseSecureTransportPtr mRTCPTransport;

      ISecureTransportSubscriptionPtr mRTCPTransportSubscription;

      IICETypes::Components mReceiveRTPOverTransport {IICETypes::Component_RTP};
      IICETypes::Components mReceiveRTCPOverTransport {IICETypes::Component_RTCP};
      IICETypes::Components mSendRTCPOverTransport {IICETypes::Component_RTCP};

      ISecureTransport::States mLastReportedTransportStateToChannels {ISecureTransport::State_Pending};

      ParametersPtrList mParametersGroupedIntoChannels;

      ChannelWeakMapPtr mChannels;                 // COW pattern, always valid ptr
      bool mCleanChannels {false};

      ParametersToChannelInfoMap mChannelInfos;

      HeaderExtensionMap mRegisteredExtensions;

      SSRCRoutingMap mSSRCRoutingPayloadTable;
      SSRCRoutingWeakMap mRegisteredSSRCRoutingPayloads;

      RIDToChannelMap mRIDTable;

      ITimerPtr mSSRCTableTimer;
      Seconds mSSRCTableExpires {};

      size_t mMaxBufferedRTPPackets {};
      Seconds mMaxRTPPacketAge {};

      BufferedRTPPacketList mBufferedRTPPackets;
      bool mReattemptRTPDelivery {false};

      ContributingSourceMap mContributingSources;
      Seconds mContributingSourcesExpiry {};
      ITimerPtr mContributingSourcesTimer;

      ChannelHolderPtr mCurrentChannel;
      Time mLastSwitchedCurrentChannel;
      Milliseconds mLockAfterSwitchTime {};

      Milliseconds mAmbiguousPayloadMappingMinDifference {};
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverFactory
    //

    interaction IRTPReceiverFactory
    {
      typedef IRTPReceiverTypes::Kinds Kinds;
      typedef IRTPReceiverTypes::CapabilitiesPtr CapabilitiesPtr;

      static IRTPReceiverFactory &singleton() noexcept;

      virtual RTPReceiverPtr create(
                                    IRTPReceiverDelegatePtr delegate,
                                    Kinds kind,
                                    IRTPTransportPtr transport,
                                    IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                    ) noexcept;

      virtual CapabilitiesPtr getCapabilities(Optional<Kinds> kind) noexcept;
    };

    class RTPReceiverFactory : public IFactory<IRTPReceiverFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPReceiverAsyncDelegate)
//ZS_DECLARE_PROXY_METHOD(onDeliverPacket, UseChannelPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_END()
