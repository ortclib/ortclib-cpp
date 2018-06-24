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
#include <set>

#define ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT "ortc/sctp/max-sessions-per-port"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPInit);
    ZS_DECLARE_CLASS_PTR(SCTPTransport);

    ZS_DECLARE_STRUCT_PTR(SCTPPacketIncoming);
    ZS_DECLARE_STRUCT_PTR(SCTPPacketOutgoing);

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSettings);
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForDataChannel);
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSCTPTransportListener);
    ZS_DECLARE_INTERACTION_PTR(IDataChannelForSCTPTransport);
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportListenerForSCTPTransport);

    ZS_DECLARE_INTERACTION_PTR(IICETransportForDataTransport);

    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportAsyncDelegate);
    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportForDataChannelDelegate);
    
    ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISCTPTransportForDataChannelSubscription, ISCTPTransportForDataChannelDelegate);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // PayloadProtocolIdentifier
    //

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
    const char *toString(SCTPPayloadProtocolIdentifier state) noexcept;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPPacketIncoming
    //

    struct SCTPPacketIncoming
    {
      SCTPPayloadProtocolIdentifier mType {SCTP_PPID_NONE};

      WORD mSessionID {};
      WORD mSequenceNumber {};
      DWORD mTimestamp {};
      int mFlags {};
      SecureByteBlockPtr mBuffer;

      ElementPtr toDebug() const noexcept;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPPacketOutgoing
    //

    struct SCTPPacketOutgoing
    {
      SCTPPayloadProtocolIdentifier mType {SCTP_PPID_NONE};

      WORD                mSessionID {};
      bool                mOrdered {true};
      Milliseconds        mMaxPacketLifetime {};
      Optional<DWORD>     mMaxRetransmits;
      SecureByteBlockPtr  mBuffer;

      ElementPtr toDebug() const noexcept;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportForSettings
    //

    interaction ISCTPTransportForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSettings, ForSettings)

      static void applyDefaults() noexcept;

      virtual ~ISCTPTransportForSettings() noexcept {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportForDataChannel
    //

    interaction ISCTPTransportForDataChannel
    {
      ZS_DECLARE_STRUCT_PTR(RejectReason);

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, ForDataChannel);

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel);

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters);

      static ElementPtr toDebug(ForDataChannelPtr transport) noexcept;

      struct RejectReason : public Any
      {
        RejectReason(WORD error, const char *reason) noexcept : mError(error), mErrorReason(reason) {}
        static RejectReasonPtr create(WORD error, const char *reason) noexcept {return make_shared<RejectReason>(error, reason);}
        WORD mError {};
        String mErrorReason;
      };

      virtual PUID getRealID() const noexcept = 0;

      virtual void registerNewDataChannel(
                                          UseDataChannelPtr &ioDataChannel,
                                          WORD &ioSessionID
                                          ) noexcept(false) = 0;

      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) noexcept = 0;

      virtual bool isShuttingDown() const noexcept = 0;
      virtual bool isShutdown() const noexcept = 0;
      virtual bool isReady() const noexcept = 0;

      virtual void announceIncoming(
                                    UseDataChannelPtr dataChannel,
                                    ParametersPtr params
                                    ) noexcept = 0;

      virtual PromisePtr sendDataNow(SCTPPacketOutgoingPtr packet) noexcept = 0;

      virtual void requestShutdown(
                                   UseDataChannelPtr dataChannel,
                                   WORD sessionID
                                   ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportForSCTPTransportListener
    //

    interaction ISCTPTransportForSCTPTransportListener
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, ForListener);

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, UseListener);
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport);

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportTypes::Capabilities, Capabilities);

      static ForListenerPtr create(
                                   UseListenerPtr listener,
                                   UseSecureTransportPtr secureTransport,
                                   WORD localPort
                                   ) noexcept;

      virtual PUID getRealID() const noexcept = 0;

      virtual void startFromListener(
                                     const Capabilities &remoteCapabilities,
                                     WORD remotePort
                                     ) noexcept(false) = 0; // throws InvalidStateError, InvalidParameters

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) noexcept = 0;

      virtual void notifyShutdown() noexcept = 0;

      virtual bool isShuttingDown() const noexcept = 0;
      virtual bool isShutdown() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportAsyncDelegate
    //

    interaction ISCTPTransportAsyncDelegate
    {
      virtual void onIncomingPacket(SCTPPacketIncomingPtr packet) = 0;
      virtual void onNotifiedToShutdown() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportForDataChannelDelegate
    //

    interaction ISCTPTransportForDataChannelDelegate
    {
      virtual void onSCTPTransportStateChanged() = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //
    // ISCTPTransportForDataChannelSubscription
    //
    
    interaction ISCTPTransportForDataChannelSubscription
    {
      virtual PUID getID() const noexcept = 0;
      
      virtual void cancel() noexcept = 0;
      
      virtual void background() noexcept = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::SCTPPacketIncomingPtr, SCTPPacketIncomingPtr)
ZS_DECLARE_PROXY_METHOD(onIncomingPacket, SCTPPacketIncomingPtr)
ZS_DECLARE_PROXY_METHOD(onNotifiedToShutdown)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportForDataChannelDelegate)
ZS_DECLARE_PROXY_METHOD(onSCTPTransportStateChanged)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::ISCTPTransportForDataChannelDelegate, ortc::internal::ISCTPTransportForDataChannelSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onSCTPTransportStateChanged)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport
    //
    
    class SCTPTransport : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public ISCTPTransport,
                          public ISCTPTransportForSettings,
                          public ISCTPTransportForDataChannel,
                          public ISCTPTransportForSCTPTransportListener,
                          public ISCTPTransportDelegate,
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
      typedef std::set<DataChannelID> DataChannelIDSet;

      typedef WORD SessionID;
      typedef std::map<SessionID, UseDataChannelPtr> DataChannelSessionMap;

      typedef std::queue<PromisePtr> PromiseQueue;

      typedef std::queue<SecureByteBlockPtr> BufferQueue;
      typedef std::pair<PromiseWithSocketOptionsPtr, SocketOptionsPtr> PromiseWithSocketOptionsPair;
      typedef std::list<PromiseWithSocketOptionsPair> PromiseWithSocketOptionsList;
      typedef std::pair<PromisePtr, SocketOptionsPtr> PromiseSocketOptionsPair;
      typedef std::list<PromiseSocketOptionsPair> PromiseSocketOptionsList;

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
      static const char *toString(InternalStates state) noexcept;
      ISCTPTransportTypes::States toState(InternalStates state) noexcept;

    public:
      SCTPTransport(
                    const make_private &,
                    IMessageQueuePtr queue,
                    ISCTPTransportDelegatePtr originalDelegate,
                    UseListenerPtr listener,
                    UseSecureTransportPtr secureTransport,
                    WORD localPort = 0,
                    WORD remotePort = 0
                    ) noexcept;

      SCTPTransport(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init() noexcept;

    public:
      virtual ~SCTPTransport() noexcept;

      static SCTPTransportPtr convert(ISCTPTransportPtr object) noexcept;
      static SCTPTransportPtr convert(IDataTransportPtr object) noexcept;
      static SCTPTransportPtr convert(ForSettingsPtr object) noexcept;
      static SCTPTransportPtr convert(ForDataChannelPtr object) noexcept;
      static SCTPTransportPtr convert(ForListenerPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransport
      //

      static ElementPtr toDebug(SCTPTransportPtr transport) noexcept;

      static ISCTPTransportPtr create(
                                      ISCTPTransportDelegatePtr delegate,
                                      IDTLSTransportPtr transport,
                                      WORD localPort = 0
                                      ) noexcept(false); // throws InvalidParameters, InvalidStateError

      PUID getID() const noexcept override;

      IDTLSTransportPtr transport() const noexcept override;
      ISCTPTransportTypes::States state() const noexcept override;

      WORD port() const noexcept override;

      WORD localPort() const noexcept override;
      Optional<WORD> remotePort() const noexcept override;

      PromiseWithSocketOptionsPtr getOptions(const SocketOptions &inWhichOptions) noexcept override;
      PromisePtr setOptions(const SocketOptions &inOptions) noexcept override;

      void start(
                 const Capabilities &remoteCapabilities,
                 WORD remotePort
                 ) noexcept(false) override; // throws InvalidStateError, InvalidParameters
      void stop() noexcept override;

      ISCTPTransportSubscriptionPtr subscribe(ISCTPTransportDelegatePtr delegate) noexcept override;


      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransportForDataChannel
      //

      // (duplciate) static ElementPtr toDebug(ForDataChannelPtr transport);

      void registerNewDataChannel(
                                  UseDataChannelPtr &ioDataChannel,
                                  WORD &ioSessionID
                                  ) noexcept(false) override; // throws InvalidParameters

      PUID getRealID() const noexcept override { return mID; }
      ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) noexcept override;

      // (duplicate) virtual bool isShuttingDown() const noexcept override;
      // (duplicate) virtual bool isShutdown() const noexcept override;
      bool isReady() const noexcept override;

      void announceIncoming(
                            UseDataChannelPtr dataChannel,
                            ParametersPtr params
                            ) noexcept override;

      PromisePtr sendDataNow(SCTPPacketOutgoingPtr packet) noexcept override;

      void requestShutdown(
                           UseDataChannelPtr dataChannel,
                           WORD sessionID
                           ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransportForSCTPTransportListener
      //

      // (duplicate) virtual PUID getID() const = 0;

      static ForListenerPtr create(
                                   UseListenerPtr listener,
                                   UseSecureTransportPtr secureTransport,
                                   WORD localPort
                                   ) noexcept;

      void startFromListener(
                             const Capabilities &remoteCapabilities,
                             WORD remotePort
                             ) noexcept(false) override; // throws InvalidStateError, InvalidParameters

      bool handleDataPacket(
                            const BYTE *buffer,
                            size_t bufferLengthInBytes
                            ) noexcept override;

      void notifyShutdown() noexcept override;

      // (duplicate) virtual bool isShuttingDown() const noexcept = 0;
      // (duplicate) virtual bool isShutdown() const noexcept = 0;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => friend SCTPInit
      //

      bool notifySendSCTPPacket(
                                const BYTE *buffer,
                                size_t bufferLengthInBytes
                                ) noexcept;
      
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransportDelegate
      //

      void onSCTPTransportStateChange(
                                      ISCTPTransportPtr transport,
                                      ISCTPTransportTypes::States state
                                      ) override;
      void onSCTPTransportDataChannel(
                                      ISCTPTransportPtr transport,
                                      IDataChannelPtr channel
                                      ) override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransportAsyncDelegate
      //

      void onIncomingPacket(SCTPPacketIncomingPtr packet) override;
      void onNotifiedToShutdown() override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => (friend SCTPInit)
      //

      virtual IMessageQueuePtr getDeliveryQueue() const noexcept { return mDeliveryQueue; }

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISecureTransportDelegate
      //

      void onSecureTransportStateChanged(
                                         ISecureTransportPtr transport,
                                         ISecureTransportTypes::States state
                                         ) override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept override;
      bool isShutdown() const noexcept override;

      void step() noexcept;
      bool stepStartCalled() noexcept;
      bool stepSecureTransport() noexcept;
      bool stepOpen() noexcept;
      bool stepDeliverIncomingPackets() noexcept;
      bool stepConnected() noexcept;
      bool stepResetStream() noexcept;

      void cancel() noexcept;

      void setState(InternalStates state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

      bool openConnectSCTPSocket() noexcept;
      bool openSCTPSocket() noexcept;
      bool prepareSocket(struct socket *sock) noexcept;

      bool isSessionAvailable(WORD sessionID) noexcept;
      bool attemptSend(
                       const SCTPPacketOutgoing &inPacket,
                       bool &outWouldBlock
                       ) noexcept;
      void notifyWriteReady() noexcept;

      void handleNotificationPacket(const sctp_notification &notification) noexcept;
      void handleNotificationAssocChange(const sctp_assoc_change &change) noexcept;
      void handleStreamResetEvent(const sctp_stream_reset_event &event) noexcept;

      virtual bool internalGetOptions(SocketOptions &ioOptions) const noexcept;
      virtual bool internalSetOptions(const SocketOptions &inOptions) noexcept;

    public:

      //-----------------------------------------------------------------------
      //
      // SCTPTransport::TearAwayData
      //

      struct TearAwayData
      {
        UseListenerPtr mListener;
      };

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => (data)
      //

      AutoPUID mID;
      mutable std::atomic<bool> mObtainedID {};
      SCTPTransportWeakPtr mThisWeak;
      SCTPTransportPtr mGracefulShutdownReference;

      SCTPInitPtr mSCTPInit;
      size_t mMaxSessionsPerPort {};

      ISCTPTransportDelegateSubscriptions mSubscriptions;
      ISCTPTransportSubscriptionPtr mDefaultSubscription;

      ISCTPTransportForDataChannelDelegateSubscriptions mDataChannelSubscriptions;

      std::atomic<InternalStates> mCurrentState {InternalState_First};
      ISCTPTransportTypes::States mLastReportedState {ISCTPTransportTypes::State_First};

      WORD mLastError {};
      String mLastErrorReason;

      UseListenerWeakPtr mListener;

      // If the transport is started after an existing transport was created
      // by the remote party then two SCTP transports will exist pointing
      // to the same local:remote mapping. This redirection forwards all
      // requests to the existing object to prevent mapping confusion.
      SCTPTransportPtr mRedirectToExistingTransport;
      ISCTPTransportSubscriptionPtr mRedirectToExistingTransportSubscription;

      UseSecureTransportWeakPtr mSecureTransport; // no lock needed
      ISecureTransportSubscriptionPtr mSecureTransportSubscription;

      CapabilitiesPtr mCapabilities;
      bool mStartedFromListener {};

      SCTPTransportWeakPtr *mThisSocket {};
      ISCTPTransportWeakPtr mTearAway;
      IMessageQueuePtr mDeliveryQueue;

      bool mIncoming {false};

      mutable struct socket *mSocket {};

      WORD mLocalPort {};
      Optional<WORD> mAllocatedLocalPort {};
      Optional<WORD> mRemotePort {};

      DataChannelMap mAnnouncedIncomingDataChannels;

      DataChannelSessionMap mSessions;

      bool mAttemptResetLater {false};
      DataChannelSessionMap mPendingResetSessions;
      DataChannelSessionMap mQueuedResetSessions;

      DataChannelIDSet mFilterSessionRequests;

      bool mSettledRole {false};
      WORD mCurrentAllocationSessionID {};
      WORD mMinAllocationSessionID {0};
      WORD mMaxAllocationSessionID {65534};
      WORD mNextAllocationIncrement {2};

      PromiseWithSocketOptionsList mGetSocketOptions;
      PromiseSocketOptionsList mSetSocketOptions;

      PromiseQueue mWaitingToSend;

      bool mConnected {false};
      bool mWriteReady {false};

      BufferQueue mPendingIncomingBuffers;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportFactory
    //

    interaction ISCTPTransportFactory
    {
      typedef ISCTPTransportTypes::CapabilitiesPtr CapabilitiesPtr;

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, ForListener);
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, UseListener);
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport);

      static ISCTPTransportFactory &singleton() noexcept;

      virtual ForListenerPtr create(
                                    UseListenerPtr listener,
                                    UseSecureTransportPtr secureTransport,
                                    WORD localPort
                                    ) noexcept;

      virtual ISCTPTransportPtr create(
                                       ISCTPTransportDelegatePtr delegate,
                                       IDTLSTransportPtr transport,
                                       WORD localPort = 0
                                       ) noexcept;
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
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(getStats, PromiseWithStatsReportPtr, const StatsTypeSet &)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(transport, IDTLSTransportPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(state, States)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(port, WORD)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(localPort, WORD)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(remotePort, Optional<WORD>)
  PromiseWithSocketOptionsPtr getOptions(const SocketOptions &inWhichOptions) noexcept override
  {
    return getDelegate()->getOptions(inWhichOptions);
  }
  PromisePtr setOptions(const SocketOptions &inOptions) noexcept override
  {
    return getDelegate()->setOptions(inOptions);
  }
  void start(const Capabilities & v1, WORD v2) noexcept(false) override { // throws ortc::InvalidStateError, ortc::InvalidParameters
    getDelegate()->start(v1, v2);
  }
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC(stop)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN(subscribe, ISCTPTransportSubscriptionPtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_END()
