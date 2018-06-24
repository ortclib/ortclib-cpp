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
#include <zsLib/RangeSelection.h>

#include <cryptopp/queue.h>

#define ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING  "ortc/gatherer/interface-name-mapping"
#define ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH  "ortc/gatherer/username-frag-length"
#define ORTC_SETTING_GATHERER_PASSWORD_LENGTH  "ortc/gatherer/password-length"

#define ORTC_SETTING_GATHERER_PORT_RESTRICTIONS "ortc/gatherer/port-restrictions" // use zsLib::RangeSelection<uint16_t> to set this setting

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
    //
    // IICEGathererForICETransport
    //

    interaction IICEGathererForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, ForICETransport);

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseICETransport);

      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList);
      typedef IICEGatherer::States States;

      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute);

      static ElementPtr toDebug(ForICETransportPtr gatherer) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual IICETypes::Components component() const noexcept = 0;
      virtual States state() const noexcept = 0;

      virtual void installTransport(
                                    ICETransportPtr transport,
                                    const String &remoteUFrag
                                    ) noexcept = 0;
      virtual void notifyTransportStateChange(ICETransportPtr transport) noexcept = 0;
      virtual void removeTransport(ICETransport &transport) noexcept = 0;

      virtual ForICETransportPtr getRTCPGatherer() const noexcept = 0;

      virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) noexcept = 0;

      virtual bool isContinousGathering() const noexcept = 0;
      virtual String getUsernameFrag() const noexcept = 0;
      virtual String getPassword() const noexcept = 0;

      virtual CandidateListPtr getLocalCandidates() const noexcept = 0;

      virtual ICEGathererRouterPtr getGathererRouter() const noexcept = 0;

      virtual void removeRoute(RouterRoutePtr routerRoute) noexcept = 0;
      virtual void remoteAllRelatedRoutes(ICETransport &transport) noexcept = 0;

      virtual bool sendPacket(
                              UseICETransport &transport,
                              RouterRoutePtr routerRoute,
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) noexcept = 0;

      virtual void notifyLikelyReflexiveActivity(RouterRoutePtr routerRoute) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IGathererAsyncDelegate
    //

    interaction IGathererAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseICETransport);

      virtual void onNotifyDeliverRouteBufferedPackets(
                                                       UseICETransportPtr transport,
                                                       PUID routerRouteID
                                                       ) = 0;
      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ICEGatherer
    //

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
      friend struct SocketDelegate;

      typedef IICEGatherer::States States;

      enum InternalStates
      {
        InternalState_Pending,
        InternalState_Gathering,
        InternalState_Ready,
        InternalState_ShuttingDown,
        InternalState_Shutdown,
      };

      static const char *toString(InternalStates state) noexcept;
      static States toState(InternalStates state) noexcept;

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
      static const char *toString(InterfaceTypes type) noexcept;
      static InterfaceTypes toInterfaceType(const char *type) noexcept;

      enum AddressFamilies
      {
        AddressFamily_First,

        AddressFamily_IPv4 = AddressFamily_First,
        AddressFamily_IPv6,

        AddressFamily_Last = AddressFamily_IPv6
      };
      static const char *toString(AddressFamilies family) noexcept;
      static AddressFamilies toAddressFamily(const char *family) noexcept;

      enum PreferenceTypes
      {
        PreferenceType_First,

        PreferenceType_Priority = PreferenceType_First,
        PreferenceType_Unfreeze,

        PreferenceType_Last = PreferenceType_Unfreeze
      };
      static const char *toString(PreferenceTypes family) noexcept;
      static PreferenceTypes toPreferenceType(const char *family) noexcept;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseICETransport);
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::ISTUNDiscovery, UseSTUNDiscovery);
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::ITURNSocket, UseTURNSocket);
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::IBackOffTimer, UseBackOffTimer);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::Socket, Socket);
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate);
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateList, CandidateList);
      ZS_DECLARE_TYPEDEF_PTR(ICEGathererRouter::Route, RouterRoute);

      typedef std::list<IPAddress> IPAddressList;

      ZS_DECLARE_STRUCT_PTR(SocketDelegate);
      ZS_DECLARE_CLASS_PTR(HostIPSorter);
      ZS_DECLARE_STRUCT_PTR(HostPort);
      ZS_DECLARE_STRUCT_PTR(ReflexivePort);
      ZS_DECLARE_STRUCT_PTR(RelayPort);
      ZS_DECLARE_STRUCT_PTR(TCPPort);
      ZS_DECLARE_STRUCT_PTR(BufferedPacket);
      ZS_DECLARE_STRUCT_PTR(Route);
      ZS_DECLARE_STRUCT_PTR(InstalledTransport);
      ZS_DECLARE_STRUCT_PTR(Preference);

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
      typedef zsLib::RangeSelection<WORD> RangeSelection;

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
                  ) noexcept;

    protected:
      ICEGatherer(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create()) {}

      void init() noexcept;

    public:
      virtual ~ICEGatherer() noexcept;

      static ICEGathererPtr convert(IICEGathererPtr object) noexcept;
      static ICEGathererPtr convert(ForICETransportPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IICETransport
      //

      static ElementPtr toDebug(ICEGathererPtr gatherer) noexcept;

      static ICEGathererPtr create(
                                   IICEGathererDelegatePtr delegate,
                                   const Options &options
                                   ) noexcept;

      PUID getID() const noexcept override {return mID;}

      IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) noexcept override;

      Components component() const noexcept override;
      States state() const noexcept override;

      ParametersPtr getLocalParameters() const noexcept override;
      CandidateListPtr getLocalCandidates() const noexcept override;

      IICEGathererPtr createAssociatedGatherer(IICEGathererDelegatePtr delegate) noexcept(false) override; // throws InvalidStateError

      void gather(const Optional<Options> &options = Optional<Options>()) noexcept override;

      void close() noexcept override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IICEGathererForICETransport
      //

      // (duplicate) virtual PUID getID() const;

      // (duplicate) virtual Components component() const = 0;
      // (duplicate) virtual States state() const = 0;

      void installTransport(
                            ICETransportPtr transport,
                            const String &remoteUFrag
                            ) noexcept override;
      void notifyTransportStateChange(ICETransportPtr transport) noexcept override;
      void removeTransport(ICETransport &transport) noexcept override;

      ForICETransportPtr getRTCPGatherer() const noexcept override;

      // (duplicate) virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) noexcept = 0;
      // (duplicate) virtual CandidateListPtr getLocalCandidates() const noexcept = 0;

      bool isContinousGathering() const noexcept override;
      String getUsernameFrag() const noexcept override {return mUsernameFrag;}
      String getPassword() const noexcept override {return mPassword;}

      ICEGathererRouterPtr getGathererRouter() const noexcept override;

      void removeRoute(RouterRoutePtr routerRoute) noexcept override;
      void remoteAllRelatedRoutes(ICETransport &transport) noexcept override;

      bool sendPacket(
                      UseICETransport &transport,
                      RouterRoutePtr routerRoute,
                      const BYTE *buffer,
                      size_t bufferSizeInBytes
                      ) noexcept override;

      void notifyLikelyReflexiveActivity(RouterRoutePtr routerRoute) noexcept override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IGathererAsyncDelegate
      //

      void onNotifyDeliverRouteBufferedPackets(
                                               UseICETransportPtr transport,
                                               PUID routerRouteID
                                               ) override;

      void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IDNSDelegate
      //

      void onLookupCompleted(IDNSQueryPtr query) override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => ISocketDelegate
      //

      void onReadReady(SocketPtr socket) override;
      void onWriteReady(SocketPtr socket) override;
      void onException(SocketPtr socket) override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => IBackOffDelegate
      //

      void onBackOffTimerStateChanged(
                                      IBackOffTimerPtr timer,
                                      IBackOffTimer::States state
                                      ) override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => ISTUNDiscoveryDelegate
      //

      void onSTUNDiscoverySendPacket(
                                     ISTUNDiscoveryPtr discovery,
                                     IPAddress destination,
                                     SecureByteBlockPtr packet
                                     ) override;

      void onSTUNDiscoveryCompleted(ISTUNDiscoveryPtr discovery) override;

      //-----------------------------------------------------------------------
      //
      // ICEGatherer => ITURNSocketDelegate
      //

      void onTURNSocketStateChanged(
                                    ITURNSocketPtr socket,
                                    TURNSocketStates state
                                    ) override;

      void handleTURNSocketReceivedPacket(
                                          ITURNSocketPtr socket,
                                          IPAddress source,
                                          const BYTE *packet,
                                          size_t packetLengthInBytes
                                          ) noexcept override;

      bool notifyTURNSocketSendPacket(
                                      ITURNSocketPtr socket,
                                      IPAddress destination,
                                      const BYTE *packet,
                                      size_t packetLengthInBytes
                                      ) noexcept override;

      void onTURNSocketWriteReady(ITURNSocketPtr socket) override;


    public:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::SocketDelegate
      //

      struct SocketDelegate : public zsLib::MessageQueueAssociator,
                              public zsLib::ISocketDelegate
      {
      protected:
        struct make_private {};

      public:
        SocketDelegate(
                       const make_private &,
                       IMessageQueuePtr queue,
                       ICEGathererPtr gatherer
                       ) noexcept :
          MessageQueueAssociator(queue),
          iceGatherer_(gatherer)
        {
        }

        static SocketDelegatePtr create(
                                        IMessageQueuePtr queue,
                                        ICEGathererPtr gatherer
                                        ) noexcept
        {
          auto pThis(std::make_shared<SocketDelegate>(make_private{}, queue, gatherer));
          return pThis;
        }

        void onReadReady(SocketPtr socket) override
        {
          auto outer = iceGatherer_.lock();
          if (!outer) return;
          outer->onReadReady(socket);
        }

        void onWriteReady(SocketPtr socket) override
        {
          auto outer = iceGatherer_.lock();
          if (!outer) return;
          outer->onWriteReady(socket);
        }

        void onException(SocketPtr socket) override
        {
          auto outer = iceGatherer_.lock();
          if (!outer) return;
          outer->onException(socket);
        }

        AutoPUID id_;
        ICEGathererWeakPtr iceGatherer_;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::HostIPSorter
      //

      class HostIPSorter
      {

      public:
        typedef ULONG OrderID;
        typedef String InterfaceName;

        ZS_DECLARE_STRUCT_PTR(Data);
        ZS_DECLARE_STRUCT_PTR(QueryData);
        ZS_DECLARE_STRUCT_PTR(InterfaceNameMappingInfo);

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

          ElementPtr toDebug() const noexcept;
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

          static InterfaceNameMappingInfo create(ElementPtr el) noexcept;

          ElementPtr createElement(const char *objectName = "interface") const noexcept;
          bool hasData() const noexcept;

          ElementPtr toDebug() const noexcept;
        };

        typedef std::list<DataPtr> DataList;

        typedef PUID QueryID;
        typedef std::map<QueryID, QueryData> QueryMap;

        typedef std::list<InterfaceNameMappingInfo> InterfaceMappingList;

        static bool compareLocalIPs(const DataPtr &data1, const DataPtr &data2) noexcept;

        static DataPtr prepare(
                               const IPAddress &ip,
                               const Options &options
                               ) noexcept;
        static DataPtr prepare(
                               const char *hostName,
                               const IPAddress &ip,
                               const Options &options
                               ) noexcept;

        static DataPtr prepare(
                               const char *interfaceName,
                               const char *hostName,
                               const IPAddress &ip,
                               const InterfaceMappingList &prefs,
                               const Options &options
                               ) noexcept;
        static DataPtr prepare(
                               const char *interfaceName,
                               const char *interfaceDescription,
                               const IPAddress &ip,
                               ULONG metric,
                               const InterfaceMappingList &prefs,
                               const Options &options
                               ) noexcept;

        static DataPtr prepare(
                               const char *interfaceName,
                               const IPAddress &ip,
                               const InterfaceMappingList &prefs,
                               const Options &options
                               ) noexcept;

        static void fixMapping(
                               Data &ioData,
                               const InterfaceMappingList &prefs
                               ) noexcept;

        static void fixPolicy(
                              Data &ioData,
                              const Options &options
                              ) noexcept;
        static void sort(DataList &ioDataList) noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::ReflexivePort
      //

      struct ReflexivePort
      {
        AutoPUID mID;

        Server mServer;
        UseSTUNDiscoveryPtr mSTUNDiscovery;

        String mOptionsHash;
        CandidatePtr mCandidate;

        Time mLastActivity;
        ITimerPtr mInactivityTimer;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::RelayPort
      //

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

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::HostPort
      //

      struct HostPort
      {
        AutoPUID mID;

        HostIPSorter::DataPtr mHostData;

        String mBoundOptionsHash;
        CandidatePtr mCandidateUDP;
        IPAddress mBoundUDPIP;
        SocketPtr mBoundUDPSocket;
        SocketDelegatePtr mBoundUDPSocketDelegateHolder;
        UseBackOffTimerPtr mBindUDPBackOffTimer;
        
        CandidatePtr mCandidateTCPPassive;
        CandidatePtr mCandidateTCPActive;

        IPAddress mBoundTCPIP;
        SocketPtr mBoundTCPSocket;
        SocketDelegatePtr mBoundTCPSocketDelegateHolder;
        UseBackOffTimerPtr mBindTCPBackOffTimer;

        bool mWarmUpAfterBinding {true};

        String mReflexiveOptionsHash;
        ReflexivePortList mReflexivePorts;

        String mRelayOptionsHash;
        RelayPortList mRelayPorts;
        IPToRelayPortMap mIPToRelayPortMapping;

        SocketToTCPPortMap mTCPPorts;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::TCPPort
      //

      struct TCPPort
      {
        AutoPUID mID;

        bool mConnected {false};
        bool mWriteReady {false};

        CandidatePtr mCandidate;

        IPAddress mRemoteIP;
        SocketPtr mSocket;
        SocketDelegatePtr mSocketDelegateHolder;
        ByteQueue mIncomingBuffer;
        ByteQueue mOutgoingBuffer;

        TransportID mTransportID {0};
        UseICETransportWeakPtr mTransport;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::BufferedPacket
      //

      struct BufferedPacket
      {
        Time mTimestamp;
        RouterRoutePtr mRouterRoute;

        STUNPacketPtr mSTUNPacket;
        String mRFrag;

        SecureByteBlockPtr mBuffer;

        ElementPtr toDebug() const noexcept;
      };
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::Route
      //

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

        void trace(
                   const char *function = NULL,
                   const char *message = NULL
                   ) const noexcept;
        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::InstalledTransport
      //

      struct InstalledTransport
      {
        TransportID mTransportID {};
        UseICETransportWeakPtr mTransport;

        ElementPtr toDebug() const noexcept;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ICEGatherer::Preference
      //

      struct Preference
      {
        PUID mOuterObjectID {};
        PreferenceTypes mType {PreferenceType_Priority};
        DWORD *mCandidateTypePreferences {NULL};
        DWORD *mProtocolTypePreferences {NULL};
        DWORD *mInterfaceTypePreferences {NULL};
        DWORD *mAddressFamilyPreferences {NULL};

        Preference() noexcept;
        Preference(PreferenceTypes type) noexcept;
        ~Preference() noexcept;

        void getSettingsPrefixes(
                                 const char * &outCandidateType,
                                 const char * &outProtocolType,
                                 const char * &outInterfaceType,
                                 const char * &outAddressFamily
                                 ) const noexcept;
        void load() noexcept;
        void save() const noexcept;

        Log::Params log(const char *message) const noexcept;
        ElementPtr toDebug() const noexcept;
      };

    protected:
      //-----------------------------------------------------------------------
      //
      // ICETransport => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      ElementPtr toDebug() const noexcept;

      bool isComplete() const noexcept;
      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      bool stepRecheckIPTimer() noexcept;
      bool stepCalculateOptionsHash() noexcept;
      bool stepResolveHostIPs() noexcept;
      bool stepGetHostIPs() noexcept;
      void stepGetHostIPs_WinUWP() noexcept;
      void stepGetHostIPs_Win32() noexcept;
      void stepGetHostIPs_ifaddr() noexcept;
      bool stepCalculateHostsHash() noexcept;
      bool stepFixHostPorts() noexcept;
      bool stepBindHostPorts() noexcept;
      bool stepCheckTransportsNeedWarmth() noexcept;
      bool stepWarmUpAfterInterfaceBinding() noexcept;
      bool stepSetupReflexive() noexcept;
      bool stepTearDownReflexive() noexcept;
      bool stepSetupRelay() noexcept;
      bool stepTearDownRelay() noexcept;
      bool stepCleanPendingShutdownTURNSockets() noexcept;
      bool stepCheckIfReady() noexcept;

      void cancel() noexcept;

      void setState(InternalStates state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

      bool hasSTUNServers() noexcept;
      bool hasTURNServers() noexcept;
      bool isServerType(
                        const Server &server,
                        const char *urlPrefix
                        ) const noexcept;

      StringList createDNSLookupString(
                                       const Server &server,
                                       const char *urlPrefix
                                       ) const noexcept;

      bool needsHostPort(HostIPSorter::DataPtr hostData) noexcept;

      void shutdown(HostPortPtr hostPort) noexcept;
      void shutdown(
                    ReflexivePortPtr reflexivePort,
                    HostPortPtr ownerHostPort
                    ) noexcept;
      void shutdown(
                    RelayPortPtr relayPort,
                    HostPortPtr ownerHostPort
                    ) noexcept;
      void shutdown(
                    TCPPortPtr tcpPort,
                    HostPortPtr ownerHostPort
                    ) noexcept;

      void bind(
                SocketPtr &outSocket,
                SocketDelegatePtr &outSocketDelegate,
                bool firstAttempt,
                IPAddress &ioBindIP,
                IICETypes::Protocols protocol
                ) noexcept;

      CandidatePtr createCandidate(
                                   HostIPSorter::DataPtr hostData,
                                   IICETypes::CandidateTypes candidateType,
                                   const IPAddress &boundIP,
                                   IICETypes::Protocols protocol = IICETypes::Protocol_UDP,
                                   IICETypes::TCPCandidateTypes tcpType = IICETypes::TCPCandidateType_Active
                                   ) noexcept;
      CandidatePtr createCandidate(
                                   HostIPSorter::DataPtr hostData,
                                   IICETypes::CandidateTypes candidateType,
                                   const IPAddress &baseIP,
                                   const IPAddress &relatedIP,
                                   const IPAddress &boundIP,
                                   const Server &server
                                   ) noexcept;

      void addCandidate(
                        const HostIPSorter::Data &hostData,
                        const IPAddress &ip,
                        CandidatePtr candidate
                        ) noexcept;
      void removeCandidate(CandidatePtr candidate) noexcept;

      bool isFiltered(
                      FilterPolicies policy,
                      const IPAddress &ip,
                      const Candidate &candidate
                      ) const noexcept;

      bool read(
                HostPortPtr hostPort,
                SocketPtr socket
                ) noexcept;
      void read(
                HostPort &hostPort,
                TCPPort &tcpPort
                ) noexcept;

      void write(
                 HostPort &hostPort,
                 SocketPtr socket
                 ) noexcept;
      bool writeIfTCPPort(SocketPtr socket) noexcept;

      void close(
                 HostPortPtr hostPort,
                 SocketPtr socket
                 ) noexcept;
      void close(
                 TCPPortPtr tcpPort,
                 HostPortPtr hostPort
                 ) noexcept;

      SecureByteBlockPtr handleIncomingPacket(
                                              CandidatePtr localCandidate,
                                              const IPAddress &remoteIP,
                                              STUNPacketPtr stunPacket
                                              ) noexcept;
      void handleIncomingPacket(
                                CandidatePtr localCandidate,
                                const IPAddress &remoteIP,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) noexcept;

      CandidatePtr findSentFromLocalCandidate(RouterRoutePtr routerRoute) noexcept;

      RoutePtr installRoute(
                            RouterRoutePtr routerRoute,
                            UseICETransportPtr transport
                            ) noexcept;
      RoutePtr installRoute(
                            CandidatePtr sentFromLocalCandidate,
                            const IPAddress &remoteIP,
                            UseICETransportPtr transport
                            ) noexcept;

      void fix(STUNPacketPtr stunPacket) const noexcept;

      void removeAllRelatedRoutes(
                                  TransportID transportID,
                                  UseICETransportPtr transportIfAvailable
                                  ) noexcept;

      bool sendUDPPacket(
                         SocketPtr socket,
                         const IPAddress &boundIP,
                         const IPAddress &remoteIP,
                         const BYTE *buffer,
                         size_t bufferSizeInBytes
                         ) noexcept;

      bool shouldKeepWarm() const noexcept;
      bool shouldWarmUpAfterInterfaceBinding() const noexcept;

      void fixSTUNParserOptions(const STUNPacketPtr &packet) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // ICETransport => (data)
      //

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

      RangeSelection mPortRestriction;
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
    //
    // IICEGathererFactory
    //

    interaction IICEGathererFactory
    {
      static IICEGathererFactory &singleton() noexcept;

      virtual ICEGathererPtr create(
                                    IICEGathererDelegatePtr delegate,
                                    const IICEGathererTypes::Options &options
                                    ) noexcept;
    };

    class ICEGathererFactory : public IFactory<IICEGathererFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IGathererAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IGathererAsyncDelegate::UseICETransportPtr, UseICETransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD(onNotifyDeliverRouteBufferedPackets, UseICETransportPtr, PUID)
ZS_DECLARE_PROXY_METHOD(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_END()
