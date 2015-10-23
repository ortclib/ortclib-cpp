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

#include <ortc/IICETransport.h>
#include <ortc/IRTPReceiver.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IMediaStreamTrack.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>

#define ORTC_SETTING_RTP_RECEIVER_SSRC_TIMEOUT_IN_SECONDS "ortc/rtp-receiver/ssrc-timeout-in-seconds"

#define ORTC_SETTING_RTP_RECEIVER_MAX_RTP_PACKETS_IN_BUFFER "ortc/rtp-receiver/max-rtp-packets-in-buffer"
#define ORTC_SETTING_RTP_RECEIVER_MAX_AGE_RTP_PACKETS_IN_SECONDS "ortc/rtp-receiver/max-age-rtp-packets-in-seconds"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForSettings)
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
    #pragma mark IRTPReceiverForSettings
    #pragma mark

    interaction IRTPReceiverForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPReceiverForSettings() {}
    };
    
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
                        public IRTPReceiverForSettings,
                        public IRTPReceiverForRTPListener,
                        public IRTPReceiverForRTPReceiverChannel,
                        public IRTPReceiverForMediaStreamTrack,
                        public IWakeDelegate,
                        public zsLib::ITimerDelegate,
                        public IRTPReceiverAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPReceiver;
      friend interaction IRTPReceiverFactory;
      friend interaction IRTPReceiverForSettings;
      friend interaction IRTPReceiverForRTPListener;
      friend interaction IRTPReceiverForRTPReceiverChannel;
      friend interaction IRTPReceiverForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPReceiver, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPReceiver, UseListener)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiver, UseChannel)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, UseMediaStreamTrack)

      ZS_DECLARE_STRUCT_PTR(RegisteredHeaderExtension)
      ZS_DECLARE_STRUCT_PTR(ChannelHolder)
      ZS_DECLARE_STRUCT_PTR(ChannelInfo)
      ZS_DECLARE_STRUCT_PTR(SSRCInfo)

      friend ChannelHolder;

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

      typedef std::map<SSRCType, SSRCInfoPtr> SSRCMap;
      typedef std::map<SSRCType, SSRCInfoWeakPtr> SSRCWeakMap;

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
        SSRCMap mRegisteredSSRCs;

        bool shouldLatchAll() const;
        String rid() const;

        void registerHolder(ChannelHolderPtr channelHolder);

        SSRCInfoPtr registerSSRCUsage(SSRCInfoPtr ssrcInfo);
        void unregisterSSRCUsage(SSRCType ssrc);

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
        String mRID;
        Time mLastUsage;

        ChannelHolderPtr mChannelHolder;      // can be NULL

        SSRCInfo();

        ElementPtr toDebug() const;
      };

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
                  IRTPTransportPtr transport,
                  IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                  );

    protected:
      RTPReceiver(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPReceiver();

      static RTPReceiverPtr convert(IRTPReceiverPtr object);
      static RTPReceiverPtr convert(ForSettingsPtr object);
      static RTPReceiverPtr convert(ForRTPListenerPtr object);
      static RTPReceiverPtr convert(ForRTPReceiverChannelPtr object);
      static RTPReceiverPtr convert(ForMediaStreamTrackPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => IRTPReceiver
      #pragma mark

      static ElementPtr toDebug(RTPReceiverPtr receiver);

      static RTPReceiverPtr create(
                                   IRTPReceiverDelegatePtr delegate,
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

      virtual void receive(const Parameters &parameters) override;
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
      #pragma mark RTPReceiver => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

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

      bool sendPacket(
                      IICETypes::Components packetType,
                      const BYTE *buffer,
                      size_t bufferSizeInBytes
                      );

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
                               String &ioRID,
                               ChannelHolderPtr &ioChannelHolder
                               );

      void setRIDUsage(
                       const String &rid,
                       ChannelInfoPtr &ioChannelInfo
                       );

      void registerSSRCUsage(SSRCInfoPtr ssrcInfo);

      void reattemptDelivery();
      void expireRTPPackets();

      bool shouldCleanChannel(bool shouldClean);
      void cleanChannels();

      bool findMapping(
                       const RTPPacket &rtpPacket,
                       ChannelHolderPtr &outChannelHolder,
                       String &outRID
                       );

      String extractRID(
                        const RTPPacket &rtpPacket,
                        ChannelHolderPtr &outChannelHolder
                        );

      String extractMuxID(const RTPPacket &rtpPacket);

      bool findMappingUsingRID(
                               const String &muridxID,
                               const RTPPacket &rtpPacket,
                               ChannelInfoPtr &outChannelInfo,
                               ChannelHolderPtr &outChannelHolder
                               );

      bool findMappingUsingSSRCInEncodingParams(
                                                const String &rid,
                                                const RTPPacket &rtpPacket,
                                                ChannelInfoPtr &outChannelInfo,
                                                ChannelHolderPtr &outChannelHolder
                                                );

      bool findMappingUsingPayloadType(
                                       const String &rid,
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
      
      UseListenerPtr mListener;

      UseSecureTransportPtr mRTPTransport;
      UseSecureTransportPtr mRTCPTransport;

      IICETypes::Components mReceiveRTPOverTransport {IICETypes::Component_RTP};
      IICETypes::Components mReceiveRTCPOverTransport {IICETypes::Component_RTCP};
      IICETypes::Components mSendRTCPOverTransport {IICETypes::Component_RTCP};

      ISecureTransport::States mLastReportedTransportStateToChannels {ISecureTransport::State_Pending};

      ParametersPtrList mParametersGroupedIntoChannels;

      ChannelWeakMapPtr mChannels;                 // COW pattern, always valid ptr
      bool mCleanChannels {false};

      ParametersToChannelInfoMap mChannelInfos;

      HeaderExtensionMap mRegisteredExtensions;

      SSRCMap mSSRCTable;
      SSRCWeakMap mRegisteredSSRCs;

      RIDToChannelMap mRIDTable;

      TimerPtr mSSRCTableTimer;
      Seconds mSSRCTableExpires {};

      size_t mMaxBufferedRTPPackets {};
      Seconds mMaxRTPPacketAge {};

      BufferedRTPPacketList mBufferedRTPPackets;
      bool mReattemptRTPDelivery {false};
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
