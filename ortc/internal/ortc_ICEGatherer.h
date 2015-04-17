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

#include <openpeer/services/IBackOffTimer.h>
#include <openpeer/services/IDNS.h>
#include <openpeer/services/IWakeDelegate.h>
#include <openpeer/services/ISTUNDiscovery.h>
#include <openpeer/services/ITURNSocket.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Socket.h>
#include <zsLib/Timer.h>

#include <cryptopp/queue.h>

#define ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING  "ortc/gatherer-interface-name-mapping"
#define ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH  "ortc/gatherer-username-frag-length"
#define ORTC_SETTING_GATHERER_PASSWORD_LENGTH  "ortc/gatherer-password-length"

#define ORTC_SETTING_GATHERER_CANDIDATE_TYPE_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer-canadidate-type-priority-"  // (0..126) << (24)
#define ORTC_SETTING_GATHERER_PROTOCOL_TYPE_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer-protocol-type-priority-"     // (0..0x3) << (24-2)
#define ORTC_SETTING_GATHERER_INTERFACE_TYPE_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer-interface-type-priority-"   // (0..0xF) << (24-6)
#define ORTC_SETTING_GATHERER_ADDRESS_FAMILY_PREFERENCE_PRIORITY_PREFIX "ortc/gatherer-address-family-priority-"   // (0..0x3) << (24-8)

#define ORTC_SETTING_GATHERER_CANDIDATE_TYPE_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer-canadidate-type-unfreeze-priority-"  // (0..126) << (24+6)
#define ORTC_SETTING_GATHERER_PROTOCOL_TYPE_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer-protocol-type-unfreeze-priority-"     // (0..0x3) << (24+4)
#define ORTC_SETTING_GATHERER_INTERFACE_TYPE_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer-interface-type-unfreeze-priority-"   // (0..0xF) << (24-4)
#define ORTC_SETTING_GATHERER_ADDRESS_FAMILY_PREFERENCE_UNFREEZE_PREFIX "ortc/gatherer-address-family-unfreeze-priority-"   // (0..0x3) << (24-8)

#define ORTC_SETTING_GATHERER_CREATE_TCP_CANDIDATES  "ortc/gatherer-create-tcp-candidates"
#define ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER  "ortc/gatherer-bind-back-off-timer"

#define ORTC_SETTING_GATHERER_DEFAULT_CANDIDATES_WARM_UNTIL_IN_SECONDS "ortc/gatherer-default-candidates-warm-until-in-seconds"
#define ORTC_SETTING_GATHERER_DEFAULT_STUN_KEEP_ALIVE_IN_SECONDS "ortc/gatherer-default-stun-keep-alive-in-seconds"

#define ORTC_SETTING_GATHERER_RELAY_INACTIVITY_TIMEOUT_IN_SECONDS "ortc/gatherer-relay-inactivity-timeout-in-seconds"

#define ORTC_SETTING_GATHERER_MAX_INCOMING_PACKET_BUFFERING_TIME_IN_SECONDS "ortc/gatherer-max-incoming-packet-buffering-time-in-seconds"

#define ORTC_SETTING_GATHERER_MAX_PENDING_OUTGOING_TCP_SOCKET_BUFFERING_IN_BYTES "ortc/gatherer-max-pending-outgoing-tcp-socket-buffering-in-bytes"
#define ORTC_SETTING_GATHERER_MAX_CONNECTED_TCP_SOCKET_BUFFERING_IN_BYTES "ortc/gatherer-max-connected-tcp-socket-buffering-in-bytes"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICEGatherer)
    ZS_DECLARE_INTERACTION_PROXY(IGathererAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererForSettings
    #pragma mark

    interaction IICEGathererForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IICEGathererForSettings() {}
    };

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

      virtual PUID getID() const = 0;

      virtual void installTransport(
                                    ICETransportPtr transport,
                                    const String &remoteUFrag
                                    ) = 0;
      virtual void removeTransport(ICETransport &transport) = 0;

      virtual PUID createRoute(
                               ICETransportPtr transport,
                               IICETypes::CandidatePtr sentFromLocalCanddiate,
                               const IPAddress &remoteIP
                               ) = 0;

      virtual void removeRoute(PUID routeID) = 0;

      virtual bool sendPacket(
                              PUID routeID,
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) = 0;
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
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseTransport)

      virtual void onNotifyDeliverRouteBufferedPackets(
                                                       UseTransportPtr transport,
                                                       PUID routeID
                                                       )  = 0;
      virtual void onNotifyAboutRemoveRoute(
                                            UseTransportPtr transport,
                                            PUID routeID
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
                        public IICEGathererForSettings,
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
    public:
      friend interaction IICEGatherer;
      friend interaction IICEGathererFactory;
      friend interaction IICEGathererForSettings;
      friend interaction IICEGathererForICETransport;

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

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, UseTransport)
      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, UseICETransport)
      ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISTUNDiscovery, UseSTUNDiscovery)
      ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ITURNSocket, UseTURNSocket)
      ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IBackOffTimer, UseBackOffTimer)
      ZS_DECLARE_TYPEDEF_PTR(zsLib::Socket, Socket)

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
      typedef std::list<RelayPortPtr> RelayPortList;
      typedef std::map<IPAddress, RelayPortPtr> IPToRelayPortMap;
      typedef std::map<TimerPtr, RelayPortPtr> TimerToRelayPortMap;

      typedef std::map<SocketPtr, TCPPortPtr> SocketToTCPPortMap;
      typedef std::map<CandidatePtr, TCPPortPtr> CandidateToTCPPortMap;

      typedef std::list<BufferedPacketPtr> BufferedPacketList;

      typedef std::pair<CandidatePtr, IPAddress> LocalCandidateRemoteIPPair;
      typedef std::map<LocalCandidateRemoteIPPair, RoutePtr> CandidateAndRemoteIPToRouteMap;

      typedef String UsernameFragment;
      typedef PUID TransportID;
      typedef std::map<UsernameFragment, InstalledTransportPtr> TransportMap;

      typedef PUID RouteID;
      typedef std::map<RouteID, RoutePtr> RouteMap;

      typedef CryptoPP::ByteQueue ByteQueue;

    protected:
      ICEGatherer(
                  IMessageQueuePtr queue,
                  IICEGathererDelegatePtr delegate,
                  const Options &options
                  );

      ICEGatherer(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create()) {}

      void init();

    public:
      virtual ~ICEGatherer();

      static ICEGathererPtr convert(IICEGathererPtr object);
      static ICEGathererPtr convert(ForSettingsPtr object);
      static ICEGathererPtr convert(ForICETransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IICETransport
      #pragma mark

      static ElementPtr toDebug(ICEGathererPtr gatherer);

      static ICEGathererPtr create(
                                   IICEGathererDelegatePtr delegate,
                                   const Options &options
                                   );

      virtual PUID getID() const;

      virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate);

      virtual States state() const;

      virtual ParametersPtr getLocalParameters() const;
      virtual CandidateListPtr getLocalCandidates() const;

      virtual void gather(const Options &options);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IICEGathererForICETransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual void installTransport(
                                    ICETransportPtr transport,
                                    const String &remoteUFrag
                                    );
      virtual void removeTransport(ICETransport &transport);

      virtual PUID createRoute(
                               ICETransportPtr transport,
                               IICETypes::CandidatePtr sentFromLocalCanddiate,
                               const IPAddress &remoteIP
                               );

      virtual void removeRoute(PUID routeID);

      virtual bool sendPacket(
                              PUID routeID,
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IGathererAsyncDelegate
      #pragma mark

      virtual void onNotifyDeliverRouteBufferedPackets(
                                                       UseTransportPtr transport,
                                                       PUID routeID
                                                       );

      virtual void onNotifyAboutRemoveRoute(
                                            UseTransportPtr transport,
                                            PUID routeID
                                            );

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IWakeDelegate
      #pragma mark

      virtual void onWake();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IDNSDelegate
      #pragma mark

      virtual void onLookupCompleted(IDNSQueryPtr query);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ISocketDelegate
      #pragma mark

      virtual void onReadReady(SocketPtr socket);
      virtual void onWriteReady(SocketPtr socket);
      virtual void onException(SocketPtr socket);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => IBackOffDelegate
      #pragma mark

      virtual void onBackOffTimerAttemptAgainNow(IBackOffTimerPtr timer);
      virtual void onBackOffTimerAttemptTimeout(IBackOffTimerPtr timer);
      virtual void onBackOffTimerAllAttemptsFailed(IBackOffTimerPtr timer);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ISTUNDiscoveryDelegate
      #pragma mark

      virtual void onSTUNDiscoverySendPacket(
                                             ISTUNDiscoveryPtr discovery,
                                             IPAddress destination,
                                             SecureByteBlockPtr packet
                                             );

      virtual void onSTUNDiscoveryCompleted(ISTUNDiscoveryPtr discovery);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => ITURNSocketDelegate
      #pragma mark

      virtual void onTURNSocketStateChanged(
                                            ITURNSocketPtr socket,
                                            TURNSocketStates state
                                            );

      virtual void handleTURNSocketReceivedPacket(
                                                  ITURNSocketPtr socket,
                                                  IPAddress source,
                                                  const BYTE *packet,
                                                  size_t packetLengthInBytes
                                                  );

      virtual bool notifyTURNSocketSendPacket(
                                              ITURNSocketPtr socket,
                                              IPAddress destination,
                                              const BYTE *packet,
                                              size_t packetLengthInBytes
                                              );

      virtual void onTURNSocketWriteReady(ITURNSocketPtr socket);


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

          ElementPtr createElement() const;
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
        Server mServer;
        UseSTUNDiscoveryPtr mSTUNDiscovery;

        String mOptionsHash;
        CandidatePtr mCandidate;

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
        Server mServer;
        UseTURNSocketPtr mTURNSocket;
        IPAddress mServerResponseIP;

        String mOptionsHash;
        CandidatePtr mRelayCandidate;
        CandidatePtr mReflexiveCandidate;

        Time mLastSentData;
        TimerPtr mInactivityTimer;

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
        HostIPSorter::DataPtr mHostData;

        String mBoundOptionsHash;
        CandidatePtr mCandidateUDP;
        IPAddress mBoundUDPIP;
        SocketPtr mBoundUDPSocket;

        CandidatePtr mCandidateTCP;
        IPAddress mBoundTCPIP;
        SocketPtr mBoundTCPSocket;

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
        bool mConnected {false};
        HostPortPtr mHostPort;

        CandidatePtr mCandidate;

        IPAddress mRemoteIP;
        SocketPtr mSocket;
        ByteQueue mIncomingBuffer;
        ByteQueue mOutgoingBuffer;

        TransportID mTransportID {0};
        UseTransportWeakPtr mTransport;

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
        CandidatePtr mLocalCandidate;
        IPAddress mFrom;

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

        Time mLastUsed;
        CandidatePtr mLocalCandidate;
        IPAddress mFrom;

        TransportID mTransportID {};
        UseTransportWeakPtr mTransport;

        HostPortPtr mHostPort;    // send via host UDP packet
        RelayPortPtr mRelayPort;  // send via relay port
        TCPPortPtr mTCPPort;      // send via TCP socket

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
        UseTransportWeakPtr mTransport;

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
        PreferenceTypes mType;
        DWORD *mCandidateTypePreferences {NULL};
        DWORD *mProtocolTypePreferences {NULL};
        DWORD *mInterfaceTypePreferences {NULL};
        DWORD *mAddressFamilyPreferences {NULL};

        Preference(PreferenceTypes type);
        ~Preference();

        void getSettingsPrefixes(
                                 const char * &outCandidateType,
                                 const char * &outProtocolType,
                                 const char * &outInterfaceType,
                                 const char * &outAddressFamily
                                 );
        void load();
        void save();

        ElementPtr toDebug();
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

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepCalculateOptionsHash();
      bool stepResolveHostIPs();
      bool stepGetHostIPs();
      bool stepCalculateHostsHash();
      bool stepFixHostPorts();
      bool stepBindHostPorts();
      bool stepWarmth();
      bool stepSetupReflexive();
      bool stepTearDownReflexive();
      bool stepSetupRelay();
      bool stepTearDownRelay();
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
      String createDNSLookupString(
                                   const Server &server,
                                   const char *urlPrefix
                                   ) const;

      bool needsHostPort(HostIPSorter::DataPtr hostData);

      void shutdown(HostPort &hostPort);
      void shutdown(ReflexivePort &reflexivePort);
      void shutdown(
                    RelayPort &relayPort,
                    HostPort &ownerHostPort
                    );
      void shutdown(TCPPort &tcpPort);

      SocketPtr bind(
                     IPAddress &ioBindIP,
                     IICETypes::Protocols protocol
                     );

      CandidatePtr createCandidate(
                                   HostIPSorter::DataPtr hostData,
                                   IICETypes::CandidateTypes candidateType,
                                   const IPAddress &boundIP,
                                   IICETypes::Protocols protocol = IICETypes::Protocol_UDP
                                   );
      CandidatePtr createCandidate(
                                   HostIPSorter::DataPtr hostData,
                                   IICETypes::CandidateTypes candidateType,
                                   const IPAddress &baseIP,
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

      void read(
                HostPortPtr hostPort,
                SocketPtr socket
                );
      void read(TCPPort &tcpPort);

      void write(
                 HostPort &hostPort,
                 SocketPtr socket
                 );
      bool writeIfTCPPort(SocketPtr socket);

      void close(
                 HostPort &hostPort,
                 SocketPtr socket
                 );
      void close(TCPPort &tcpPort);

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

      // WARNING: DO NOT CALL FROM WITHIN A LOCK
      RoutePtr installRoute(
                            CandidatePtr localCandidate,
                            const IPAddress &remoteIP,
                            UseTransportPtr transport,
                            bool notifyTransport = true
                            );

      void sendSTUNPacket(
                          CandidatePtr localCandidate,
                          const IPAddress &remoteIP,
                          STUNPacketPtr stunPacket
                          );

      void fix(STUNPacketPtr stunPacket) const;

      void removeAllRelatedRoutes(
                                  TransportID transportID,
                                  UseTransportPtr transportIfAvailable
                                  );

      bool sendUDPPacket(
                         SocketPtr socket,
                         const IPAddress &remoteIP,
                         const BYTE *buffer,
                         size_t bufferSizeInBytes
                         );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (data)
      #pragma mark

      ICEGathererWeakPtr mThisWeak;
      AutoPUID mID;

      ICEGathererPtr mGracefulShutdownReference;

      IICEGathererDelegateSubscriptions mSubscriptions;
      IICEGathererSubscriptionPtr mDefaultSubscription;

      InternalStates mCurrentState {InternalState_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      Components mComponent {Component_RTP};
      String mUsernameFrag;
      String mPassword;

      Options mOptions;

      Preference mPreferences[PreferenceType_Last+1] {PreferenceType_Priority, PreferenceType_Unfreeze};
      HostIPSorter::InterfaceMappingList mInterfaceMappings;
      bool mCreateTCPCandidates {true};

      bool mGetLocalIPsNow {true};
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
      String mLastCandidatesWarmUntilOptionsHash;

      IPToHostPortMap mHostPorts;
      SocketToHostPortMap mHostPortSockets;

      STUNToReflexivePortMap mSTUNDiscoveries;
      TURNToRelayPortMap mTURNSockets;

      String mHasSTUNServersOptionsHash;
      bool mHasSTUNServers {false};

      String mHasTURNServersOptionsHash;
      bool mHasTURNServers {false};

      FoundationToLocalPreferenceMap mLastLocalPreference;

      UseBackOffTimerPtr mBindBackOffTimer;

      CandidateMap mNotifiedCandidates;
      CandidateMap mLocalCandidates;

      Time mWarmUntil;  // keep candidates warm until this time
      TimerPtr mWarmUntilTimer;

      Seconds mRelayInactivityTime;
      TimerToRelayPortMap mRelayInactivityTimers;

      SocketToTCPPortMap mTCPPorts;
      CandidateToTCPPortMap mTCPCandidateToTCPPorts;
      size_t mMaxTCPBufferingSizePendingConnection {};
      size_t mMaxTCPBufferingSizeConnected {};

      TimerPtr mCleanUpBufferingTimer;
      Seconds mMaxBufferingTime;
      BufferedPacketList mBufferedPackets;

      CandidateAndRemoteIPToRouteMap mRouteCandidateAndIPMappings;
      RouteMap mRoutes;

      TransportMap mInstalledTransports;
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
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IGathererAsyncDelegate::UseTransportPtr, UseTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD_2(onNotifyDeliverRouteBufferedPackets, UseTransportPtr, PUID)
ZS_DECLARE_PROXY_METHOD_2(onNotifyAboutRemoveRoute, UseTransportPtr, PUID)
ZS_DECLARE_PROXY_METHOD_1(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_END()
