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

#include <ortc/ISCTPTransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>
#include <zsLib/ProxySubscriptions.h>
#include <zsLib/TearAway.h>

#include <usrsctp.h>

#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"
#define ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT "ortc/sctp/max-sessions-per-port"
#define ORTC_SETTING_SCTP_TRANSPORT_MAX_PORTS "ortc/sctp/max-ports"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPInit)
    ZS_DECLARE_CLASS_PTR(SCTPTransport)
    ZS_DECLARE_STRUCT_PTR(SCTPPacket)

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSettings)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForDataChannel)
    ZS_DECLARE_INTERACTION_PTR(IDataChannelForSCTPTransport)
    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForDataTransport)

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
      // They're not yet assigned by IANA.
      SCTP_PPID_CONTROL = 50,
      SCTP_PPID_BINARY_PARTIAL = 52,
      SCTP_PPID_BINARY_LAST = 53,
      SCTP_PPID_TEXT_PARTIAL = 54,
      SCTP_PPID_TEXT_LAST = 51
    };
    static const char *toString(SCTPPayloadProtocolIdentifier state);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPPacket
    #pragma mark

    struct SCTPPacket
    {
      SCTPPayloadProtocolIdentifier mType {SCTP_PPID_NONE};

      DWORD mTupleID {};
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
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, ForDataChannel)

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel)

      static ElementPtr toDebug(ForDataChannelPtr transport);

      virtual PUID getID() const = 0;

      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) = 0;

      virtual bool isShuttingDown() const = 0;
      virtual bool isShutdown() const = 0;
      virtual bool isReady() const = 0;

      virtual WORD allocateLocalPort(UseDataChannelPtr channel) = 0;
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
      virtual void onIncomingPacket(SCTPPacketPtr packet) = 0;
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
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::SCTPPacketPtr, SCTPPacketPtr)
ZS_DECLARE_PROXY_METHOD_1(onIncomingPacket, SCTPPacketPtr)
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
                          public IDataTransportForSecureTransport,
                          public ISCTPTransportForDataChannel,
                          public IWakeDelegate,
                          public zsLib::ITimerDelegate,
                          public ISCTPTransportAsyncDelegate,
                          public IICETransportDelegate,
                          public IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISCTPTransport;
      friend interaction ISCTPTransportFactory;
      friend interaction ISCTPTransportForSettings;
      friend interaction ISCTPTransportForSecureTransport;
      friend interaction ISCTPTransportForDataChannel;
      friend interaction IDataTransportForSecureTransport;
      friend class SCTPInit;

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDataTransport, UseICETransport)

      ZS_DECLARE_STRUCT_PTR(TearAwayData)
      ZS_DECLARE_STRUCT_PTR(SocketInfo)

      typedef DWORD LocalRemoteTupleID;
      typedef std::map<LocalRemoteTupleID, SocketInfoPtr> SocketInfoMap;

      typedef PUID DataChannelID;
      typedef std::map<DataChannelID, UseDataChannelPtr> DataChannelMap;

      typedef WORD SessionID;
      typedef std::map<SessionID, UseDataChannelPtr> DataChannelSessionMap;

      typedef std::map<WORD, size_t> AllocatedPortMap;

      typedef std::queue<SecureByteBlockPtr> BufferedQueue;

      enum States
      {
        State_Pending,
        State_Ready,
        State_Disconnected,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      SCTPTransport(
                    const make_private &,
                    IMessageQueuePtr queue,
                    UseSecureTransportPtr secureTransport
                    );

    protected:
      SCTPTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~SCTPTransport();

      static SCTPTransportPtr convert(ISCTPTransportPtr object);
      static SCTPTransportPtr convert(IDataTransportPtr object);
      static SCTPTransportPtr convert(ForSettingsPtr object);
      static SCTPTransportPtr convert(ForSecureTransportPtr object);
      static SCTPTransportPtr convert(ForDataChannelPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransport
      #pragma mark

      static ElementPtr toDebug(SCTPTransportPtr transport);

      static ISCTPTransportPtr create(
                                      ISCTPTransportDelegatePtr delegate,
                                      IDTLSTransportPtr transport
                                      );

      virtual PUID getID() const override {return mID;}

      static CapabilitiesPtr getCapabilities();

      virtual void start(const Capabilities &remoteCapabilities) override;
      virtual void stop() override;

      virtual ISCTPTransportSubscriptionPtr subscribe(ISCTPTransportDelegatePtr delegate) override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IDataTransportForSecureTransport
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForDTLSTransportPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      static ForSecureTransportPtr create(UseSecureTransportPtr transport);

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportForDataChannel
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForDataChannelPtr transport);

      // (duplicate) virtual PUID getID() const = 0;
      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) override;

      // (duplicate) virtual bool isShuttingDown() const override;
      // (duplicate) virtual bool isShutdown() const override;
      virtual bool isReady() const override;

      virtual WORD allocateLocalPort(UseDataChannelPtr channel);

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

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportAsyncDelegate
      #pragma mark

      virtual void onIncomingPacket(SCTPPacketPtr packet);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IICETransportDelegate
      #pragma mark

      virtual void onICETransportStateChanged(
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


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepSecureTransport();
      bool stepICETransport();
      bool stepDeliverIncomingPackets();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

      void allocatePort(
                        AllocatedPortMap &useMap,
                        WORD port
                        );
      void deallocatePort(
                          AllocatedPortMap &useMap,
                          WORD port
                          );

      SocketInfoPtr openIncomingSocket(
                                       WORD localPort,
                                       WORD remotePort,
                                       LocalRemoteTupleID tupleID
                                       );

      bool openListenSCTPSocket(SocketInfo &socketInfo);
      bool openSCTPSocket(SocketInfo &socketInfo);
      bool prepareSocket(struct socket *sock);

      void attemptAccept(SocketInfo &info);

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport::TearAwayData
      #pragma mark

      struct TearAwayData
      {
        UseSecureTransportPtr mSecureTransport;
        ISCTPTransportSubscriptionPtr mDefaultSubscription;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport::SocketInfo
      #pragma mark

      struct SocketInfo
      {
        SocketInfoPtr *mThis {};
        SCTPTransportWeakPtr mTransport;

        bool mIncoming {false};

        struct socket *mSocket {};
        struct socket *mAcceptSocket {};

        WORD mLocalPort {};
        WORD mRemotePort {};
        LocalRemoteTupleID mTupleID {};

        DataChannelSessionMap mSessions;

        DataChannelSessionMap mPendingResetSessions;
        DataChannelSessionMap mQueuedResetSessions;

        WORD mCurrentAllocationSessionID {};
        WORD mMinAllocationSessionID {0};
        WORD mMaxAllocationSessionID {65534};
        WORD mNextAllocationIncrement {2};

        SocketInfo();
        ~SocketInfo();

        sockaddr_conn getSockAddr(bool localPort);
        void close();

        ElementPtr toDebug() const;
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

      size_t mMaxSessionsPerPort;
      size_t mMaxPorts;

      ISCTPTransportDelegateSubscriptions mSubscriptions;

      ISCTPTransportForDataChannelDelegateSubscriptions mDataChannelSubscriptions;

      std::atomic<States> mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseSecureTransportWeakPtr mSecureTransport;
      PromisePtr mSecureTransportReady;
      PromisePtr mSecureTransportClosed;

      UseICETransportWeakPtr mICETransport;
      IICETransportSubscriptionPtr mICETransportSubscription;

      SocketInfoMap mSockets;

      AllocatedPortMap mAllocatedLocalPorts;
      AllocatedPortMap mAllocatedRemotePorts;

      WORD mCurrentAllocationPort {};
      WORD mMinAllocationPort {5000};
      WORD mMaxAllocationPort {65535};
      WORD mNextAllocationIncremement {2};

      Capabilities mCapabilities;

      DataChannelMap mAnnouncedIncomingDataChannels;

      BufferedQueue mPendingIncomingData;
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

      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, ForSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)

      static ISCTPTransportFactory &singleton();

      virtual ISCTPTransportPtr create(
                                       ISCTPTransportDelegatePtr delegate,
                                       IDTLSTransportPtr transport
                                       );

      virtual ForSecureTransportPtr create(UseSecureTransportPtr transport);

      virtual CapabilitiesPtr getCapabilities();
    };

    class SCTPTransportFactory : public IFactory<ISCTPTransportFactory> {};
  }
}

ZS_DECLARE_TEAR_AWAY_BEGIN(ortc::ISCTPTransport, ortc::internal::SCTPTransport::TearAwayData)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportSubscriptionPtr, ISCTPTransportSubscriptionPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportDelegatePtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportTypes::Capabilities, Capabilities)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::InvalidStateError, InvalidStateError)
  // NOTE: custom tear away forward
  virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError)
  {
    return getDelegate()->getStats();
  }
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_1(start, const Capabilities &)
ZS_DECLARE_TEAR_AWAY_METHOD_0(stop)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_1(subscribe, ISCTPTransportSubscriptionPtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_END()
