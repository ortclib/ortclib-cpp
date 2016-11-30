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
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPListener)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForMediaStreamTrack)

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForRTPReceiver)

    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForRTPReceiver)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPReceiver)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPReceiverChannel)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiver)

    ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverForRTPListener
    #pragma mark

    interaction IRTPReceiverForRTPListener
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, ForRTPListener)

      static ElementPtr toDebug(ForRTPListenerPtr transport);

      virtual PUID getID() const = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTPPacketPtr packet
                                ) = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTCPPacketPtr packet
                                ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverForRTPReceiverChannel
    #pragma mark

    interaction IRTPReceiverForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPReceiverChannel, ForRTPReceiverChannel)

      static ElementPtr toDebug(ForRTPReceiverChannelPtr object);

      virtual PUID getID() const = 0;

      virtual bool sendPacket(RTCPPacketPtr packet) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverForMediaStreamTrack
    #pragma mark

    interaction IRTPReceiverForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForMediaStreamTrack, ForMediaStreamTrack)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverAsyncDelegate
    #pragma mark

    interaction IRTPReceiverAsyncDelegate
    {
      virtual ~IRTPReceiverAsyncDelegate() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver
    #pragma mark
    
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

      static const char *toString(CodecTypes codecType);
      static CodecTypes toCodecType(const char *type);

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
      #pragma mark
      #pragma mark RTPReceiver::RegisteredHeaderExtension
      #pragma mark

      typedef USHORT LocalID;
      typedef size_t ReferenceCount;

      struct RegisteredHeaderExtension
      {
        HeaderExtensionURIs mHeaderExtensionURI {HeaderExtensionURI_Unknown};
        LocalID mLocalID {};
        bool mEncrypted {};

        ElementPtr toDebug() const;
      };

      typedef std::map<LocalID, RegisteredHeaderExtension> HeaderExtensionMap;

      typedef std::map<SSRCType, ContributingSource> ContributingSourceMap;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver::ChannelHolder
      #pragma mark

      struct ChannelHolder
      {
        RTPReceiverWeakPtr mHolder;
        UseChannelPtr mChannel;
        ChannelInfoPtr mChannelInfo;
        std::atomic<ISecureTransport::States> mLastReportedState {ISecureTransport::State_Pending};

        ChannelHolder();
        ~ChannelHolder();

        PUID getID() const;
        void notify(ISecureTransport::States state);

        void notify(RTPPacketPtr packet);
        void notify(RTCPPacketListPtr packets);

        void update(const Parameters &params);

        bool handle(RTPPacketPtr packet);
        bool handle(RTCPPacketPtr packet);

        void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats);

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver::ChannelInfo
      #pragma mark

      struct ChannelInfo
      {
        AutoPUID mID;

        ParametersPtr mOriginalParameters;
        ParametersPtr mFilledParameters;

        ChannelHolderWeakPtr mChannelHolder;  // NOTE: might be null if channel is not created yet (or previously destoyed)
        SSRCRoutingMap mRegisteredSSRCs;

        bool shouldLatchAll() const;
        String rid() const;

        void registerHolder(ChannelHolderPtr channelHolder);

        SSRCInfoPtr registerSSRCUsage(SSRCInfoPtr ssrcInfo);
        void unregisterSSRCUsage(
                                 SSRCType ssrc,
                                 RoutingPayloadType routingPayload
                                 );

        ElementPtr toDebug() const;
      };

      typedef std::map<ParametersPtr, ChannelInfoPtr> ParametersToChannelInfoMap;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver::RIDInfo
      #pragma mark

      struct RIDInfo
      {
        String mRID;
        ChannelInfoPtr mChannelInfo;

        ElementPtr toDebug() const;
      };

      typedef std::map<RID, RIDInfo> RIDToChannelMap;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver::SSRCInfo
      #pragma mark

      struct SSRCInfo
      {
        SSRCType mSSRC {};
        RoutingPayloadType mRoutingPayload {};
        String mRID;
        Time mLastUsage;

        ChannelHolderPtr mChannelHolder;      // can be NULL

        SSRCInfo();

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver::CodecInfo
      #pragma mark

      struct CodecInfo
      {
        PayloadType mPayloadType {};
        CodecTypes mCodecType {CodecType_First};
        CodecInfo *mOriginalCodec {};  // used for rtx to point to original codec
        IRTPTypes::CodecParameters *mOriginalCodecParams {};

        ElementPtr toDebug() const;
      };

      typedef std::map<PayloadType, CodecInfo> CodecInfoMap;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver::States
      #pragma mark

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPReceiver(
                  const make_private &,
                  IMessageQueuePtr queue,
                  IRTPReceiverDelegatePtr delegate,
                  Kinds kind,
                  IRTPTransportPtr transport,
                  IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                  );

    protected:
      RTPReceiver(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPReceiver();

      static RTPReceiverPtr convert(IRTPReceiverPtr object);
      static RTPReceiverPtr convert(ForRTPListenerPtr object);
      static RTPReceiverPtr convert(ForRTPReceiverChannelPtr object);
      static RTPReceiverPtr convert(ForMediaStreamTrackPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IRTPReceiver
      #pragma mark

      static ElementPtr toDebug(RTPReceiverPtr receiver);

      static RTPReceiverPtr create(
                                   IRTPReceiverDelegatePtr delegate,
                                   Kinds kind,
                                   IRTPTransportPtr transport,
                                   IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                   );

      virtual PUID getID() const override {return mID;}

      virtual IRTPReceiverSubscriptionPtr subscribe(IRTPReceiverDelegatePtr delegate) override;

      virtual IMediaStreamTrackPtr track() const override;
      virtual IRTPTransportPtr transport() const override;
      virtual IRTCPTransportPtr rtcpTransport() const override;

      virtual void setTransport(
                                IRTPTransportPtr transport,
                                IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                ) override;

      static CapabilitiesPtr getCapabilities(Optional<Kinds> kind);

      virtual PromisePtr receive(const Parameters &parameters) override;
      virtual void stop() override;

      virtual ContributingSourceList getContributingSources() const override;

      virtual void requestSendCSRC(SSRCType csrc) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IRTPReceiverForRTPListener
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForRTPListenerPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTPPacketPtr packet
                                ) override;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTCPPacketPtr packet
                                ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IRTPReceiverForRTPReceiverChannel
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPReceiverChannelPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool sendPacket(RTCPPacketPtr packet) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IRTPReceiverForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => ISecureTransportDelegate
      #pragma mark

      virtual void onSecureTransportStateChanged(
                                                 ISecureTransportPtr transport,
                                                 ISecureTransport::States state
                                                 ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IRTPReceiverAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => (friend RTPReceiver::ChannelHolder)
      #pragma mark

      void notifyChannelGone();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepAttemptDelivery();
      bool stepCleanChannels();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

      bool shouldLatchAll();
      void notifyChannelsOfTransportState();

      void flushAllAutoLatchedChannels();

      void addChannel(ParametersPtr params);
      void updateChannel(
                         ChannelInfoPtr channelInfo,
                         ParametersPtr newParams
                         );
      void removeChannel(const ChannelInfo &channelInfo);

      void registerHeaderExtensions(const Parameters &params);

      SSRCInfoPtr setSSRCUsage(
                               SSRCType ssrc,
                               RoutingPayloadType routingPayload,
                               String &ioRID,
                               ChannelHolderPtr &ioChannelHolder
                               );

      void setRIDUsage(
                       const String &rid,
                       ChannelInfoPtr &ioChannelInfo
                       );

      void registerSSRCUsage(SSRCInfoPtr ssrcInfo);

      void registerUsage(
                         const IRTPTypes::EncodingParameters &encodingParams,
                         ChannelInfoPtr channelInfo,
                         ChannelHolderPtr &ioChannelHolder
                         );

      void reattemptDelivery();
      void expireRTPPackets();

      bool shouldCleanChannel(bool objectExists);
      void cleanChannels();

      bool findMapping(
                       const RTPPacket &rtpPacket,
                       ChannelHolderPtr &outChannelHolder,
                       String &outRID
                       );

      String extractRID(
                        RoutingPayloadType routingPayload,
                        const RTPPacket &rtpPacket,
                        ChannelHolderPtr &outChannelHolder
                        );

      String extractMuxID(const RTPPacket &rtpPacket);

      bool findMappingUsingRID(
                               const String &ridID,
                               RoutingPayloadType routingPayload,
                               const RTPPacket &rtpPacket,
                               ChannelInfoPtr &outChannelInfo,
                               ChannelHolderPtr &outChannelHolder
                               );

      bool findMappingUsingSSRCInEncodingParams(
                                                const String &rid,
                                                RoutingPayloadType routingPayload,
                                                const RTPPacket &rtpPacket,
                                                ChannelInfoPtr &outChannelInfo,
                                                ChannelHolderPtr &outChannelHolder
                                                );

      bool findMappingUsingPayloadType(
                                       const String &rid,
                                       RoutingPayloadType routingPayload,
                                       const RTPPacket &rtpPacket,
                                       ChannelInfoPtr &outChannelInfo,
                                       ChannelHolderPtr &outChannelHolder
                                       );

      bool findBestExistingLatchAllOrCreateNew(
                                               const RTPTypesHelper::DecodedCodecInfo &decodedCodec,
                                               const String &rid,
                                               RoutingPayloadType routingPayload,
                                               const RTPPacket &rtpPacket,
                                               ChannelInfoPtr &outChannelInfo,
                                               ChannelHolderPtr &outChannelHolder
                                               );

      bool fillRIDParameters(
                             const String &rid,
                             ChannelInfoPtr &ioChannelInfo
                             );

      void createChannel(
                         SSRCType ssrc,
                         RoutingPayloadType routingPayload,
                         const String &rid,
                         ChannelInfoPtr channelInfo,
                         ChannelHolderPtr &ioChannelHolder
                         );

      void processUnhandled(
                            const String &muxID,
                            const String &rid,
                            IRTPTypes::SSRCType ssrc,
                            IRTPTypes::PayloadType payloadType
                            );

      void processByes(const RTCPPacket &rtcpPacket);
      void processSenderReports(const RTCPPacket &rtcpPacket);

      void extractCSRCs(const RTPPacket &rtpPacket);
      void setContributingSource(
                                 SSRCType csrc,
                                 BYTE level,
                                 const Optional<bool> &voiceActivityFlag
                                 );

      void postFindMappingProcessPacket(
                                        const RTPPacket &rtpPacket,
                                        ChannelHolderPtr &channelHolder
                                        );

      void resetActiveReceiverChannel();

      Optional<RoutingPayloadType> decodeREDRoutingPayloadType(
                                                               const BYTE *buffer,
                                                               size_t bufferSizeInBytes
                                                               );

      Optional<RoutingPayloadType> decodeFECPayloadType(
                                                        const BYTE *buffer,
                                                        size_t bufferSizeInBytes
                                                        );

      Optional<RoutingPayloadType> getRoutingPayloadType(const RTPPacket &rtpPacket);

      Optional<RoutingPayloadType> getRoutingPayload(const RTPTypesHelper::DecodedCodecInfo &decodedCodec);

      RoutingPayloadType getMediaCodecRoutingPayload(PayloadType originalPayload);

      void getFECMediaCodecRoutingPayload(
                                          PayloadType originalPayload,
                                          IRTPTypes::KnownFECMechanisms mechanism,
                                          Optional<RoutingPayloadType> &resultFECv1,
                                          Optional<RoutingPayloadType> &resultFECv2,
                                          PayloadType *outPayloadTypeFECv1 = NULL,
                                          PayloadType *outPayloadTypeFECv2 = NULL
                                          );

      Optional<RoutingPayloadType> getRtxMediaCodecRoutingPayload(PayloadType originalPayload);
      void getRtxFECMediaCodecRoutingPayload(
                                             PayloadType originalPayload,
                                             IRTPTypes::KnownFECMechanisms mechanism,
                                             Optional<RoutingPayloadType> &resultFECv1,
                                             Optional<RoutingPayloadType> &resultFECv2
                                             );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IRTPReceiverFactory
    #pragma mark

    interaction IRTPReceiverFactory
    {
      typedef IRTPReceiverTypes::Kinds Kinds;
      typedef IRTPReceiverTypes::CapabilitiesPtr CapabilitiesPtr;

      static IRTPReceiverFactory &singleton();

      virtual RTPReceiverPtr create(
                                    IRTPReceiverDelegatePtr delegate,
                                    Kinds kind,
                                    IRTPTransportPtr transport,
                                    IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                    );

      virtual CapabilitiesPtr getCapabilities(Optional<Kinds> kind);
    };

    class RTPReceiverFactory : public IFactory<IRTPReceiverFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPReceiverAsyncDelegate)
//ZS_DECLARE_PROXY_METHOD_2(onDeliverPacket, UseChannelPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_END()
