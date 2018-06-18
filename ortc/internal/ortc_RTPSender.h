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
#include <ortc/internal/ortc_IMediaStreamTrack.h>

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

    ZS_DECLARE_INTERACTION_PROXY(IRTPSenderAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderForSettings
    //

    interaction IRTPSenderForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForSettings, ForSettings)

      static void applyDefaults() noexcept;

      virtual ~IRTPSenderForSettings() noexcept {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderForRTPListener
    //

    interaction IRTPSenderForRTPListener
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPListener, ForRTPListener);

      static ElementPtr toDebug(ForRTPListenerPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

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
    // IRTPSenderForRTPSenderChannel
    //

    interaction IRTPSenderForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPSenderChannel, ForRTPSenderChannel);

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, UseChannel);

      static ElementPtr toDebug(ForRTPSenderChannelPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual bool sendPacket(RTPPacketPtr packet) noexcept = 0;
      virtual bool sendPacket(RTCPPacketPtr packet) noexcept = 0;

      virtual void notifyConflict(
                                  UseChannelPtr channel,
                                  IRTPTypes::SSRCType ssrc,
                                  bool selfDestruct
                                  ) noexcept = 0;

      virtual void notifyDTMFSenderToneChanged(const char *tone) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderForDTMFSender
    //

    interaction IRTPSenderForDTMFSender : public IDTMFSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForDTMFSender, ForDTMFSender);

      static ElementPtr toDebug(ForDTMFSenderPtr transport) noexcept;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderForMediaStreamTrack
    //

    interaction IRTPSenderForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForMediaStreamTrack, ForMediaStreamTrack);

      static ElementPtr toDebug(ForMediaStreamTrackPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderAsyncDelegate
    //

    interaction IRTPSenderAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, UseChannel);

      virtual void onDestroyChannel(UseChannelPtr channel) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPSender
    //
    
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
      //
      // RTPSender::ChannelHolder
      //

      struct ChannelHolder
      {
        RTPSenderWeakPtr mHolder;
        UseChannelPtr mChannel;
        std::atomic<ISecureTransport::States> mLastReportedState {ISecureTransport::State_Pending};

        ChannelHolder() noexcept;
        ~ChannelHolder() noexcept;

        PUID getID() const noexcept;
        void notify(ISecureTransport::States state) noexcept;

        void notify(RTCPPacketListPtr packets) noexcept;

        void update(const Parameters &params) noexcept;

        bool handle(RTCPPacketPtr packet) noexcept;

        void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) noexcept;

        void insertDTMF(
                        const char *tones,
                        Milliseconds duration,
                        Milliseconds interToneGap
                        ) noexcept;

        String toneBuffer() const noexcept;
        Milliseconds duration() const noexcept;
        Milliseconds interToneGap() const noexcept;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //
      // RTPSender::States
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
      RTPSender(
                const make_private &,
                IMessageQueuePtr queue,
                IRTPSenderDelegatePtr delegate,
                ortc::IMediaStreamTrackPtr track,
                IRTPTransportPtr transport,
                IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                ) noexcept(false); // throws InvalidParameters, InvalidStateError

    protected:
      RTPSender(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~RTPSender() noexcept;

      static RTPSenderPtr convert(IRTPSenderPtr object) noexcept;
      static RTPSenderPtr convert(ForSettingsPtr object) noexcept;
      static RTPSenderPtr convert(ForRTPListenerPtr object) noexcept;
      static RTPSenderPtr convert(ForRTPSenderChannelPtr object) noexcept;
      static RTPSenderPtr convert(ForDTMFSenderPtr object) noexcept;
      static RTPSenderPtr convert(ForMediaStreamTrackPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPSender => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSender => IRTPSender
      //

      static ElementPtr toDebug(RTPSenderPtr receiver) noexcept;

      static RTPSenderPtr create(
                                 IRTPSenderDelegatePtr delegate,
                                 ortc::IMediaStreamTrackPtr track,
                                 IRTPTransportPtr transport,
                                 IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                 ) noexcept;

      PUID getID() const noexcept override {return mID;}

      IRTPSenderSubscriptionPtr subscribe(IRTPSenderDelegatePtr delegate) noexcept override;

      ortc::IMediaStreamTrackPtr track() const noexcept override;
      IRTPTransportPtr transport() const noexcept override;
      IRTCPTransportPtr rtcpTransport() const noexcept override;

      virtual void setTransport(
                                IRTPTransportPtr transport,
                                IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                ) noexcept(false) override; // throws InvalidParameters
      PromisePtr setTrack(ortc::IMediaStreamTrackPtr track) noexcept override;

      static CapabilitiesPtr getCapabilities(Optional<Kinds> kind) noexcept;

      PromisePtr send(const Parameters &parameters) noexcept(false) override;   // throws InvalidParameters
      void stop() noexcept override;


      //-----------------------------------------------------------------------
      //
      // RTPSender => IRTPSenderForRTPListener
      //

      // (duplciate) static ElementPtr toDebug(ForRTPListenerPtr transport) noexcept;

      // (duplicate) virtual PUID getID() const noexcept = 0;

      virtual bool handlePacket(
                                IICETypes::Components viaTransport,
                                RTCPPacketPtr packet
                                ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSender => IRTPSenderForRTPSenderChannel
      //

      // (duplciate) static ElementPtr toDebug(ForRTPSenderChannelPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      bool sendPacket(RTPPacketPtr packet) noexcept override;
      bool sendPacket(RTCPPacketPtr packet) noexcept override;

      virtual void notifyConflict(
                                  UseChannelPtr channel,
                                  IRTPTypes::SSRCType ssrc,
                                  bool selfDestruct
                                  ) noexcept override;

      void notifyDTMFSenderToneChanged(const char *tone) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSender => IRTPSenderForMediaStreamTrack
      //

      // (duplciate) static ElementPtr toDebug(ForMediaStreamTrackPtr transport) noexcept;

      // (duplicate) virtual PUID getID() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // RTPSender => IRTPSenderForDTMFSender
      //

      // (duplciate) static ElementPtr toDebug(ForRTPSenderPtr transport) noexcept;

      // (duplicate) virtual PUID getID() const noexcept = 0;

      //IDTMFSenderSubscriptionPtr subscribe(IDTMFSenderDelegatePtr delegate) noexcept override { return subscribeDTMF(delegate); }

      //virtual IDTMFSenderSubscriptionPtr subscribeDTMF(IDTMFSenderDelegatePtr delegate) noexcept = 0;

      IDTMFSenderSubscriptionPtr subscribe(IDTMFSenderDelegatePtr delegate) noexcept override;

      bool canInsertDTMF() const noexcept override;

      virtual void insertDTMF(
                              const char *tones,
                              Milliseconds duration = Milliseconds(70),
                              Milliseconds interToneGap = Milliseconds(70)
                              ) noexcept(false) override; // throws InvalidStateError, InvalidCharacterError

      IRTPSenderPtr sender() const noexcept override;

      String toneBuffer() const noexcept override;
      Milliseconds duration() const noexcept override;
      Milliseconds interToneGap() const noexcept override;

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
      // RTPSender => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // RTPSender => IDTMFSenderDelegate
      //

      void onDTMFSenderToneChanged(
                                   IDTMFSenderPtr sender,
                                   String tone
                                   ) override;

      //-----------------------------------------------------------------------
      //
      // RTPSender => IRTPSenderAsyncDelegate
      //

      void onDestroyChannel(UseChannelPtr channel) override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiver => (friend ChannelHolder)
      //

      void notifyChannelGone() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPSender => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;

      void cancel() noexcept;

      void setState(States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

      ChannelHolderPtr addChannel(ParametersPtr newParams) noexcept;
      void updateChannel(
                         ChannelHolderPtr channel,
                         ParametersPtr newParams
                         ) noexcept;
      void removeChannel(ChannelHolderPtr channel) noexcept;

      void notifyChannelsOfTransportState() noexcept;

      ChannelHolderPtr getDTMFChannelHolder() const noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPSender => (data)
      //

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
    //
    // IRTPSenderFactory
    //

    interaction IRTPSenderFactory
    {
      typedef IRTPSenderTypes::Kinds Kinds;
      typedef IRTPSenderTypes::CapabilitiesPtr CapabilitiesPtr;

      static IRTPSenderFactory &singleton() noexcept;

      virtual RTPSenderPtr create(
                                  IRTPSenderDelegatePtr delegate,
                                  ortc::IMediaStreamTrackPtr track,
                                  IRTPTransportPtr transport,
                                  IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                  ) noexcept;

      virtual CapabilitiesPtr getCapabilities(Optional<Kinds> kind) noexcept;
    };

    class RTPSenderFactory : public IFactory<IRTPSenderFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPSenderAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPSenderAsyncDelegate::UseChannelPtr, UseChannelPtr)
ZS_DECLARE_PROXY_METHOD(onDestroyChannel, UseChannelPtr)
ZS_DECLARE_PROXY_END()
