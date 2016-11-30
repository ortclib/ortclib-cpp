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

#include <ortc/IRTPSender.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IDTMFSender.h>
#include <ortc/IICETransport.h>
#include <ortc/IStatsReport.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForSettings);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForRTPListener);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForMediaStreamTrack);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForDTMFSender);

    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForRTPSender);
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPSender);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSender);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSender);

    ZS_DECLARE_INTERACTION_PROXY(IRTPSenderAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForSettings
    #pragma mark

    interaction IRTPSenderForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPSenderForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForRTPListener
    #pragma mark

    interaction IRTPSenderForRTPListener
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPListener, ForRTPListener)

      static ElementPtr toDebug(ForRTPListenerPtr transport);

      virtual PUID getID() const = 0;

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
    #pragma mark IRTPSenderForRTPSenderChannel
    #pragma mark

    interaction IRTPSenderForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPSenderChannel, ForRTPSenderChannel)

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, UseChannel)

      static ElementPtr toDebug(ForRTPSenderChannelPtr transport);

      virtual PUID getID() const = 0;

      virtual bool sendPacket(RTPPacketPtr packet) = 0;
      virtual bool sendPacket(RTCPPacketPtr packet) = 0;

      virtual void notifyConflict(
                                  UseChannelPtr channel,
                                  IRTPTypes::SSRCType ssrc,
                                  bool selfDestruct
                                  ) = 0;

      virtual void notifyDTMFSenderToneChanged(const char *tone) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForDTMFSender
    #pragma mark

    interaction IRTPSenderForDTMFSender : public IDTMFSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForDTMFSender, ForDTMFSender)

      static ElementPtr toDebug(ForDTMFSenderPtr transport);

      virtual PUID getID() const = 0;

      virtual IDTMFSenderSubscriptionPtr subscribe(IDTMFSenderDelegatePtr delegate) override { return subscribeDTMF(delegate); }

      virtual IDTMFSenderSubscriptionPtr subscribeDTMF(IDTMFSenderDelegatePtr delegate) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderForMediaStreamTrack
    #pragma mark

    interaction IRTPSenderForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForMediaStreamTrack, ForMediaStreamTrack)

      static ElementPtr toDebug(ForMediaStreamTrackPtr transport);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderAsyncDelegate
    #pragma mark

    interaction IRTPSenderAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, UseChannel)

      virtual void onDestroyChannel(UseChannelPtr channel) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender
    #pragma mark
    
    class RTPSender : public Noop,
                      public MessageQueueAssociator,
                      public SharedRecursiveLock,
                      public IRTPSender,
                      public IRTPSenderForSettings,
                      public IRTPSenderForRTPListener,
                      public IRTPSenderForRTPSenderChannel,
                      public IRTPSenderForDTMFSender,
                      public IRTPSenderForMediaStreamTrack,
                      public ISecureTransportDelegate,
                      public IWakeDelegate,
                      public IDTMFSenderDelegate,
                      public IRTPSenderAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPSender;
      friend interaction IRTPSenderFactory;
      friend interaction IRTPSenderForSettings;
      friend interaction IRTPSenderForRTPListener;
      friend interaction IRTPSenderForRTPSenderChannel;
      friend interaction IRTPSenderForDTMFSender;
      friend interaction IRTPSenderForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPSender, UseSecureTransport);
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPSender, UseListener);
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, UseChannel);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSender, UseMediaStreamTrack);

      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);

      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)

      ZS_DECLARE_STRUCT_PTR(ChannelHolder)

      typedef std::map<ParametersPtr, ChannelHolderPtr> ParametersToChannelHolderMap;
      ZS_DECLARE_PTR(ParametersToChannelHolderMap)

      ZS_DECLARE_TYPEDEF_PTR(std::list<ParametersPtr>, ParametersPtrList)

      typedef std::list<IRTPTypes::SSRCType> SSRCList;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender::ChannelHolder
      #pragma mark

      struct ChannelHolder
      {
        RTPSenderWeakPtr mHolder;
        UseChannelPtr mChannel;
        std::atomic<ISecureTransport::States> mLastReportedState {ISecureTransport::State_Pending};

        ChannelHolder();
        ~ChannelHolder();

        PUID getID() const;
        void notify(ISecureTransport::States state);

        void notify(RTCPPacketListPtr packets);

        void update(const Parameters &params);

        bool handle(RTCPPacketPtr packet);

        void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats);

        void insertDTMF(
                        const char *tones,
                        Milliseconds duration,
                        Milliseconds interToneGap
                        );

        String toneBuffer() const;
        Milliseconds duration() const;
        Milliseconds interToneGap() const;

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender::States
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
      RTPSender(
                const make_private &,
                IMessageQueuePtr queue,
                IRTPSenderDelegatePtr delegate,
                IMediaStreamTrackPtr track,
                IRTPTransportPtr transport,
                IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                );

    protected:
      RTPSender(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPSender();

      static RTPSenderPtr convert(IRTPSenderPtr object);
      static RTPSenderPtr convert(ForSettingsPtr object);
      static RTPSenderPtr convert(ForRTPListenerPtr object);
      static RTPSenderPtr convert(ForRTPSenderChannelPtr object);
      static RTPSenderPtr convert(ForDTMFSenderPtr object);
      static RTPSenderPtr convert(ForMediaStreamTrackPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSender
      #pragma mark

      static ElementPtr toDebug(RTPSenderPtr receiver);

      static RTPSenderPtr create(
                                 IRTPSenderDelegatePtr delegate,
                                 IMediaStreamTrackPtr track,
                                 IRTPTransportPtr transport,
                                 IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                 );

      virtual PUID getID() const override {return mID;}

      virtual IRTPSenderSubscriptionPtr subscribe(IRTPSenderDelegatePtr delegate) override;

      virtual IMediaStreamTrackPtr track() const override;
      virtual IRTPTransportPtr transport() const override;
      virtual IRTCPTransportPtr rtcpTransport() const override;

      virtual void setTransport(
                                IRTPTransportPtr transport,
                                IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                ) override;
      virtual PromisePtr setTrack(IMediaStreamTrackPtr track) override;

      static CapabilitiesPtr getCapabilities(Optional<Kinds> kind);

      virtual PromisePtr send(const Parameters &parameters) override;
      virtual void stop() override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSenderForRTPListener
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForRTPListenerPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTCPPacketPtr packet
                                ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSenderForRTPSenderChannel
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForRTPSenderChannelPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool sendPacket(RTPPacketPtr packet) override;
      virtual bool sendPacket(RTCPPacketPtr packet) override;

      virtual void notifyConflict(
                                  UseChannelPtr channel,
                                  IRTPTypes::SSRCType ssrc,
                                  bool selfDestruct
                                  ) override;

      virtual void notifyDTMFSenderToneChanged(const char *tone) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSenderForMediaStreamTrack
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForMediaStreamTrackPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSenderForDTMFSender
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForRTPSenderPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual IDTMFSenderSubscriptionPtr subscribeDTMF(IDTMFSenderDelegatePtr delegate) override;

      virtual bool canInsertDTMF() const override;

      virtual void insertDTMF(
                              const char *tones,
                              Milliseconds duration = Milliseconds(70),
                              Milliseconds interToneGap = Milliseconds(70)
                              ) throw (
                                        InvalidStateError,
                                        InvalidCharacterError
                                        ) override;

      virtual IRTPSenderPtr sender() const override;

      virtual String toneBuffer() const override;
      virtual Milliseconds duration() const override;
      virtual Milliseconds interToneGap() const override;

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
      #pragma mark RTPSender => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IDTMFSenderDelegate
      #pragma mark

      virtual void onDTMFSenderToneChanged(
                                           IDTMFSenderPtr sender,
                                           String tone
                                           ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSenderAsyncDelegate
      #pragma mark

      virtual void onDestroyChannel(UseChannelPtr channel) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiver => (friend ChannelHolder)
      #pragma mark

      void notifyChannelGone();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

      ChannelHolderPtr addChannel(ParametersPtr newParams);
      void updateChannel(
                         ChannelHolderPtr channel,
                         ParametersPtr newParams
                         );
      void removeChannel(ChannelHolderPtr channel);

      void notifyChannelsOfTransportState();

      ChannelHolderPtr getDTMFChannelHolder() const;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => (data)
      #pragma mark

      AutoPUID mID;
      RTPSenderWeakPtr mThisWeak;
      RTPSenderPtr mGracefulShutdownReference;

      IRTPSenderDelegateSubscriptions mSubscriptions;
      IRTPSenderSubscriptionPtr mDefaultSubscription;

      IDTMFSenderDelegateSubscriptions mDTMFSubscriptions;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      ParametersPtr mParameters;
      ParametersPtrList mParametersGroupedIntoChannels;

      UseListenerPtr mListener;

      UseSecureTransportPtr mRTPTransport;
      UseSecureTransportPtr mRTCPTransport;

      ISecureTransportSubscriptionPtr mRTCPTransportSubscription;

      IICETypes::Components mSendRTPOverTransport {IICETypes::Component_RTP};
      IICETypes::Components mSendRTCPOverTransport {IICETypes::Component_RTCP};
      IICETypes::Components mReceiveRTCPOverTransport {IICETypes::Component_RTCP};

      ISecureTransport::States mLastReportedTransportStateToChannels {ISecureTransport::State_Pending};

      Optional<IMediaStreamTrackTypes::Kinds> mKind;
      UseMediaStreamTrackPtr mTrack;

      ParametersToChannelHolderMapPtr mChannels;  // using COW pattern

      SSRCList mConflicts;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderFactory
    #pragma mark

    interaction IRTPSenderFactory
    {
      typedef IRTPSenderTypes::Kinds Kinds;
      typedef IRTPSenderTypes::CapabilitiesPtr CapabilitiesPtr;

      static IRTPSenderFactory &singleton();

      virtual RTPSenderPtr create(
                                  IRTPSenderDelegatePtr delegate,
                                  IMediaStreamTrackPtr track,
                                  IRTPTransportPtr transport,
                                  IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                  );

      virtual CapabilitiesPtr getCapabilities(Optional<Kinds> kind);
    };

    class RTPSenderFactory : public IFactory<IRTPSenderFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPSenderAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPSenderAsyncDelegate::UseChannelPtr, UseChannelPtr)
ZS_DECLARE_PROXY_METHOD_1(onDestroyChannel, UseChannelPtr)
ZS_DECLARE_PROXY_END()
