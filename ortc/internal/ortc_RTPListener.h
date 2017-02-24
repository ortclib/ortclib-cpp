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

#include <ortc/IRTPListener.h>
#include <ortc/IMediaStreamTrack.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>
#include <zsLib/TearAway.h>

#define ORTC_SETTING_RTP_LISTENER_MAX_RTP_PACKETS_IN_BUFFER "ortc/rtp-listener/max-rtp-packets-in-buffer"
#define ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTP_PACKETS_IN_SECONDS "ortc/rtp-listener/max-age-rtp-packets-in-seconds"

#define ORTC_SETTING_RTP_LISTENER_MAX_RTCP_PACKETS_IN_BUFFER "ortc/rtp-listener/max-rtcp-packets-in-buffer"
#define ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS "ortc/rtp-listener/max-age-rtcp-packets-in-seconds"

#define ORTC_SETTING_RTP_LISTENER_SSRC_TIMEOUT_IN_SECONDS "ortc/rtp-listener/ssrc-timeout-in-seconds"
#define ORTC_SETTING_RTP_LISTENER_UNHANDLED_EVENTS_TIMEOUT_IN_SECONDS "ortc/rtp-listener/unhandled-ssrc-event-timeout-in-seconds"

#define ORTC_SETTING_RTP_LISTENER_ONLY_RESOLVE_AMBIGUOUS_PAYLOAD_MAPPING_IF_ACTIVITY_DIFFERS_IN_MILLISECONDS "ortc/rtp-listener/only-resolve-ambiguous-payload-mapping-if-activity-differs-in-milliseconds"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPListener)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForRTPListener)

    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPReceiver)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPSender)

    ZS_DECLARE_INTERACTION_PROXY(IRTPListenerAsyncDelegate)

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForSecureTransport
    #pragma mark

    interaction IRTPListenerForSecureTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForSecureTransport, ForSecureTransport)

      static ElementPtr toDebug(ForSecureTransportPtr listener);

      static RTPListenerPtr create(IRTPTransportPtr transport);

      virtual PUID getID() const = 0;

      virtual bool handleRTPPacket(
                                   IICETypes::Components viaComponent,
                                   IICETypes::Components packetType,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForRTPReceiver
    #pragma mark

    interaction IRTPListenerForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPReceiver, ForRTPReceiver)

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, UseReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;

      static ElementPtr toDebug(ForRTPReceiverPtr listener);

      static ForRTPReceiverPtr getListener(IRTPTransportPtr rtpTransport);

      virtual PUID getID() const = 0;

      virtual void registerReceiver(
                                    Optional<IMediaStreamTrack::Kinds> kind,
                                    UseReceiverPtr inReceiver,
                                    const Parameters &inParams,
                                    RTCPPacketList *outPacketList = NULL
                                    ) = 0;

      virtual void unregisterReceiver(UseReceiver &inReceiver) = 0;

      virtual void getPackets(RTCPPacketList &outPacketList) = 0;

      virtual void notifyUnhandled(
                                   const String &muxID,
                                   const String &rid,
                                   IRTPTypes::SSRCType ssrc,
                                   IRTPTypes::PayloadType payloadType
                                   ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForRTPSender
    #pragma mark

    interaction IRTPListenerForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPSender, ForRTPSender)

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPListener, UseSender)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;

      static ElementPtr toDebug(ForRTPSenderPtr listener);

      static ForRTPSenderPtr getListener(IRTPTransportPtr rtpTransport);

      virtual PUID getID() const = 0;

      virtual void registerSender(
                                  UseSenderPtr inSender,
                                  const Parameters &inParams,
                                  RTCPPacketList &outPacketList
                                  ) = 0;

      virtual void unregisterSender(UseSender &inSender) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerAsyncDelegate
    #pragma mark

    interaction IRTPListenerAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, UseRTPReceiver)

      virtual void onDeliverPacket(
                                   IICETypes::Components viaComponent,
                                   UseRTPReceiverPtr receiver,
                                   RTPPacketPtr packet
                                   ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener
    #pragma mark

    class RTPListener : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IRTPListener,
                        public IRTPListenerForSecureTransport,
                        public IRTPListenerForRTPReceiver,
                        public IRTPListenerForRTPSender,
                        public IWakeDelegate,
                        public zsLib::ITimerDelegate,
                        public IRTPListenerAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPListener;
      friend interaction IRTPListenerFactory;
      friend interaction IRTPListenerForSecureTransport;
      friend interaction IRTPListenerForRTPReceiver;
      friend interaction IRTPListenerForRTPSender;

      ZS_DECLARE_STRUCT_PTR(TearAwayData)
      ZS_DECLARE_STRUCT_PTR(RegisteredHeaderExtension)
      ZS_DECLARE_STRUCT_PTR(ReceiverInfo)
      ZS_DECLARE_STRUCT_PTR(SSRCInfo)
      ZS_DECLARE_STRUCT_PTR(UnhandledEventInfo)

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, UseRTPReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPListener, UseRTPSender)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTransport, UseRTPTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPListener, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      typedef std::list<RTCPPacketPtr> RTCPPacketList;

      typedef std::pair<Time, RTPPacketPtr> TimeRTPPacketPair;
      typedef std::list<TimeRTPPacketPair> BufferedRTPPacketList;

      typedef std::pair<Time, RTCPPacketPtr> TimeRTCPPacketPair;
      typedef std::list<TimeRTCPPacketPair> BufferedRTCPPacketList;

      typedef std::map<SSRCType, SSRCInfoPtr> SSRCMap;
      typedef std::map<SSRCType, SSRCInfoWeakPtr> SSRCWeakMap;

      typedef PUID ObjectID;
      typedef USHORT LocalID;
      typedef size_t ReferenceCount;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener::RegisteredHeaderExtension
      #pragma mark

      struct RegisteredHeaderExtension
      {
        typedef std::map<ObjectID, bool> ReferenceMap;

        HeaderExtensionURIs mHeaderExtensionURI {HeaderExtensionURI_Unknown};
        LocalID mLocalID {};
        bool mEncrypted {};

        ReferenceMap mReferences;

        ElementPtr toDebug() const;
      };

      typedef std::map<LocalID, RegisteredHeaderExtension> HeaderExtensionMap;

      const ObjectID kAPIReference {0};

      typedef ObjectID ReceiverID;
      typedef ObjectID SenderID;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener::ReceiverInfo
      #pragma mark

      struct ReceiverInfo
      {
        PUID mOrderID {};
        ReceiverID mReceiverID {};
        UseRTPReceiverWeakPtr mReceiver;

        Optional<IMediaStreamTrack::Kinds> mKind;
        Parameters mFilledParameters;
        Parameters mOriginalParameters;

        SSRCMap mRegisteredSSRCs;

        SSRCInfoPtr registerSSRCUsage(SSRCInfoPtr ssrcInfo);
        void unregisterSSRCUsage(SSRCType ssrc);

        ElementPtr toDebug() const;
      };

      typedef std::map<ReceiverID, ReceiverInfoPtr> ReceiverObjectMap;
      typedef std::map<SenderID, UseSenderWeakPtr> SenderObjectMap;
      ZS_DECLARE_PTR(ReceiverObjectMap)
      ZS_DECLARE_PTR(SenderObjectMap)

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener::SSRCInfo
      #pragma mark

      struct SSRCInfo
      {
        SSRCType mSSRC {};
        Time mLastUsage;
        String mMuxID;

        ReceiverInfoPtr mReceiverInfo;    // can be NULL

        SSRCInfo();
        ElementPtr toDebug() const;
      };

      typedef String MuxID;
      typedef std::map<MuxID, ReceiverInfoPtr> MuxIDMap;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener::UnhandledEventInfo
      #pragma mark

      struct UnhandledEventInfo
      {
        SSRCType mSSRC {};
        PayloadType mCodecPayloadType {};
        String mMuxID;
        String mRID;

        bool operator<(const UnhandledEventInfo &) const;

        ElementPtr toDebug() const;
      };

      typedef std::map<struct UnhandledEventInfo, Time> UnhandledEventMap;

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPListener(
                  const make_private &,
                  IMessageQueuePtr queue,
                  IRTPListenerDelegatePtr delegate,
                  UseRTPTransportPtr transport
                  );

    protected:
      RTPListener(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPListener();

      static RTPListenerPtr convert(IRTPListenerPtr object);
      static RTPListenerPtr convert(ForSecureTransportPtr object);
      static RTPListenerPtr convert(ForRTPReceiverPtr object);
      static RTPListenerPtr convert(ForRTPSenderPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListener
      #pragma mark

      static ElementPtr toDebug(RTPListenerPtr listener);

      static IRTPListenerPtr create(
                                    IRTPListenerDelegatePtr delegate,
                                    IRTPTransportPtr transport,
                                    Optional<HeaderExtensionParametersList> headerExtensions
                                    );

      virtual PUID getID() const override {return mID;}

      virtual IRTPListenerSubscriptionPtr subscribe(IRTPListenerDelegatePtr delegate) override;

      virtual IRTPTransportPtr transport() const override;

      virtual void setHeaderExtensions(const HeaderExtensionParametersList &headerExtensions) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerForSecureTransport
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForSecureTransportPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      static RTPListenerPtr create(IRTPTransportPtr transport);

      virtual bool handleRTPPacket(
                                   IICETypes::Components viaComponent,
                                   IICETypes::Components packetType,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerForRTPReceiver
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPReceiverPtr listener);

      // (duplicate) virtual PUID getID() const = 0;

      virtual void registerReceiver(
                                    Optional<IMediaStreamTrack::Kinds> kind,
                                    UseReceiverPtr inReceiver,
                                    const Parameters &inParams,
                                    RTCPPacketList *outPacketList = NULL
                                    ) override;

      virtual void unregisterReceiver(UseReceiver &inReceiver) override;

      virtual void getPackets(RTCPPacketList &outPacketList) override;

      virtual void notifyUnhandled(
                                   const String &muxID,
                                   const String &rid,
                                   IRTPTypes::SSRCType ssrc,
                                   IRTPTypes::PayloadType payloadType
                                   ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerForRTPSender
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr listener);

      // (duplicate) virtual PUID getID() const = 0;

      virtual void registerSender(
                                  UseSenderPtr inSender,
                                  const Parameters &inParams,
                                  RTCPPacketList &outPacketList
                                  ) override;

      virtual void unregisterSender(UseSender &inReceiver) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerAsyncDelegate
      #pragma mark

      virtual void onDeliverPacket(
                                   IICETypes::Components viaComponent,
                                   UseRTPReceiverPtr receiver,
                                   RTPPacketPtr packet
                                   ) override;

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener::TearAwayData
      #pragma mark

      struct TearAwayData
      {
        UseRTPTransportPtr mRTPTransport;
        IRTPListenerSubscriptionPtr mDefaultSubscription;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => (internal)
      #pragma mark

      static Log::Params slog(const char *message);
      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepAttemptDelivery();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

      void expireRTPPackets();
      void expireRTCPPackets();

      void registerHeaderExtensionReference(
                                            PUID objectID,
                                            HeaderExtensionURIs extensionURI,
                                            LocalID localID,
                                            bool encrytped
                                            );

      void unregisterAllHeaderExtensionReferences(PUID objectID);

      bool findMapping(
                       const RTPPacket &rtpPacket,
                       ReceiverInfoPtr &outReceiverInfo,
                       String &outMuxID
                       );

      bool findMappingUsingMuxID(
                                 const String &muxID,
                                 const RTPPacket &rtpPacket,
                                 ReceiverInfoPtr &outReceiverInfo
                                 );

      bool findMappingUsingSSRCInEncodingParams(
                                                const String &muxID,
                                                const RTPPacket &rtpPacket,
                                                ReceiverInfoPtr &outReceiverInfo
                                                );

      bool findMappingUsingPayloadType(
                                       const String &muxID,
                                       const RTPPacket &rtpPacket,
                                       ReceiverInfoPtr &outReceiverInfo
                                       );

      String extractMuxID(
                          const RTPPacket &rtpPacket,
                          ReceiverInfoPtr &ioReceiverInfo
                          );
      String extractRID(const RTPPacket &rtpPacket);

      bool fillMuxIDParameters(
                               const String &muxID,
                               ReceiverInfoPtr &ioReceiverInfo
                               );

      void setReceiverInfo(ReceiverInfoPtr receiverInfo);

      void processByes(const RTCPPacket &rtcpPacket);
      void processSDESMid(const RTCPPacket &rtcpPacket);
      void processSenderReports(const RTCPPacket &rtcpPacket);

      void handleDeltaChanges(
                              ReceiverInfoPtr replacementInfo,
                              const EncodingParameters &existing,
                              EncodingParameters &ioReplacement
                              );

      void unregisterEncoding(const EncodingParameters &existing);

      SSRCInfoPtr setSSRCUsage(
                               SSRCType ssrc,
                               String &ioMuxID,
                               ReceiverInfoPtr &ioReceiverInfo
                               );
      void registerSSRCUsage(SSRCInfoPtr ssrcInfo);

      void reattemptDelivery();

      void processUnhandled(
                            const String &muxID,
                            const String &rid,
                            IRTPTypes::SSRCType ssrc,
                            IRTPTypes::PayloadType payloadType,
                            const Time &tick
                            );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => (data)
      #pragma mark

      AutoPUID mID;
      RTPListenerWeakPtr mThisWeak;
      RTPListenerPtr mGracefulShutdownReference;

      IRTPListenerDelegateSubscriptions mSubscriptions;
      IRTPListenerSubscriptionPtr mDefaultSubscription;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseRTPTransportWeakPtr mRTPTransport;

      size_t mMaxBufferedRTPPackets {};
      Seconds mMaxRTPPacketAge {};

      size_t mMaxBufferedRTCPPackets {};
      Seconds mMaxRTCPPacketAge {};

      BufferedRTPPacketList mBufferedRTPPackets;
      BufferedRTCPPacketList mBufferedRTCPPackets;

      HeaderExtensionMap mRegisteredExtensions;

      ReceiverObjectMapPtr mReceivers;  // non-mutable map values (COW)
      SenderObjectMapPtr mSenders;      // non-mutable map values (COW)

      SSRCMap mSSRCTable;
      SSRCWeakMap mRegisteredSSRCs;

      MuxIDMap mMuxIDTable;

      ITimerPtr mSSRCTableTimer;
      Seconds mSSRCTableExpires {};

      bool mReattemptRTPDelivery {};

      UnhandledEventMap mUnhandledEvents;
      ITimerPtr mUnhanldedEventsTimer;
      Seconds mUnhandledEventsExpires {};

      Milliseconds mAmbiguousPayloadMappingMinDifference {};
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerFactory
    #pragma mark

    interaction IRTPListenerFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionParametersList, HeaderExtensionParametersList)

      static IRTPListenerFactory &singleton();

      virtual IRTPListenerPtr create(
                                     IRTPListenerDelegatePtr delegate,
                                     IRTPTransportPtr transport,
                                     Optional<HeaderExtensionParametersList> headerExtensions = Optional<HeaderExtensionParametersList>()
                                     );

      virtual RTPListenerPtr create(IRTPTransportPtr transport);
    };

    class RTPListenerFactory : public IFactory<IRTPListenerFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPListenerAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETypes::Components, Components)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPListenerAsyncDelegate::UseRTPReceiverPtr, UseRTPReceiverPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD_3(onDeliverPacket, Components, UseRTPReceiverPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_END()




ZS_DECLARE_TEAR_AWAY_BEGIN(ortc::IRTPListener, ortc::internal::RTPListener::TearAwayData)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::InvalidStateError, InvalidStateError)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPListenerSubscriptionPtr, IRTPListenerSubscriptionPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPListenerDelegatePtr, IRTPListenerDelegatePtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPTransportPtr, IRTPTransportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPTypes::HeaderExtensionParametersList, HeaderExtensionParametersList)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_1(getStats, PromiseWithStatsReportPtr, const StatsTypeSet &)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_1(subscribe, IRTPListenerSubscriptionPtr, IRTPListenerDelegatePtr)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(transport, IRTPTransportPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_1(setHeaderExtensions, const HeaderExtensionParametersList &)
ZS_DECLARE_TEAR_AWAY_END()
