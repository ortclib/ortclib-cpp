/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/IICEGatherer.h>

#include <ortc/internal/ortc_ICEGathererRouter.h>

#include <ortc/services/IBackOffTimer.h>
#include <ortc/services/IDNS.h>
#include <ortc/services/ISTUNDiscovery.h>
#include <ortc/services/ITURNSocket.h>
#include <ortc/services/STUNPacket.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Socket.h>
#include <zsLib/ITimer.h>

#include <cryptopp/queue.h>

#define ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING  "ortc/gatherer/interface-name-mapping"
#define ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH  "ortc/gatherer/username-frag-length"
#define ORTC_SETTING_GATHERER_PASSWORD_LENGTH  "ortc/gatherer/password-length"

#define ORTC_SETTING_GATHERER_CANDIDATE_TYPE_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer/canadidate-type-priority-"  // (0..126) << (24)
#define ORTC_SETTING_GATHERER_PROTOCOL_TYPE_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer/protocol-type-priority-"     // (0..0x3) << (24-2)
#define ORTC_SETTING_GATHERER_INTERFACE_TYPE_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer/interface-type-priority-"   // (0..0xF) << (24-6)
#define ORTC_SETTING_GATHERER_ADDRESS_FAMILY_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer/address-family-priority-"   // (0..0x3) << (24-8)

#define ORTC_SETTING_GATHERER_CANDIDATE_TYPE_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer/canadidate-type-unfreeze-priority-"  // (0..126) << (24+6)
#define ORTC_SETTING_GATHERER_PROTOCOL_TYPE_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer/protocol-type-unfreeze-priority-"     // (0..0x3) << (24+4)
#define ORTC_SETTING_GATHERER_INTERFACE_TYPE_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer/interface-type-unfreeze-priority-"   // (0..0xF) << (24-4)
#define ORTC_SETTING_GATHERER_ADDRESS_FAMILY_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer/address-family-unfreeze-priority-"   // (0..0x3) << (24-8)

#define ORTC_SETTING_GATHERER_CREATE_TCP_CANDIDATES  "ortc/gatherer/create-tcp-candidates"
#define ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER  "ortc/gatherer/bind-back-off-timer"

#define ORTC_SETTING_GATHERER_WARM_UP_TIME_AFTER_NEW_INTERFACE_IN_SECONDS "ortc/gatherer/warm-up-time-after-new-interface-in-seconds"

#define ORTC_SETTING_GATHERER_DEFAULT_STUN_KEEP_ALIVE_IN_SECONDS "ortc/gatherer/default-stun-keep-alive-in-seconds"

#define ORTC_SETTING_GATHERER_REFLEXIVE_INACTIVITY_TIMEOUT_IN_SECONDS "ortc/gatherer/reflexive-inactivity-timeout-in-seconds"
#define ORTC_SETTING_GATHERER_RELAY_INACTIVITY_TIMEOUT_IN_SECONDS "ortc/gatherer/relay-inactivity-timeout-in-seconds"

#define ORTC_SETTING_GATHERER_MAX_INCOMING_PACKET_BUFFERING_TIME_IN_SECONDS "ortc/gatherer/max-incoming-packet-buffering-time-in-seconds"
#define ORTC_SETTING_GATHERER_MAX_TOTAL_INCOMING_PACKET_BUFFERING "ortc/gatherer/max-total-packet-buffering"

#define ORTC_SETTING_GATHERER_MAX_PENDING_OUTGOING_TCP_SOCKET_BUFFERING_IN_BYTES "ortc/gatherer/max-pending-outgoing-tcp-socket-buffering-in-bytes"
#define ORTC_SETTING_GATHERER_MAX_CONNECTED_TCP_SOCKET_BUFFERING_IN_BYTES "ortc/gatherer/max-connected-tcp-socket-buffering-in-bytes"

#define ORTC_SETTING_GATHERER_CLEAN_UNUSED_ROUTES_NOT_USED_IN_SECONDS "ortc/gatherer/clean-unused-routes-not-used-in-seconds"

#define ORTC_SETTING_GATHERER_GATHER_PASSIVE_TCP_CANDIDATES "ortc/gatherer/gather-passive-tcp-candidates"

#define ORTC_SETTING_GATHERER_RECHECK_IP_ADDRESSES_IN_SECONDS "ortc/gatherer/recheck-ip-addresses-in-seconds"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICEGatherer);
    ZS_DECLARE_INTERACTION_PROXY(IGathererAsyncDelegate);


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererForICETransport
    #pragma mark

    interaction IICEGathererForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, ForICETransport)

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseICETransport)

      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList)
      typedef IICEGatherer::States States;

      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute)

      static ElementPtr toDebug(ForICETransportPtr gatherer);

      virtual PUID getID() const = 0;

      virtual IICETypes::Components component() const = 0;
      virtual States state() const = 0;

      virtual void installTransport(
                                    ICETransportPtr transport,
                                    const String &remoteUFrag
                                    ) = 0;
      virtual void notifyTransportStateChange(ICETransportPtr transport) = 0;
      virtual void removeTransport(ICETransport &transport) = 0;

      virtual ForICETransportPtr getRTCPGatherer() const = 0;

      virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) = 0;

      virtual bool isContinousGathering() const = 0;
      virtual String getUsernameFrag() const = 0;
      virtual String getPassword() const = 0;

      virtual CandidateListPtr getLocalCandidates() const = 0;

      virtual ICEGathererRouterPtr getGathererRouter() const = 0;

      virtual void removeRoute(RouterRoutePtr routerRoute) = 0;
      virtual void remoteAllRelatedRoutes(ICETransport &transport) = 0;

      virtual bool sendPacket(
                              UseICETransport &transport,
                              RouterRoutePtr routerRoute,
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) = 0;

      virtual void notifyLikelyReflexiveActivity(RouterRoutePtr routerRoute) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IGathererAsyncDelegate
    #pragma mark

    interaction IGathererAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseICETransport)

      virtual void onNotifyDeliverRouteBufferedPackets(
                                                       UseICETransportPtr transport,
                                                       PUID routerRouteID
                                                       )  = 0;
      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer
    #pragma mark

    class ICEGatherer : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IICEGatherer,
                        public IICEGathererForICETransport,
                        public IGathererAsyncDelegate,
                        public IWakeDelegate,
                        public IDNSDelegate,
                        public zsLib::ITimerDelegate,
                        public zsLib::ISocketDelegate,
                        public IBackOffTimerDelegate,
                        public ISTUNDiscoveryDelegate,
                        public ITURNSocketDelegate
    {
    protected:
      struct make_private {};

    public:
      friend class make_shared;

      friend interaction IICEGatherer;
      friend interaction IICEGathererFactory;
      friend interaction IICEGathererForICETransport;

      typedef IICEGatherer::States States;

      enum InternalStates
      {
        InternalState_Pending,
        InternalState_Gathering,
        InternalState_Ready,
        InternalState_ShuttingDown,
        InternalState_Shutdown,
      };

      static const char *toString(InternalStates state);
      static States toState(InternalStates state);

      enum InterfaceTypes
      {
        InterfaceType_First,

        InterfaceType_Unknown = InterfaceType_First,
        InterfaceType_Default,
        InterfaceType_LAN,
        InterfaceType_Tunnel,
        InterfaceType_WLAN,
        InterfaceType_WWAN,
        InterfaceType_VPN,

        InterfaceType_Last = InterfaceType_VPN
      };
      static const char *toString(InterfaceTypes type);
      static InterfaceTypes toInterfaceType(const char *type);

      enum AddressFamilies
      {
        AddressFamily_First,

        AddressFamily_IPv4 = AddressFamily_First,
        AddressFamily_IPv6,

        AddressFamily_Last = AddressFamily_IPv6
      };
      static const char *toString(AddressFamilies family);
      static AddressFamilies toAddressFamily(const char *family);

      enum PreferenceTypes
      {
        PreferenceType_First,

        PreferenceType_Priority = PreferenceType_First,
        PreferenceType_Unfreeze,

        PreferenceType_Last = PreferenceType_Unfreeze
      };
      static const char *toString(PreferenceTypes family);
      static PreferenceTypes toPreferenceType(const char *family);

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseICETransport)
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::ISTUNDiscovery, UseSTUNDiscovery)
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::ITURNSocket, UseTURNSocket)
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::IBackOffTimer, UseBackOffTimer)
      ZS_DECLARE_TYPEDEF_PTR(zsLib::Socket, Socket)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList)
      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute)

      typedef std::list<IPAddress> IPAddressList;

      ZS_DECLARE_CLASS_PTR(HostIPSorter)
      ZS_DECLARE_STRUCT_PTR(HostPort)
      ZS_DECLARE_STRUCT_PTR(ReflexivePort)
      ZS_DECLARE_STRUCT_PTR(RelayPort)
      ZS_DECLARE_STRUCT_PTR(TCPPort)
      ZS_DECLARE_STRUCT_PTR(BufferedPacket)
      ZS_DECLARE_STRUCT_PTR(Route)
      ZS_DECLARE_STRUCT_PTR(InstalledTransport)
      ZS_DECLARE_STRUCT_PTR(Preference)

      typedef std::pair<HostPortPtr, ReflexivePortPtr> HostAndReflexivePortPair;
      typedef std::pair<HostPortPtr, RelayPortPtr> HostAndRelayPortPair;

      typedef std::map<IPAddress, HostPortPtr> IPToHostPortMap;
      typedef std::map<SocketPtr, HostPortPtr> SocketToHostPortMap;
      typedef std::map<UseSTUNDiscoveryPtr, HostAndReflexivePortPair> STUNToReflexivePortMap;
      typedef std::map<UseTURNSocketPtr, HostAndRelayPortPair> TURNToRelayPortMap;

      typedef String Foundation;
      typedef WORD LocalPreference;
      typedef std::map<Foundation, LocalPreference> FoundationToLocalPreferenceMap;

      typedef String CandidateHash;
      typedef std::pair<CandidatePtr, CandidateHash> CandidatePair;
      typedef std::map<CandidateHash, CandidatePair> CandidateMap;

      typedef std::list<ReflexivePortPtr> ReflexivePortList;
      typedef std::map<ITimerPtr, HostAndReflexivePortPair> TimerToReflexivePortMap;

      typedef std::list<RelayPortPtr> RelayPortList;
      typedef std::map<IPAddress, RelayPortPtr> IPToRelayPortMap;
      typedef std::map<ITimerPtr, HostAndRelayPortPair> TimerToRelayPortMap;

      typedef std::pair<HostPortPtr, TCPPortPtr> HostAndTCPPortPair;
      typedef std::map<SocketPtr, HostAndTCPPortPair> SocketToTCPPortMap;
      typedef std::map<CandidatePtr, TCPPortPtr> CandidateToTCPPortMap;

      typedef std::list<BufferedPacketPtr> BufferedPacketList;

      typedef String UsernameFragment;
      typedef PUID TransportID;
      typedef std::map<UsernameFragment, InstalledTransportPtr> TransportMap;

      typedef std::list<InstalledTransportPtr> TransportList;

      typedef PUID RouteID;
      typedef std::map<RouteID, RoutePtr> RouteMap;

      typedef std::pair<CandidatePtr, IPAddress> LocalCandidateRemoteIPPair;
      typedef std::map<LocalCandidateRemoteIPPair, RoutePtr> LocalCandidateRemoteIPRouteMap;

      typedef CryptoPP::ByteQueue ByteQueue;

    public:
      struct ConstructorOptions
      {
        IMessageQueuePtr mQueue;
        IICEGathererDelegatePtr mDelegate;
        Options mOptions;
        String mUsernameFragment;
        String mPassword;
        IICETypes::Components mComponent {Component_RTP};
        ICEGathererPtr mRTPGatherer;
      };

      ICEGatherer(
                  const make_private &,
                  const ConstructorOptions &options
                  );

    protected:
      ICEGatherer(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create()) {}

      void init();

    public:
      virtual ~ICEGatherer();

      static ICEGathererPtr convert(IICEGathererPtr object);
      static ICEGathererPtr convert(ForICETransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IICETransport
      #pragma mark

      static ElementPtr toDebug(ICEGathererPtr gatherer);

      static ICEGathererPtr create(
                                   IICEGathererDelegatePtr delegate,
                                   const Options &options
                                   );

      virtual PUID getID() const override {return mID;}

      virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) override;

      virtual Components component() const override;
      virtual States state() const override;

      virtual ParametersPtr getLocalParameters() const override;
      virtual CandidateListPtr getLocalCandidates() const override;

      virtual IICEGathererPtr createAssociatedGatherer(IICEGathererDelegatePtr delegate) throw(InvalidStateError) override;

      virtual void gather(const Optional<Options> &options = Optional<Options>()) override;

      virtual void close() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IICEGathererForICETransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      // (duplicate) virtual Components component() const = 0;
      // (duplicate) virtual States state() const = 0;

      virtual void installTransport(
                                    ICETransportPtr transport,
                                    const String &remoteUFrag
                                    ) override;
      virtual void notifyTransportStateChange(ICETransportPtr transport) override;
      virtual void removeTransport(ICETransport &transport) override;

      virtual ForICETransportPtr getRTCPGatherer() const override;

      // (duplicate) virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) = 0;
      // (duplicate) virtual CandidateListPtr getLocalCandidates() const = 0;

      virtual bool isContinousGathering() const override;
      virtual String getUsernameFrag() const override {return mUsernameFrag;}
      virtual String getPassword() const override {return mPassword;}

      virtual ICEGathererRouterPtr getGathererRouter() const override;

      virtual void removeRoute(RouterRoutePtr routerRoute) override;
      virtual void remoteAllRelatedRoutes(ICETransport &transport) override;

      virtual bool sendPacket(
                              UseICETransport &transport,
                              RouterRoutePtr routerRoute,
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) override;

      virtual void notifyLikelyReflexiveActivity(RouterRoutePtr routerRoute) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IGathererAsyncDelegate
      #pragma mark

      virtual void onNotifyDeliverRouteBufferedPackets(
                                                       UseICETransportPtr transport,
                                                       PUID routerRouteID
                                                       ) override;

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IDNSDelegate
      #pragma mark

      virtual void onLookupCompleted(IDNSQueryPtr query) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ISocketDelegate
      #pragma mark

      virtual void onReadReady(SocketPtr socket) override;
      virtual void onWriteReady(SocketPtr socket) override;
      virtual void onException(SocketPtr socket) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IBackOffDelegate
      #pragma mark

      virtual void onBackOffTimerStateChanged(
                                              IBackOffTimerPtr timer,
                                              IBackOffTimer::States state
                                              ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ISTUNDiscoveryDelegate
      #pragma mark

      virtual void onSTUNDiscoverySendPacket(
                                             ISTUNDiscoveryPtr discovery,
                                             IPAddress destination,
                                             SecureByteBlockPtr packet
                                             ) override;

      virtual void onSTUNDiscoveryCompleted(ISTUNDiscoveryPtr discovery) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ITURNSocketDelegate
      #pragma mark

      virtual void onTURNSocketStateChanged(
                                            ITURNSocketPtr socket,
                                            TURNSocketStates state
                                            ) override;

      virtual void handleTURNSocketReceivedPacket(
                                                  ITURNSocketPtr socket,
                                                  IPAddress source,
                                                  const BYTE *packet,
                                                  size_t packetLengthInBytes
                                                  ) override;

      virtual bool notifyTURNSocketSendPacket(
                                              ITURNSocketPtr socket,
                                              IPAddress destination,
                                              const BYTE *packet,
                                              size_t packetLengthInBytes
                                              ) override;

      virtual void onTURNSocketWriteReady(ITURNSocketPtr socket) override;


    public:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::HostIPSorter
      #pragma mark

      class HostIPSorter
      {

      public:
        typedef ULONG OrderID;
        typedef String InterfaceName;

        ZS_DECLARE_STRUCT_PTR(Data)
        ZS_DECLARE_STRUCT_PTR(QueryData)
        ZS_DECLARE_STRUCT_PTR(InterfaceNameMappingInfo)

        //---------------------------------------------------------------------
        struct Data {
          InterfaceName mInterfaceName;
          InterfaceName mInterfaceDescription;
          InterfaceTypes mInterfaceType {InterfaceType_Unknown};
          FilterPolicies mFilterPolicy {FilterPolicy_None};

          String mHostName;
          IPAddress mIP;
          OrderID mOrderIndex {};
          ULONG mAdapterMetric {};
          ULONG mIndex {};
          bool mIsTemporaryIP {false};

          ElementPtr toDebug() const;
        };

        //---------------------------------------------------------------------
        struct QueryData
        {
          IDNSQueryPtr mQuery;
          IPAddressList mResolvedIPs;
          DataPtr mOriginalData;
        };

        //---------------------------------------------------------------------
        struct InterfaceNameMappingInfo
        {
          String mInterfaceNameRegularExpression;
          InterfaceTypes mInterfaceType {InterfaceType_Unknown};
          OrderID mOrderIndex {};

          static InterfaceNameMappingInfo create(ElementPtr el);

          ElementPtr createElement(const char *objectName = "interface") const;
          bool hasData() const;

          ElementPtr toDebug() const;
        };

        typedef std::list<DataPtr> DataList;

        typedef PUID QueryID;
        typedef std::map<QueryID, QueryData> QueryMap;

        typedef std::list<InterfaceNameMappingInfo> InterfaceMappingList;

        static bool compareLocalIPs(const DataPtr &data1, const DataPtr &data2);

        static DataPtr prepare(
                               const IPAddress &ip,
                               const Options &options
                               );
        static DataPtr prepare(
                               const char *hostName,
                               const IPAddress &ip,
                               const Options &options
                               );

        static DataPtr prepare(
                               const char *interfaceName,
                               const char *hostName,
                               const IPAddress &ip,
                               const InterfaceMappingList &prefs,
                               const Options &options
                               );
        static DataPtr prepare(
                               const char *interfaceName,
                               const char *interfaceDescription,
                               const IPAddress &ip,
                               ULONG metric,
                               const InterfaceMappingList &prefs,
                               const Options &options
                               );

        static DataPtr prepare(
                               const char *interfaceName,
                               const IPAddress &ip,
                               const InterfaceMappingList &prefs,
                               const Options &options
                               );

        static void fixMapping(
                               Data &ioData,
                               const InterfaceMappingList &prefs
                               );

        static void fixPolicy(
                              Data &ioData,
                              const Options &options
                              );
        static void sort(DataList &ioDataList);
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::ReflexivePort
      #pragma mark

      struct ReflexivePort
      {
        AutoPUID mID;

        Server mServer;
        UseSTUNDiscoveryPtr mSTUNDiscovery;

        String mOptionsHash;
        CandidatePtr mCandidate;

        Time mLastActivity;
        ITimerPtr mInactivityTimer;

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::RelayPort
      #pragma mark

      struct RelayPort
      {
        AutoPUID mID;

        Server mServer;
        UseTURNSocketPtr mTURNSocket;
        IPAddress mServerResponseIP;

        String mOptionsHash;
        CandidatePtr mRelayCandidate;
        CandidatePtr mReflexiveCandidate;

        Time mLastActivity;
        ITimerPtr mInactivityTimer;

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::HostPort
      #pragma mark

      struct HostPort
      {
        AutoPUID mID;

        HostIPSorter::DataPtr mHostData;

        String mBoundOptionsHash;
        CandidatePtr mCandidateUDP;
        IPAddress mBoundUDPIP;
        SocketPtr mBoundUDPSocket;
        UseBackOffTimerPtr mBindUDPBackOffTimer;
        
        CandidatePtr mCandidateTCPPassive;
        CandidatePtr mCandidateTCPActive;

        IPAddress mBoundTCPIP;
        SocketPtr mBoundTCPSocket;
        UseBackOffTimerPtr mBindTCPBackOffTimer;

        bool mWarmUpAfterBinding {true};

        String mReflexiveOptionsHash;
        ReflexivePortList mReflexivePorts;

        String mRelayOptionsHash;
        RelayPortList mRelayPorts;
        IPToRelayPortMap mIPToRelayPortMapping;

        SocketToTCPPortMap mTCPPorts;

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::TCPPort
      #pragma mark

      struct TCPPort
      {
        AutoPUID mID;

        bool mConnected {false};
        bool mWriteReady {false};

        CandidatePtr mCandidate;

        IPAddress mRemoteIP;
        SocketPtr mSocket;
        ByteQueue mIncomingBuffer;
        ByteQueue mOutgoingBuffer;

        TransportID mTransportID {0};
        UseICETransportWeakPtr mTransport;

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::BufferedPacket
      #pragma mark

      struct BufferedPacket
      {
        Time mTimestamp;
        RouterRoutePtr mRouterRoute;

        STUNPacketPtr mSTUNPacket;
        String mRFrag;

        SecureByteBlockPtr mBuffer;

        ElementPtr toDebug() const;
      };
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::Route
      #pragma mark

      struct Route
      {
        AutoPUID mID;

        PUID mOuterObjectID {};

        RouterRoutePtr mRouterRoute;

        Time mLastUsed;
        CandidatePtr mLocalCandidate;

        TransportID mTransportID {};
        UseICETransportWeakPtr mTransport;

        HostPortPtr mHostPort;    // send via host UDP packet
        RelayPortPtr mRelayPort;  // send via relay port
        TCPPortPtr mTCPPort;      // send via TCP socket

        void trace(const char *function, const char *message = NULL) const;
        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::InstalledTransport
      #pragma mark

      struct InstalledTransport
      {
        TransportID mTransportID {};
        UseICETransportWeakPtr mTransport;

        ElementPtr toDebug() const;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer::Preference
      #pragma mark

      struct Preference
      {
        PUID mOuterObjectID {};
        PreferenceTypes mType {PreferenceType_Priority};
        DWORD *mCandidateTypePreferences {NULL};
        DWORD *mProtocolTypePreferences {NULL};
        DWORD *mInterfaceTypePreferences {NULL};
        DWORD *mAddressFamilyPreferences {NULL};

        Preference();
        Preference(PreferenceTypes type);
        ~Preference();

        void getSettingsPrefixes(
                                 const char * &outCandidateType,
                                 const char * &outProtocolType,
                                 const char * &outInterfaceType,
                                 const char * &outAddressFamily
                                 ) const;
        void load();
        void save() const;

        Log::Params log(const char *message) const;
        ElementPtr toDebug() const;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isComplete() const;
      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepRecheckIPTimer();
      bool stepCalculateOptionsHash();
      bool stepResolveHostIPs();
      bool stepGetHostIPs();
      void stepGetHostIPs_WinRT();
      void stepGetHostIPs_Win32();
      void stepGetHostIPs_ifaddr();
      bool stepCalculateHostsHash();
      bool stepFixHostPorts();
      bool stepBindHostPorts();
      bool stepCheckTransportsNeedWarmth();
      bool stepWarmUpAfterInterfaceBinding();
      bool stepSetupReflexive();
      bool stepTearDownReflexive();
      bool stepSetupRelay();
      bool stepTearDownRelay();
      bool stepCleanPendingShutdownTURNSockets();
      bool stepCheckIfReady();

      void cancel();

      void setState(InternalStates state);
      void setError(WORD error, const char *reason = NULL);

      bool hasSTUNServers();
      bool hasTURNServers();
      bool isServerType(
                        const Server &server,
                        const char *urlPrefix
                        ) const;

      StringList createDNSLookupString(
                                       const Server &server,
                                       const char *urlPrefix
                                       ) const;

      bool needsHostPort(HostIPSorter::DataPtr hostData);

      void shutdown(HostPortPtr hostPort);
      void shutdown(
                    ReflexivePortPtr reflexivePort,
                    HostPortPtr ownerHostPort
                    );
      void shutdown(
                    RelayPortPtr relayPort,
                    HostPortPtr ownerHostPort
                    );
      void shutdown(
                    TCPPortPtr tcpPort,
                    HostPortPtr ownerHostPort
                    );

      SocketPtr bind(
                     bool firstAttempt,
                     IPAddress &ioBindIP,
                     IICETypes::Protocols protocol
                     );

      CandidatePtr createCandidate(
                                   HostIPSorter::DataPtr hostData,
                                   IICETypes::CandidateTypes candidateType,
                                   const IPAddress &boundIP,
                                   IICETypes::Protocols protocol = IICETypes::Protocol_UDP,
                                   IICETypes::TCPCandidateTypes tcpType = IICETypes::TCPCandidateType_Active
                                   );
      CandidatePtr createCandidate(
                                   HostIPSorter::DataPtr hostData,
                                   IICETypes::CandidateTypes candidateType,
                                   const IPAddress &baseIP,
                                   const IPAddress &relatedIP,
                                   const IPAddress &boundIP,
                                   const Server &server
                                   );

      void addCandidate(
                        const HostIPSorter::Data &hostData,
                        const IPAddress &ip,
                        CandidatePtr candidate
                        );
      void removeCandidate(CandidatePtr candidate);

      bool isFiltered(
                      FilterPolicies policy,
                      const IPAddress &ip,
                      const Candidate &candidate
                      ) const;

      bool read(
                HostPortPtr hostPort,
                SocketPtr socket
                );
      void read(
                HostPort &hostPort,
                TCPPort &tcpPort
                );

      void write(
                 HostPort &hostPort,
                 SocketPtr socket
                 );
      bool writeIfTCPPort(SocketPtr socket);

      void close(
                 HostPortPtr hostPort,
                 SocketPtr socket
                 );
      void close(
                 TCPPortPtr tcpPort,
                 HostPortPtr hostPort
                 );

      SecureByteBlockPtr handleIncomingPacket(
                                              CandidatePtr localCandidate,
                                              const IPAddress &remoteIP,
                                              STUNPacketPtr stunPacket
                                              );
      void handleIncomingPacket(
                                CandidatePtr localCandidate,
                                const IPAddress &remoteIP,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                );

      CandidatePtr findSentFromLocalCandidate(RouterRoutePtr routerRoute);

      RoutePtr installRoute(
                            RouterRoutePtr routerRoute,
                            UseICETransportPtr transport
                            );
      RoutePtr installRoute(
                            CandidatePtr sentFromLocalCandidate,
                            const IPAddress &remoteIP,
                            UseICETransportPtr transport
                            );

      void fix(STUNPacketPtr stunPacket) const;

      void removeAllRelatedRoutes(
                                  TransportID transportID,
                                  UseICETransportPtr transportIfAvailable
                                  );

      bool sendUDPPacket(
                         SocketPtr socket,
                         const IPAddress &boundIP,
                         const IPAddress &remoteIP,
                         const BYTE *buffer,
                         size_t bufferSizeInBytes
                         );

      bool shouldKeepWarm() const;
      bool shouldWarmUpAfterInterfaceBinding() const;

      void fixSTUNParserOptions(const STUNPacketPtr &packet);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (data)
      #pragma mark

      ICEGathererWeakPtr mThisWeak;
      AutoPUID mID;

      ICEGathererRouterPtr mGathererRouter;

      ICEGathererPtr mGracefulShutdownReference;

      IICEGathererDelegateSubscriptions mSubscriptions;
      IICEGathererSubscriptionPtr mDefaultSubscription;

      InternalStates mCurrentState {InternalState_Pending};


      WORD mLastError {};
      String mLastErrorReason;

      Components mComponent {Component_RTP};
      String mUsernameFrag;
      String mPassword;

      ICEGathererPtr mRTPGatherer;
      ICEGathererWeakPtr mRTCPGatherer;

      Options mOptions;

      Preference mPreferences[PreferenceType_Last+1];
      HostIPSorter::InterfaceMappingList mInterfaceMappings;
      bool mCreateTCPCandidates {true};

      bool mGetLocalIPsNow {true};
      Seconds mRecheckIPsDuration {};
      ITimerPtr mRecheckIPsTimer;
      HostIPSorter::DataList mPendingHostIPs;
      HostIPSorter::DataList mResolvedHostIPs;
      HostIPSorter::QueryMap mResolveHostIPQueries;

      String mHostsHash;
      String mOptionsHash;

      WORD mDefaultPort {0};

      String mLastFixedHostPortsHostsHash;
      String mLastBoundHostPortsHostHash;
      String mLastReflexiveHostsHash;
      String mLastRelayHostsHash;

      IPToHostPortMap mHostPorts;
      SocketToHostPortMap mHostPortSockets;

      STUNToReflexivePortMap mSTUNDiscoveries;
      TURNToRelayPortMap mTURNSockets;
      TURNToRelayPortMap mShutdownTURNSockets;

      String mHasSTUNServersOptionsHash;
      bool mHasSTUNServers {false};

      String mHasTURNServersOptionsHash;
      bool mHasTURNServers {false};

      FoundationToLocalPreferenceMap mLastLocalPreference;

      CandidateMap mNotifiedCandidates;
      CandidateMap mLocalCandidates;

      bool mKeepWarmSinceJustCreated {true};

      String mWarmUpAfterNewInterfaceBindingHostsHash;
      Time mWarmUpAfterNewInterfaceBindingUntil;
      ITimerPtr mWarmUpAterNewInterfaceBindingTimer;

      Seconds mReflexiveInactivityTime {};
      TimerToReflexivePortMap mReflexiveInactivityTimers;

      Seconds mRelayInactivityTime {};
      TimerToRelayPortMap mRelayInactivityTimers;

      bool mGatherPassiveTCP {false};
      SocketToTCPPortMap mTCPPorts;
      CandidateToTCPPortMap mTCPCandidateToTCPPorts;
      size_t mMaxTCPBufferingSizePendingConnection {};
      size_t mMaxTCPBufferingSizeConnected {};

      ITimerPtr mCleanUpBufferingTimer;
      Seconds mMaxBufferingTime {};
      size_t mMaxTotalBuffers {};
      BufferedPacketList mBufferedPackets;

      LocalCandidateRemoteIPRouteMap mQuickSearchRoutes;
      RouteMap mRoutes;
      ITimerPtr mCleanUnusedRoutesTimer;
      Seconds mCleanUnusedRoutesDuration {};

      bool mTransportsChanged {true};
      bool mTransportsStillNeedsCandidates {true};
      TransportMap mInstalledTransports;
      TransportList mPendingTransports;

      STUNPacket::ParseOptions mSTUNPacketParseOptions;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererFactory
    #pragma mark

    interaction IICEGathererFactory
    {
      static IICEGathererFactory &singleton();

      virtual ICEGathererPtr create(
                                    IICEGathererDelegatePtr delegate,
                                    const IICEGathererTypes::Options &options
                                    );
    };

    class ICEGathererFactory : public IFactory<IICEGathererFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IGathererAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IGathererAsyncDelegate::UseICETransportPtr, UseICETransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD_2(onNotifyDeliverRouteBufferedPackets, UseICETransportPtr, PUID)
ZS_DECLARE_PROXY_METHOD_1(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_END()
