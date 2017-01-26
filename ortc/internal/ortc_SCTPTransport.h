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

#include <ortc/IDataChannel.h>
#include <ortc/ISCTPTransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>
#include <zsLib/ProxySubscriptions.h>
#include <zsLib/TearAway.h>

#include <usrsctp.h>
#include <queue>

#define ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT "ortc/sctp/max-sessions-per-port"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPInit)
    ZS_DECLARE_CLASS_PTR(SCTPTransport)

    ZS_DECLARE_STRUCT_PTR(SCTPPacketIncoming)
    ZS_DECLARE_STRUCT_PTR(SCTPPacketOutgoing)

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSettings)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForDataChannel)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSCTPTransportListener)
    ZS_DECLARE_INTERACTION_PTR(IDataChannelForSCTPTransport)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportListenerForSCTPTransport)

    ZS_DECLARE_INTERACTION_PTR(IICETransportForDataTransport)

    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportAsyncDelegate)
    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportForDataChannelDelegate)
    
    ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISCTPTransportForDataChannelSubscription, ISCTPTransportForDataChannelDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark PayloadProtocolIdentifier
    #pragma mark

    enum SCTPPayloadProtocolIdentifier
    {
      SCTP_PPID_NONE = 0,  // No protocol is specified.
      // Matches the PPIDs in mozilla source and
      // https://datatracker.ietf.org/doc/draft-ietf-rtcweb-data-protocol Sec. 8.1
      // http://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13 Sec 8
      // They're not yet assigned by IANA.
      SCTP_PPID_CONTROL = 50,
      SCTP_PPID_BINARY_EMPTY = 57,
      SCTP_PPID_BINARY_PARTIAL = 52,
      SCTP_PPID_BINARY_LAST = 53,
      SCTP_PPID_STRING_EMPTY = 56,
      SCTP_PPID_STRING_PARTIAL = 54,
      SCTP_PPID_STRING_LAST = 51,
    };
    const char *toString(SCTPPayloadProtocolIdentifier state);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPPacketIncoming
    #pragma mark

    struct SCTPPacketIncoming
    {
      SCTPPayloadProtocolIdentifier mType {SCTP_PPID_NONE};

      WORD mSessionID {};
      WORD mSequenceNumber {};
      DWORD mTimestamp {};
      int mFlags {};
      SecureByteBlockPtr mBuffer;

      ElementPtr toDebug() const;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPPacketOutgoing
    #pragma mark

    struct SCTPPacketOutgoing
    {
      SCTPPayloadProtocolIdentifier mType {SCTP_PPID_NONE};

      WORD                mSessionID {};
      bool                mOrdered {true};
      Milliseconds        mMaxPacketLifetime {};
      Optional<DWORD>     mMaxRetransmits;
      SecureByteBlockPtr  mBuffer;

      ElementPtr toDebug() const;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForSettings
    #pragma mark

    interaction ISCTPTransportForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSettings, ForSettings)

      static void applyDefaults();

      virtual ~ISCTPTransportForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannel
    #pragma mark

    interaction ISCTPTransportForDataChannel
    {
      ZS_DECLARE_STRUCT_PTR(RejectReason)

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, ForDataChannel)

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel)

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForDataChannelPtr transport);

      struct RejectReason : public Any
      {
        RejectReason(WORD error, const char *reason) : mError(error), mErrorReason(reason) {}
        static RejectReasonPtr create(WORD error, const char *reason) {return make_shared<RejectReason>(error, reason);}
        WORD mError {};
        String mErrorReason;
      };

      virtual PUID getID() const = 0;

      virtual void registerNewDataChannel(
                                          UseDataChannelPtr &ioDataChannel,
                                          WORD &ioSessionID
                                          ) = 0;

      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) = 0;

      virtual bool isShuttingDown() const = 0;
      virtual bool isShutdown() const = 0;
      virtual bool isReady() const = 0;

      virtual void announceIncoming(
                                    UseDataChannelPtr dataChannel,
                                    ParametersPtr params
                                    ) = 0;

      virtual PromisePtr sendDataNow(SCTPPacketOutgoingPtr packet) = 0;

      virtual void requestShutdown(
                                   UseDataChannelPtr dataChannel,
                                   WORD sessionID
                                   ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForSCTPTransportListener
    #pragma mark

    interaction ISCTPTransportForSCTPTransportListener
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, ForListener)

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, UseListener)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportTypes::Capabilities, Capabilities);

      static ForListenerPtr create(
                                   UseListenerPtr listener,
                                   UseSecureTransportPtr secureTransport,
                                   WORD localPort
                                   );

      virtual PUID getID() const = 0;

      virtual void start(
                         const Capabilities &remoteCapabilities,
                         WORD remotePort
                         ) throw (InvalidStateError, InvalidParameters) = 0;

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) = 0;

      virtual void notifyShutdown() = 0;

      virtual bool isShuttingDown() const = 0;
      virtual bool isShutdown() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportAsyncDelegate
    #pragma mark

    interaction ISCTPTransportAsyncDelegate
    {
      virtual void onIncomingPacket(SCTPPacketIncomingPtr packet) = 0;
      virtual void onNotifiedToShutdown() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannelDelegate
    #pragma mark

    interaction ISCTPTransportForDataChannelDelegate
    {
      virtual void onSCTPTransportStateChanged() = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannelSubscription
    #pragma mark
    
    interaction ISCTPTransportForDataChannelSubscription
    {
      virtual PUID getID() const = 0;
      
      virtual void cancel() = 0;
      
      virtual void background() = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::SCTPPacketIncomingPtr, SCTPPacketIncomingPtr)
ZS_DECLARE_PROXY_METHOD_1(onIncomingPacket, SCTPPacketIncomingPtr)
ZS_DECLARE_PROXY_METHOD_0(onNotifiedToShutdown)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportForDataChannelDelegate)
ZS_DECLARE_PROXY_METHOD_0(onSCTPTransportStateChanged)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::ISCTPTransportForDataChannelDelegate, ortc::internal::ISCTPTransportForDataChannelSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onSCTPTransportStateChanged)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport
    #pragma mark
    
    class SCTPTransport : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public ISCTPTransport,
                          public ISCTPTransportForSettings,
                          public ISCTPTransportForDataChannel,
                          public ISCTPTransportForSCTPTransportListener,
                          public IWakeDelegate,
                          public zsLib::ITimerDelegate,
                          public ISCTPTransportAsyncDelegate,
                          public ISecureTransportDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISCTPTransport;
      friend interaction ISCTPTransportFactory;
      friend interaction ISCTPTransportForSettings;
      friend interaction ISCTPTransportForDataChannel;
      friend interaction ISCTPTransportForSCTPTransportListener;
      friend class SCTPInit;

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel);
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, UseListener);
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDataTransport, UseICETransport);

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportTypes::Capabilities, Capabilities);

      ZS_DECLARE_STRUCT_PTR(TearAwayData);

      typedef PUID DataChannelID;
      typedef std::map<DataChannelID, UseDataChannelPtr> DataChannelMap;

      typedef WORD SessionID;
      typedef std::map<SessionID, UseDataChannelPtr> DataChannelSessionMap;

      typedef std::queue<PromisePtr> PromiseQueue;

      typedef std::queue<SecureByteBlockPtr> BufferQueue;

      enum InternalStates
      {
        InternalState_First,

        InternalState_New                   = InternalState_First,
        InternalState_Connecting,
        InternalState_ConnectingDisrupted,
        InternalState_Ready,
        InternalState_Disconnected,
        InternalState_ShuttingDown,
        InternalState_Shutdown,

        InternalState_Last                  = InternalState_Shutdown,
      };
      static const char *toString(InternalStates state);
      ISCTPTransportTypes::States toState(InternalStates state);

    public:
      SCTPTransport(
                    const make_private &,
                    IMessageQueuePtr queue,
                    UseListenerPtr listener,
                    UseSecureTransportPtr secureTransport,
                    WORD localPort = 0,
                    WORD remotePort = 0
                    );

      SCTPTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init();

    public:
      virtual ~SCTPTransport();

      static SCTPTransportPtr convert(ISCTPTransportPtr object);
      static SCTPTransportPtr convert(IDataTransportPtr object);
      static SCTPTransportPtr convert(ForSettingsPtr object);
      static SCTPTransportPtr convert(ForDataChannelPtr object);
      static SCTPTransportPtr convert(ForListenerPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransport
      #pragma mark

      static ElementPtr toDebug(SCTPTransportPtr transport);

      static ISCTPTransportPtr create(
                                      ISCTPTransportDelegatePtr delegate,
                                      IDTLSTransportPtr transport,
                                      WORD localPort = 0
                                      ) throw (InvalidParameters, InvalidStateError);

      virtual PUID getID() const override {return mID;}

      virtual IDTLSTransportPtr transport() const override;
      virtual ISCTPTransportTypes::States state() const override;

      virtual WORD port() const override;

      virtual WORD localPort() const override;
      virtual Optional<WORD> remotePort() const override;

      virtual void start(
                         const Capabilities &remoteCapabilities,
                         WORD remotePort
                         ) throw (InvalidStateError, InvalidParameters) override;
      virtual void stop() override;

      virtual ISCTPTransportSubscriptionPtr subscribe(ISCTPTransportDelegatePtr delegate) override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportForDataChannel
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForDataChannelPtr transport);

      virtual void registerNewDataChannel(
                                          UseDataChannelPtr &ioDataChannel,
                                          WORD &ioSessionID
                                          ) override;

      // (duplicate) virtual PUID getID() const = 0;
      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) override;

      // (duplicate) virtual bool isShuttingDown() const override;
      // (duplicate) virtual bool isShutdown() const override;
      virtual bool isReady() const override;

      virtual void announceIncoming(
                                    UseDataChannelPtr dataChannel,
                                    ParametersPtr params
                                    ) override;

      virtual PromisePtr sendDataNow(SCTPPacketOutgoingPtr packet) override;

      virtual void requestShutdown(
                                   UseDataChannelPtr dataChannel,
                                   WORD sessionID
                                   ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportForSCTPTransportListener
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      static ForListenerPtr create(
                                   UseListenerPtr listener,
                                   UseSecureTransportPtr secureTransport,
                                   WORD localPort
                                   );

      // (duplicate) virtual void start(
      //                                const Capabilities &remoteCapabilities,
      //                                WORD remotePort
      //                                ) throw (InvalidStateError, InvalidParameters) = 0;

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) override;

      virtual void notifyShutdown() override;

      // (duplicate) virtual bool isShuttingDown() const = 0;
      // (duplicate) virtual bool isShutdown() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => friend SCTPInit
      #pragma mark

      virtual bool notifySendSCTPPacket(
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportAsyncDelegate
      #pragma mark

      virtual void onIncomingPacket(SCTPPacketIncomingPtr packet) override;
      virtual void onNotifiedToShutdown() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISecureTransportDelegate
      #pragma mark

      virtual void onSecureTransportStateChanged(
                                                 ISecureTransportPtr transport,
                                                 ISecureTransportTypes::States state
                                                 ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const override;
      bool isShutdown() const override;

      void step();
      bool stepStartCalled();
      bool stepSecureTransport();
      bool stepOpen();
      bool stepDeliverIncomingPackets();
      bool stepConnected();
      bool stepResetStream();

      void cancel();

      void setState(InternalStates state);
      void setError(WORD error, const char *reason = NULL);

      bool openConnectSCTPSocket();
      bool openSCTPSocket();
      bool prepareSocket(struct socket *sock);

      bool isSessionAvailable(WORD sessionID);
      bool attemptSend(
                       const SCTPPacketOutgoing &inPacket,
                       bool &outWouldBlock
                       );
      void notifyWriteReady();

      void handleNotificationPacket(const sctp_notification &notification);
      void handleNotificationAssocChange(const sctp_assoc_change &change);
      void handleStreamResetEvent(const sctp_stream_reset_event &event);

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport::TearAwayData
      #pragma mark

      struct TearAwayData
      {
        UseListenerPtr mListener;
        ISCTPTransportDelegateWeakPtr mDelegate;
        ISCTPTransportSubscriptionPtr mDefaultSubscription;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (data)
      #pragma mark

      AutoPUID mID;
      SCTPTransportWeakPtr mThisWeak;
      SCTPTransportPtr mGracefulShutdownReference;

      SCTPInitPtr mSCTPInit;
      size_t mMaxSessionsPerPort {};

      ISCTPTransportDelegateSubscriptions mSubscriptions;

      ISCTPTransportForDataChannelDelegateSubscriptions mDataChannelSubscriptions;

      std::atomic<InternalStates> mCurrentState {InternalState_First};
      ISCTPTransportTypes::States mLastReportedState {ISCTPTransportTypes::State_First};

      WORD mLastError {};
      String mLastErrorReason;

      UseListenerWeakPtr mListener;

      UseSecureTransportWeakPtr mSecureTransport; // no lock needed
      ISecureTransportSubscriptionPtr mSecureTransportSubscription;

      CapabilitiesPtr mCapabilities;

      SCTPTransportWeakPtr *mThisSocket {};
      ISCTPTransportWeakPtr mTearAway;

      bool mIncoming {false};

      struct socket *mSocket {};

      WORD mLocalPort {};
      Optional<WORD> mAllocatedLocalPort {};
      Optional<WORD> mRemotePort {};

      DataChannelMap mAnnouncedIncomingDataChannels;

      DataChannelSessionMap mSessions;

      bool mAttemptResetLater {false};
      DataChannelSessionMap mPendingResetSessions;
      DataChannelSessionMap mQueuedResetSessions;

      bool mSettledRole {false};
      WORD mCurrentAllocationSessionID {};
      WORD mMinAllocationSessionID {0};
      WORD mMaxAllocationSessionID {65534};
      WORD mNextAllocationIncrement {2};

      PromiseQueue mWaitingToSend;

      bool mConnected {false};
      bool mWriteReady {false};

      BufferQueue mPendingIncomingBuffers;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportFactory
    #pragma mark

    interaction ISCTPTransportFactory
    {
      typedef ISCTPTransportTypes::CapabilitiesPtr CapabilitiesPtr;

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, ForListener)
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, UseListener)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)

      static ISCTPTransportFactory &singleton();

      virtual ForListenerPtr create(
                                    UseListenerPtr listener,
                                    UseSecureTransportPtr secureTransport,
                                    WORD localPort
                                    );

      virtual ISCTPTransportPtr create(
                                       ISCTPTransportDelegatePtr delegate,
                                       IDTLSTransportPtr transport,
                                       WORD localPort = 0
                                       );
    };

    class SCTPTransportFactory : public IFactory<ISCTPTransportFactory> {};
  }
}

ZS_DECLARE_TEAR_AWAY_BEGIN(ortc::ISCTPTransport, ortc::internal::SCTPTransport::TearAwayData)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::InvalidStateError, InvalidStateError)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportSubscriptionPtr, ISCTPTransportSubscriptionPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportDelegatePtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportTypes::Capabilities, Capabilities)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IDTLSTransportPtr, IDTLSTransportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportTypes::States, States)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_1(getStats, PromiseWithStatsReportPtr, const StatsTypeSet &)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(transport, IDTLSTransportPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(state, States)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(port, WORD)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(localPort, WORD)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(remotePort, Optional<WORD>)
  virtual void start(const Capabilities & v1, WORD v2) throw (ortc::InvalidStateError, ortc::InvalidParameters) override {
    getDelegate()->start(v1, v2);
  }
ZS_DECLARE_TEAR_AWAY_METHOD_0(stop)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_1(subscribe, ISCTPTransportSubscriptionPtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_END()
