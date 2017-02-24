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

#include <ortc/internal/ortc_ICEGatherer.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IDNS.h>
#include <ortc/services/IHTTP.h>
#include <ortc/services/ISTUNDiscovery.h>
#include <ortc/services/ISTUNRequester.h>
#include <ortc/services/ITURNSocket.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/ITimer.h>
#include <zsLib/XML.h>

#include <zsLib/SafeInt.h>

#include <regex>

#include <cryptopp/sha.h>


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif //HAVE_SYS_TYPES_H

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif //HAVE_IFADDRS_H

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif //HAVE_NET_IF_H

#ifdef HAVE_NETINIT6_IN6_VAR_H
#include <netinet6/in6_var.h>
#endif //HAVE_NETINIT6_IN6_VAR_H

#ifdef _ANDROID
#include <ortc/internal/ifaddrs-android.h>
#else
#endif

#ifdef HAVE_IPHLPAPI_H
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#endif //HAVE_IPHLPAPI_H

#ifdef WINRT
using namespace Windows::Networking::Connectivity;
#endif //WINRT

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif //__APPLE__

#define ORTC_ICEGATHERER_TO_ORDER(xInterfaceType, xOrder) ((((ULONG)xInterfaceType)*100)+xOrder)

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_icegatherer) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IDNS, UseDNS);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IBackOffTimerPattern, UseBackOffTimerPattern);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::ITURNSocket, ITURNSocket);

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  using zsLib::Numeric;
  using zsLib::Log;

  ZS_DECLARE_USING_PROXY(zsLib, ISocketDelegate)

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(ICEGathererSettingsDefaults);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark helpers
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGathererSettingsDefaults
    #pragma mark

    class ICEGathererSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~ICEGathererSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static ICEGathererSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<ICEGathererSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static ICEGathererSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<ICEGathererSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ElementPtr rootEl = Element::create("interfaces");

        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "lo[0-9].*"; // loopback
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
#ifdef __APPLE__
#ifdef TARGET_OS_IPHONE
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "en0";       // always wifi on ios
          info.mInterfaceType = ICEGatherer::InterfaceType_WLAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "en1";       // always WLAN on ios
          info.mInterfaceType = ICEGatherer::InterfaceType_WWAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
#elif TARGET_OS_MAX
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "en0";       // typically local ethernet
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 10);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "en1";       // typically wifi
          info.mInterfaceType = ICEGatherer::InterfaceType_WLAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
#endif //TARGET_OS_IPHONE
#else //__APPLE__
#endif //__APPLE__
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "en[0-9].*";   // ethernet
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 99);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "bbptp[0-9].*";
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 50);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "p2p[0-9].*";    // point to point
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 55);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "awdl[0-9].*";   // apple wireless direct link
          info.mInterfaceType = ICEGatherer::InterfaceType_WLAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 50);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "pdp_ip[0-9].*"; // GSM style data service
          info.mInterfaceType = ICEGatherer::InterfaceType_WWAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 50);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "vmnet[0-9].*";  // virtual interface
          info.mInterfaceType = ICEGatherer::InterfaceType_VPN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "bridge[0-9].*"; // bridged network
          info.mInterfaceType = ICEGatherer::InterfaceType_VPN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 10);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "stf[0-9].*";  // 6 to 4 IP
          info.mInterfaceType = ICEGatherer::InterfaceType_Tunnel;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "tsp[0-9].*";  // 6 to 4 IP tunnel
          info.mInterfaceType = ICEGatherer::InterfaceType_Tunnel;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 1);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)isatap";  // 6 to 4 IP tunnel
          info.mInterfaceType = ICEGatherer::InterfaceType_Tunnel;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 2);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "gif[0-9].*";   // generic tunnel interface
          info.mInterfaceType = ICEGatherer::InterfaceType_Tunnel;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 10);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "fw[0-9].*";   // firewire
          info.mInterfaceType = ICEGatherer::InterfaceType_Unknown;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 20);
          rootEl->adoptAsLastChild(info.createElement());
        }

        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)gigabit";     // ethernet
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 15);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)wwan";    // wireless wan
          info.mInterfaceType = ICEGatherer::InterfaceType_WWAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 5);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)wireless";    // wireless lan
          info.mInterfaceType = ICEGatherer::InterfaceType_WLAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 5);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)bluetooth";   // bluetooh network
          info.mInterfaceType = ICEGatherer::InterfaceType_WLAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 15);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)local area connection";   // lan
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 25);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)vmware";   // treat like vpn
          info.mInterfaceType = ICEGatherer::InterfaceType_VPN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 5);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)ethernet";   // treat like lan
          info.mInterfaceType = ICEGatherer::InterfaceType_LAN;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 23);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = "(?i)tunnel";   // tunneled network adapter
          info.mInterfaceType = ICEGatherer::InterfaceType_Tunnel;
          info.mOrderIndex = ORTC_ICEGATHERER_TO_ORDER(info.mInterfaceType, 15);
          rootEl->adoptAsLastChild(info.createElement());
        }
        {
          ICEGatherer::HostIPSorter::InterfaceNameMappingInfo info;
          info.mInterfaceNameRegularExpression = ".*";   // catch-all
          info.mInterfaceType = ICEGatherer::InterfaceType_Unknown;
          info.mOrderIndex = 9999;
          rootEl->adoptAsLastChild(info.createElement());
        }

        String mapping = IHelper::toString(rootEl);

        ISettings::setString(ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING, mapping);
        ISettings::setUInt(ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH, 80/5); // must be at least 80 bits
        ISettings::setUInt(ORTC_SETTING_GATHERER_PASSWORD_LENGTH, 128/5);     // must be at least 128 bits

        ISettings::setBool(ORTC_SETTING_GATHERER_CREATE_TCP_CANDIDATES, true);

        {
          UseBackOffTimerPatternPtr pattern = UseBackOffTimerPattern::create();
          pattern->addNextRetryAfterFailureDuration(Seconds(1));
          pattern->addNextRetryAfterFailureDuration(Seconds(1));
          pattern->setMultiplierForLastRetryAfterFailureDuration(2.0);
          pattern->setMaxRetryAfterFailureDuration(Seconds(120));
          pattern->setMaxAttempts(6);

          ISettings::setString(ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER, pattern->save());
        }

        ISettings::setUInt(ORTC_SETTING_GATHERER_WARM_UP_TIME_AFTER_NEW_INTERFACE_IN_SECONDS, 30);

        {
          ICEGatherer::Preference pref(ICEGatherer::PreferenceType_Priority);
          pref.save();
        }
        {
          ICEGatherer::Preference pref(ICEGatherer::PreferenceType_Unfreeze);
          pref.save();
        }

        ISettings::setUInt(ORTC_SETTING_GATHERER_DEFAULT_STUN_KEEP_ALIVE_IN_SECONDS, 30);

        ISettings::setUInt(ORTC_SETTING_GATHERER_REFLEXIVE_INACTIVITY_TIMEOUT_IN_SECONDS, 60*2);
        ISettings::setUInt(ORTC_SETTING_GATHERER_RELAY_INACTIVITY_TIMEOUT_IN_SECONDS, 60*2);
        ISettings::setUInt(ORTC_SETTING_GATHERER_MAX_INCOMING_PACKET_BUFFERING_TIME_IN_SECONDS, 30);
        ISettings::setUInt(ORTC_SETTING_GATHERER_MAX_TOTAL_INCOMING_PACKET_BUFFERING, 50);

        ISettings::setUInt(ORTC_SETTING_GATHERER_MAX_PENDING_OUTGOING_TCP_SOCKET_BUFFERING_IN_BYTES, 100*1024); // max 100K
        ISettings::setUInt(ORTC_SETTING_GATHERER_MAX_CONNECTED_TCP_SOCKET_BUFFERING_IN_BYTES, 10*1024);  // max 10K

        ISettings::setUInt(ORTC_SETTING_GATHERER_CLEAN_UNUSED_ROUTES_NOT_USED_IN_SECONDS, 90);

        ISettings::setBool(ORTC_SETTING_GATHERER_GATHER_PASSIVE_TCP_CANDIDATES, true);

        ISettings::setUInt(ORTC_SETTING_GATHERER_RECHECK_IP_ADDRESSES_IN_SECONDS, 60);
      }
      
    };

    //-------------------------------------------------------------------------
    void installICEGathererSettingsDefaults()
    {
      ICEGathererSettingsDefaults::singleton();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IICEGathererForICETransport::toDebug(ForICETransportPtr gatherer)
    {
      if (!gatherer) return ElementPtr();
      return ICEGatherer::convert(gatherer)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer
    #pragma mark

    //-------------------------------------------------------------------------
    const char *ICEGatherer::toString(InternalStates state)
    {
      switch (state) {
        case InternalState_Pending:       return "Pending";
        case InternalState_Gathering:     return "Gathering";
        case InternalState_Ready:         return "Ready";
        case InternalState_ShuttingDown:  return "Shutting down";
        case InternalState_Shutdown:      return "Shutdown";
      }
      return "Unknown";
    }

    //-------------------------------------------------------------------------
    IICEGatherer::States ICEGatherer::toState(InternalStates state)
    {
      switch (state) {
        case InternalState_Pending:       return IICEGathererTypes::State_New;
        case InternalState_Gathering:     return IICEGathererTypes::State_Gathering;
        case InternalState_Ready:         return IICEGathererTypes::State_Complete;
        case InternalState_ShuttingDown:  return IICEGathererTypes::State_Complete;
        case InternalState_Shutdown:      return IICEGathererTypes::State_Closed;
      }
      return IICEGathererTypes::State_Complete;
    }

    //-------------------------------------------------------------------------
    const char *ICEGatherer::toString(InterfaceTypes type)
    {
      switch (type) {
        case InterfaceType_Unknown:   return "unknown";
        case InterfaceType_Default:   return "default";
        case InterfaceType_LAN:       return "lan";
        case InterfaceType_Tunnel:    return "tunnel";
        case InterfaceType_WLAN:      return "wlan";
        case InterfaceType_WWAN:      return "wwan";
        case InterfaceType_VPN:       return "vpn";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    ICEGatherer::InterfaceTypes ICEGatherer::toInterfaceType(const char *type)
    {
      static InterfaceTypes types[] = {
        InterfaceType_Default,
        InterfaceType_LAN,
        InterfaceType_WLAN,
        InterfaceType_WWAN,
        InterfaceType_VPN,
        InterfaceType_Unknown,
      };

      String typeStr(type);
      if (typeStr.isEmpty()) return InterfaceType_Default;

      for (size_t loop = 0; InterfaceType_Unknown != types[loop]; ++loop) {
        if (0 == typeStr.compareNoCase(toString(types[loop]))) {
          return types[loop];
        }
      }
      return InterfaceType_Unknown;
    }

    //-------------------------------------------------------------------------
    const char *ICEGatherer::toString(AddressFamilies family)
    {
      switch (family) {
        case AddressFamily_IPv4:   return "ipv4";
        case AddressFamily_IPv6:   return "ipv6";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    ICEGatherer::AddressFamilies ICEGatherer::toAddressFamily(const char *family)
    {
      static AddressFamilies families[] = {
        AddressFamily_IPv4,
        AddressFamily_IPv6,
      };

      String familyStr(family);
      if (familyStr.isEmpty()) return AddressFamily_IPv4;

      for (size_t loop = 0; loop < (sizeof(families) / sizeof(families[0])); ++loop) {
        if (0 == familyStr.compareNoCase(toString(families[loop]))) {
          return families[loop];
        }
      }
      return AddressFamily_IPv4;
    }

    //-------------------------------------------------------------------------
    const char *ICEGatherer::toString(PreferenceTypes preference)
    {
      switch (preference) {
        case AddressFamily_IPv4:   return "ipv4";
        case AddressFamily_IPv6:   return "ipv6";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    ICEGatherer::PreferenceTypes ICEGatherer::toPreferenceType(const char *preference)
    {
      static PreferenceTypes preferences[] = {
        PreferenceType_Priority,
        PreferenceType_Unfreeze,
      };

      String preferenceStr(preference);
      if (preferenceStr.isEmpty()) return PreferenceType_Priority;

      for (size_t loop = 0; loop <= PreferenceType_Last; ++loop) {
        if (0 == preferenceStr.compareNoCase(toString(preferences[loop]))) {
          return preferences[loop];
        }
      }
      return PreferenceType_Priority;
    }

    //-------------------------------------------------------------------------
    ICEGatherer::ICEGatherer(
                             const make_private &,
                             const ConstructorOptions &options
                             ) :
      SharedRecursiveLock(SharedRecursiveLock::create()),
      MessageQueueAssociator(options.mQueue),
      mGathererRouter(ICEGathererRouter::create()),
      mUsernameFrag(options.mUsernameFragment.hasData() ? options.mUsernameFragment : IHelper::randomString(ISettings::getUInt(ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH))),
      mPassword(options.mPassword.hasData() ? options.mPassword : IHelper::randomString(ISettings::getUInt(ORTC_SETTING_GATHERER_PASSWORD_LENGTH))),
      mCreateTCPCandidates(ISettings::getBool(ORTC_SETTING_GATHERER_CREATE_TCP_CANDIDATES)),
      mOptions(options.mOptions),
      mComponent(options.mComponent),
      mRTPGatherer(options.mRTPGatherer),
      mReflexiveInactivityTime(Seconds(ISettings::getUInt(ORTC_SETTING_GATHERER_REFLEXIVE_INACTIVITY_TIMEOUT_IN_SECONDS))),
      mRelayInactivityTime(Seconds(ISettings::getUInt(ORTC_SETTING_GATHERER_RELAY_INACTIVITY_TIMEOUT_IN_SECONDS))),
      mMaxBufferingTime(Seconds(ISettings::getUInt(ORTC_SETTING_GATHERER_MAX_INCOMING_PACKET_BUFFERING_TIME_IN_SECONDS))),
      mMaxTotalBuffers(ISettings::getUInt(ORTC_SETTING_GATHERER_MAX_TOTAL_INCOMING_PACKET_BUFFERING)),
      mMaxTCPBufferingSizePendingConnection(ISettings::getUInt(ORTC_SETTING_GATHERER_MAX_PENDING_OUTGOING_TCP_SOCKET_BUFFERING_IN_BYTES)),
      mMaxTCPBufferingSizeConnected(ISettings::getUInt(ORTC_SETTING_GATHERER_MAX_CONNECTED_TCP_SOCKET_BUFFERING_IN_BYTES)),
      mGatherPassiveTCP(ISettings::getBool(ORTC_SETTING_GATHERER_GATHER_PASSIVE_TCP_CANDIDATES))
    {
      mSTUNPacketParseOptions = STUNPacket::ParseOptions(STUNPacket::RFC_AllowAll, false, "ortc::ICEGatherer", mID);

      auto recheckIPsInSeconds = ISettings::getUInt(ORTC_SETTING_GATHERER_RECHECK_IP_ADDRESSES_IN_SECONDS);

      if (0 != recheckIPsInSeconds) {
        mRecheckIPsDuration = Seconds(recheckIPsInSeconds);
      }
      
      ZS_EVENTING_16(
                     x, i, Detail, IceGathererCreate, ol, IceGatherer, Start,
                     puid, id, mID,
                     puid, gatherRouterId, mGathererRouter->getID(),
                     string, username, mUsernameFrag,
                     string, password, mPassword,
                     bool, createTcpCandidates, mCreateTCPCandidates,
                     bool, continuousGathering, mOptions.mContinuousGathering,
                     size_t, totalInterfacesPolicies, mOptions.mInterfacePolicies.size(),
                     size_t, totalIceServers, mOptions.mICEServers.size(),
                     duration, reflexiveInactivityTime, mReflexiveInactivityTime.count(),
                     duration, relayInactivityTime, mRelayInactivityTime.count(),
                     duration, maxBufferingTime, mMaxBufferingTime.count(),
                     duration, recheckIpsDuration, mRecheckIPsDuration.count(),
                     size_t, maxTotalBuffers, mMaxTotalBuffers,
                     size_t, maxTcpBufferingSizePendingConnection, mMaxTCPBufferingSizePendingConnection,
                     size_t, maxTcpBufferingSizeConnected, mMaxTCPBufferingSizeConnected,
                     bool, gatherPassiveTcp, mGatherPassiveTCP
                     );

      mPreferences[PreferenceType_Priority].mType = PreferenceType_Priority;
      mPreferences[PreferenceType_Priority].mOuterObjectID = mID;
      mPreferences[PreferenceType_Unfreeze].mType = PreferenceType_Unfreeze;
      mPreferences[PreferenceType_Unfreeze].mOuterObjectID = mID;
      mPreferences[PreferenceType_Priority].load();
      mPreferences[PreferenceType_Unfreeze].load();

      ZS_LOG_DETAIL(log("use candidate priority") + mPreferences[PreferenceType_Priority].toDebug())
      ZS_LOG_DETAIL(log("unfreeze candidate priority") + mPreferences[PreferenceType_Unfreeze].toDebug())

      String networkOrder = ISettings::getString(ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING);
      if (networkOrder.hasData()) {

        ElementPtr rootEl = IHelper::toJSON(networkOrder);
        if (rootEl) {
          ElementPtr interfaceEl = rootEl->getFirstChildElement();
          while (interfaceEl) {
            HostIPSorter::InterfaceNameMappingInfo info = HostIPSorter::InterfaceNameMappingInfo::create(interfaceEl);
            if (info.hasData()) {
              ZS_EVENTING_4(
                            x, i, Detail, IceGathererInitializeInstallInterfaceNameMapping, ol, IceGatherer, Initialization,
                            puid, id, mID,
                            string, interfaceNameRegularExpression, info.mInterfaceNameRegularExpression,
                            string, interfaceType, toString(info.mInterfaceType),
                            ulong, orderIndex, info.mOrderIndex
                            );
              ZS_LOG_TRACE(log("installed interface mapping info") + info.toDebug());
              mInterfaceMappings.push_back(info);
            }
            interfaceEl = interfaceEl->getNextSiblingElement();
          }
        }
      }

      if (options.mDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(options.mDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::init()
    {
      ZS_LOG_DETAIL(log("created"))

      AutoRecursiveLock lock(*this);

      auto cleanInSeconds = ISettings::getUInt(ORTC_SETTING_GATHERER_CLEAN_UNUSED_ROUTES_NOT_USED_IN_SECONDS);
      if (0 != cleanInSeconds) {
        mCleanUnusedRoutesDuration = Seconds(cleanInSeconds);
        mCleanUnusedRoutesTimer = ITimer::create(mThisWeak.lock(), mCleanUnusedRoutesDuration);

        ZS_LOG_DETAIL(log("setting up timer to clean unsed routes") + ZS_PARAM("clean duration (s)", mCleanUnusedRoutesDuration) + ZS_PARAM("timer", mCleanUnusedRoutesTimer->getID()))
      }

      // kick start the process
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    ICEGatherer::~ICEGatherer()
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"))
      cancel();
      ZS_EVENTING_1(x, i, Detail, IceGathererDestroy, ol, IceGatherer, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr ICEGatherer::convert(IICEGathererPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICEGatherer, object);
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr ICEGatherer::convert(ForICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICEGatherer, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    ICEGatherer::PromiseWithStatsReportPtr ICEGatherer::getStats(const StatsTypeSet &stats) const
    {
      if ((!stats.hasStatType(IStatsReportTypes::StatsType_ICEGatherer)) &&
          (!stats.hasStatType(IStatsReportTypes::StatsType_Candidate))) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      AutoRecursiveLock lock(*this);
      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Debug, log("cannot collect stats while shutdown / shutting down"));
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      IGathererAsyncDelegateProxy::create(mThisWeak.lock())->onResolveStatsPromise(promise);
      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::toDebug(ICEGathererPtr gatherer)
    {
      if (!gatherer) return ElementPtr();
      return gatherer->toDebug();
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr ICEGatherer::create(
                                       IICEGathererDelegatePtr delegate,
                                       const Options &options
                                       )
    {
      ConstructorOptions creationOptions;
      creationOptions.mQueue = IORTCForInternal::queueORTC();
      creationOptions.mDelegate = delegate;
      creationOptions.mOptions = options;

      ICEGathererPtr pThis(make_shared<ICEGatherer>(make_private {}, creationOptions));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IICEGathererSubscriptionPtr ICEGatherer::subscribe(IICEGathererDelegatePtr originalDelegate)
    {
      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      auto subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      auto delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        auto pThis = mThisWeak.lock();

        auto currentState = toState(mCurrentState);

        if (State_Closed != currentState) {
          if (State_New != currentState) {
            delegate->onICEGathererStateChange(pThis, State_Gathering);
          }
          for (auto iter = mNotifiedCandidates.begin(); iter != mNotifiedCandidates.end(); ++iter) {
            auto candidate = (*iter).second.first;

            ZS_LOG_DEBUG(log("reporting existing candidate") + ZS_PARAM("subscription", subscription->getID()) + candidate->toDebug())

            CandidatePtr sendCandidate(make_shared<Candidate>(*candidate));
            delegate->onICEGathererLocalCandidate(mThisWeak.lock(), sendCandidate);
          }
        }

        if (State_Complete == currentState) {
          CandidateCompletePtr complete(make_shared<CandidateComplete>());
          complete->mComponent = mComponent;
          delegate->onICEGathererLocalCandidateComplete(pThis, complete);
        }

        if ((State_New != currentState) &&
            (State_Gathering != currentState)) {
          // get out of the gathering state
          delegate->onICEGathererStateChange(pThis, toState(mCurrentState));
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICETypes::Components ICEGatherer::component() const
    {
      return mComponent;
    }

    //-------------------------------------------------------------------------
    IICEGatherer::States ICEGatherer::state() const
    {
      AutoRecursiveLock lock(*this);
      auto state = toState(mCurrentState);
      ZS_LOG_TRACE(log("get current state") + ZS_PARAM("internal state", toString(mCurrentState)) + ZS_PARAM("state", IICEGathererTypes::toString(state)))
      return state;
    }

    //-------------------------------------------------------------------------
    IICEGatherer::ParametersPtr ICEGatherer::getLocalParameters() const
    {
      ParametersPtr result(make_shared<Parameters>());
      result->mUseUnfreezePriority = true;
      result->mUsernameFragment = mUsernameFrag;
      result->mPassword = mPassword;
      return result;
    }

    //-------------------------------------------------------------------------
    IICEGathererTypes::CandidateListPtr ICEGatherer::getLocalCandidates() const
    {
      AutoRecursiveLock lock(*this);

      CandidateListPtr candidates(make_shared<CandidateList>());

      for (auto iter = mNotifiedCandidates.begin(); iter != mNotifiedCandidates.end(); ++iter) {
        auto candidate = (*iter).second.first;
        candidates->push_back(*candidate);
      }

      return candidates;
    }

    //-------------------------------------------------------------------------
    IICEGathererPtr ICEGatherer::createAssociatedGatherer(IICEGathererDelegatePtr delegate) throw(InvalidStateError)
    {
      ORTC_THROW_INVALID_STATE_IF(Component_RTCP == mComponent)

      ICEGathererPtr pThis;

      {
        AutoRecursiveLock lock(*this);

        ORTC_THROW_INVALID_STATE_IF(mRTCPGatherer.lock())

        ConstructorOptions creationOptions;
        creationOptions.mQueue = IORTCForInternal::queueORTC();
        creationOptions.mDelegate = delegate;
        creationOptions.mOptions = mOptions;
        creationOptions.mUsernameFragment = mUsernameFrag;
        creationOptions.mPassword = mPassword;
        creationOptions.mComponent = Component_RTCP;
        creationOptions.mRTPGatherer = mThisWeak.lock();

        pThis = make_shared<ICEGatherer>(make_private {}, creationOptions);
        pThis->mThisWeak = pThis;
        mRTCPGatherer = pThis;

        ZS_EVENTING_2(x, i, Detail, IceGathererCreatedAssociateGatherer, ol, IceGatherer, CreateAssociated, puid, id, mID, puid, associatedId, pThis->getID());
      }

      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::gather(const Optional<Options> &options)
    {
      AutoRecursiveLock lock(*this);

      if (options.hasValue()) {
        mOptions = options.value();
        mOptionsHash = mOptions.hash();
      }

      ZS_EVENTING_4(
                    x, i, Detail, IceGathererGather, ol, IceGatherer, Gather,
                    puid, id, mID,
                    bool, continuousGathering, mOptions.mContinuousGathering,
                    size_t, totalInterfacePolicies, mOptions.mInterfacePolicies.size(),
                    size_t, totalIceServers, mOptions.mICEServers.size()
                    );

      mGetLocalIPsNow = true; // obtain local IPs again
      mLastBoundHostPortsHostHash.clear();
      mLastReflexiveHostsHash.clear();
      mLastRelayHostsHash.clear();

      switch (mCurrentState) {
        case InternalState_Pending:       {
          setState(InternalState_Gathering);
          break;
        }
        case InternalState_Ready:         {
          ZS_LOG_DETAIL(log("must initiate gathering again"))
          setState(InternalState_Gathering);
          break;
        }
        case InternalState_Gathering:
        case InternalState_ShuttingDown:
        case InternalState_Shutdown:      {
          // nothing to do...
          break;
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::close()
    {
      ZS_LOG_DETAIL(log("close called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IICEGathererForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::installTransport(
                                       ICETransportPtr inTransport,
                                       const String &remoteUFrag
                                       )
    {
      UseICETransportPtr transport = inTransport;
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport);

      ZS_EVENTING_3(
                    x, i, Detail, IceGathererInstallIceTransport, ol, IceGatherer, Info,
                    puid, id, mID,
                    puid, iceTransportId, transport->getID(),
                    string, remoteUFrag, remoteUFrag
                    );

      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("cannot install transport when shutting down / shutdown"))
        return;
      }

      if (mKeepWarmSinceJustCreated) {
        // no longer kept warm after creation since now a transport is being installed
        mKeepWarmSinceJustCreated = false;
        mLastReflexiveHostsHash.clear();
        mLastRelayHostsHash.clear();
      }

      // scope: check to see if a transport already existed with this remote ufrag (and if it does, remove it)
      if (remoteUFrag.hasData()) {
        auto found = mInstalledTransports.find(remoteUFrag);
        if (found != mInstalledTransports.end()) {
          auto previousInstalledTransport = (*found).second;
          auto previousTransport = previousInstalledTransport->mTransport.lock();

          if (transport == previousTransport) {
            ZS_LOG_WARNING(Detail, log("already installed transport") + ZS_PARAM("transport", previousInstalledTransport->mTransportID))
            return;
          }

          removeAllRelatedRoutes(previousInstalledTransport->mTransportID, previousTransport);
        }

        for (auto iter_doNotUse = mPendingTransports.begin(); iter_doNotUse != mPendingTransports.end();) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto installedTransport = (*current);
          if (installedTransport->mTransportID != transport->getID()) continue;

          ZS_LOG_DEBUG(log("removing pending transport (since now fully installed)") + installedTransport->toDebug())

          mPendingTransports.erase(current);
          break;
        }
      }

      InstalledTransportPtr installedTransport(make_shared<InstalledTransport>());
      installedTransport->mTransportID = transport->getID();
      installedTransport->mTransport = transport;

      if (remoteUFrag.isEmpty()) {
        ZS_LOG_DEBUG(log("transport is added in a pending state") + installedTransport->toDebug())
        mPendingTransports.push_back(installedTransport);
        goto transport_installed;
      }

      mInstalledTransports[remoteUFrag] = installedTransport;

      // scope: check to see if this remote ufrag will now cause route mappings to occur
      {
        for (auto iter = mBufferedPackets.begin(); iter != mBufferedPackets.end(); ++iter) {
          auto packet = (*iter);
          if (packet->mRFrag != remoteUFrag) continue;

          ZS_LOG_DEBUG(log("found buffered packet matching installed transport (will attempt to install route for buffered packet)") + ZS_PARAM("remoteUFrag", remoteUFrag) + packet->toDebug() + ZS_PARAM("transport", transport->getID()))

          auto installed = installRoute(packet->mRouterRoute, transport);
          if (!installed) {
            ZS_LOG_WARNING(Debug, log("failed to install route for buffered packet") + packet->toDebug() + ZS_PARAM("transport", transport->getID()) + packet->toDebug())
          }
        }
      }

      goto transport_installed;

    transport_installed:
      {
        mTransportsChanged = true;
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::notifyTransportStateChange(ICETransportPtr inTransport)
    {
      UseICETransportPtr transport = inTransport;
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport);

      ZS_EVENTING_2(
                    x, i, Debug, IceGathererInternalIceTransportStateChangedEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    puid, iceTransportId, transport->getID()
                    );

      ZS_LOG_TRACE(log("notified transport state changed") + ZS_PARAM("transport id", transport->getID()))
      AutoRecursiveLock lock(*this);

      mTransportsChanged = true;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::removeTransport(ICETransport &inTransport)
    {
      UseICETransport &transport = inTransport;

      ZS_EVENTING_2(
                    x, i, Debug, IceGathererRemoveIceTransport, ol, IceGatherer, Info,
                    puid, id, mID,
                    puid, iceTransportId, transport.getID()
                    );

      ZS_LOG_TRACE(log("removing transport") + ZS_PARAM("transport id", transport.getID()))

      AutoRecursiveLock lock(*this);
      removeAllRelatedRoutes(transport.getID(), UseICETransportPtr());

      for (auto iter_doNotUse = mInstalledTransports.begin(); iter_doNotUse != mInstalledTransports.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto installedTransport = (*current).second;
        if (installedTransport->mTransportID == transport.getID()) {
          ZS_LOG_DEBUG(log("found installed transport to remove") + installedTransport->toDebug())

          mInstalledTransports.erase(current);
          goto transport_removed;
        }
      }

      for (auto iter_doNotUse = mPendingTransports.begin(); iter_doNotUse != mPendingTransports.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto installedTransport = (*current);
        if (installedTransport->mTransportID == transport.getID()) {
          ZS_LOG_DEBUG(log("found pending transport to remove") + installedTransport->toDebug())

          mPendingTransports.erase(current);
          goto transport_removed;
        }
      }

      goto did_not_remove;

    did_not_remove:
      {
        ZS_LOG_WARNING(Detail, log("did not find installed transport (okay if shutdown)") + ZS_PARAM("transport id", transport.getID()))
        return;
      }

    transport_removed:
      {
        mTransportsChanged = true;
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      }
    }

    //-------------------------------------------------------------------------
    ICEGatherer::ForICETransportPtr ICEGatherer::getRTCPGatherer() const
    {
      AutoRecursiveLock lock(*this);
      return mRTCPGatherer.lock();
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::isContinousGathering() const
    {
      AutoRecursiveLock lock(*this);
      return mOptions.mContinuousGathering;
    }

    //-------------------------------------------------------------------------
    ICEGathererRouterPtr ICEGatherer::getGathererRouter() const
    {
      return mGathererRouter;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::removeRoute(RouterRoutePtr routerRoute)
    {
      AutoRecursiveLock lock(*this);

      auto found = mRoutes.find(routerRoute->mID);
      if (found == mRoutes.end()) {
        ZS_LOG_WARNING(Detail, log("route is not found") + routerRoute->toDebug())
        return;
      }

      auto route = (*found).second;

      route->trace(__func__, "remove");

      ZS_LOG_DEBUG(log("removing route") + route->toDebug())

      mRoutes.erase(found);

      auto foundQuick = mQuickSearchRoutes.find(LocalCandidateRemoteIPPair(route->mLocalCandidate, routerRoute->mRemoteIP));
      ZS_EVENTING_4(
                    x, i, Trace, IceGathererSearchQuickRoute, ol, IceGatherer, Info,
                    puid, id, mID,
                    pointer, localCandidate, route->mLocalCandidate.get(),
                    string, remoteIp, routerRoute->mRemoteIP.string(),
                    bool, found, foundQuick != mQuickSearchRoutes.end()
                    );
      if (foundQuick == mQuickSearchRoutes.end()) {
        ZS_LOG_WARNING(Detail, log("quick route is not found") + route->toDebug())
        return;
      }

      ZS_EVENTING_4(
                    x, i, Debug, IceGathererRemoveQuickRoute, ol, IceGatherer, Info,
                    puid, id, mID,
                    pointer, candidate, (*foundQuick).first.first.get(),
                    string, ipAddress, (*foundQuick).first.second.string(),
                    puid, routeId, route->mID
                    );

      mQuickSearchRoutes.erase(foundQuick);
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::remoteAllRelatedRoutes(ICETransport &inTransport)
    {
      UseICETransport &transport = inTransport;

      auto transportID = transport.getID();

      ZS_EVENTING_2(
                    x, i, Debug, IceGathererRemovetAllIceTransportRelatedRoutes, ol, IceGatherer, Info,
                    puid, id, mID,
                    puid, iceTransportId, transportID
                    );

      ZS_LOG_DEBUG(log("removing all related routes") + ZS_PARAMIZE(transportID))

      AutoRecursiveLock lock(*this);
      removeAllRelatedRoutes(transportID, UseICETransportPtr());
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::sendPacket(
                                 UseICETransport &transport,
                                 RouterRoutePtr routerRoute,
                                 const BYTE *buffer,
                                 size_t bufferSizeInBytes
                                 )
    {
      if (!buffer) return true;
      if (!bufferSizeInBytes) return true;

      ZS_EVENTING_5(
                    x, i, Trace, IceGathererSendIceTransportPacket, ol, IceGatherer, Send,
                    puid, id, mID,
                    puid, iceTransportId, transport.getID(),
                    puid, routerRouteId, routerRoute->mID,
                    buffer, packet, buffer,
                    size, size, bufferSizeInBytes
                    );

      ITURNSocketPtr turn;
      RoutePtr route;

      {
        AutoRecursiveLock lock(*this);

        {
          auto found = mRoutes.find(routerRoute->mID);
          if (found == mRoutes.end()) {
            ZS_LOG_DEBUG(log("no route found at this time (attempt to install a route)") + routerRoute->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes))

            route = installRoute(routerRoute, transport.getForGatherer());
          } else {
            route = (*found).second;
            route->mLastUsed = zsLib::now();
          }
        }

        if (!route) {
          ZS_LOG_WARNING(Debug, log("no route found at this time") + routerRoute->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes))
          goto send_failed;
        }

        route->mLastUsed = zsLib::now();

        if (route->mHostPort) {
          if (!route->mHostPort->mBoundUDPSocket) {
            ZS_LOG_WARNING(Debug, log("no UDP socket found at this time") + route->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes))
            goto send_failed;
          }
          ZS_EVENTING_7(
                        x, i, Trace, IceGathererSendIceTransportPacketViaUdp, ol, IceGatherer, Send,
                        puid, id, mID,
                        puid, iceTransportId, transport.getID(),
                        puid, routerRouteId, routerRoute->mID,
                        puid, hostPortId, route->mHostPort->mID,
                        string, remoteIp, route->mRouterRoute->mRemoteIP.string(),
                        buffer, packet, buffer,
                        size, size, bufferSizeInBytes
                        );
          return sendUDPPacket(route->mHostPort->mBoundUDPSocket, route->mHostPort->mBoundUDPIP, route->mRouterRoute->mRemoteIP, buffer, bufferSizeInBytes);
        }
        if (route->mRelayPort) {
          if (!route->mRelayPort->mTURNSocket) {
            ZS_LOG_WARNING(Debug, log("no turn socket available at this time") + route->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes))
            goto send_failed;
          }
          route->mRelayPort->mLastActivity = route->mLastUsed;
          turn = route->mRelayPort->mTURNSocket;
          goto send_via_turn;
        }
        if (route->mTCPPort) {
          if (!route->mTCPPort->mSocket) {
            ZS_LOG_WARNING(Debug, log("no TCP socket found at this time") + route->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes))
            goto send_failed;
          }

          auto currentSize = route->mTCPPort->mOutgoingBuffer.CurrentSize();
          auto maxSize = (route->mTCPPort->mConnected ? mMaxTCPBufferingSizeConnected : mMaxTCPBufferingSizePendingConnection);

          ZS_LOG_INSANE(log("putting packet into TCP buffer for sending") + route->toDebug() + route->mTCPPort->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes) + ZS_PARAM("current buffer size", currentSize) + ZS_PARAM("max size", maxSize))

          if (currentSize > maxSize) {
            ZS_LOG_WARNING(Trace, log("cannot send packet due to TCP buffer overflow") + route->toDebug() + route->mTCPPort->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes) + ZS_PARAM("current buffer size", currentSize) + ZS_PARAM("max size", maxSize))
            return false;
          }

          ZS_EVENTING_6(
                        x, i, Trace, IceGathererSendIceTransportPacketViaTcp, ol, IceGatherer, Send,
                        puid, id, mID,
                        puid, iceTransportId, transport.getID(),
                        puid, routerRouteId, routerRoute->mID,
                        puid, tcpPortId, route->mTCPPort->mID,
                        buffer, packet, buffer,
                        size, size, bufferSizeInBytes
                        );

          CryptoPP::word16 packeSize {static_cast<CryptoPP::word16>(htons(static_cast<unsigned short>(bufferSizeInBytes)))};

          route->mTCPPort->mOutgoingBuffer.Put((const BYTE *)(&packeSize), sizeof(packeSize));
          route->mTCPPort->mOutgoingBuffer.Put(buffer, bufferSizeInBytes);
          if ((route->mTCPPort->mConnected) &&
              (0 == currentSize) &&
              (route->mTCPPort->mWriteReady)) {
            ZS_LOG_INSANE(log("simulate write ready for TCP socket (to ensure packet is sent out straight away)"))
            ISocketDelegateProxy::create(mThisWeak.lock())->onWriteReady(route->mTCPPort->mSocket);
          }
          return true;
        }

        ZS_LOG_WARNING(Debug, log("route does not have any source / destination ports") + route->toDebug())
      }
      goto send_failed;

    send_via_turn:

      ZS_LOG_INSANE(log("sent packet over TURN"));

      ZS_EVENTING_6(
                    x, i, Trace, IceGathererSendIceTransportPacketViaTurn, ol, IceGatherer, Send,
                    puid, id, mID,
                    puid, iceTransportId, transport.getID(),
                    puid, routerRouteId, routerRoute->mID,
                    puid, turnSocketId, turn->getID(),
                    buffer, packet, buffer,
                    size, size, bufferSizeInBytes
                    );

      if (!turn->sendPacket(route->mRouterRoute->mRemoteIP, buffer, bufferSizeInBytes)) {
        AutoRecursiveLock lock(*this);
        ZS_LOG_WARNING(Debug, log("turn socket not able to send packet at this time") + route->toDebug() + ZS_PARAM("buffer size", bufferSizeInBytes));
        goto send_failed;
      }

      return true;

    send_failed: {}
      ZS_EVENTING_5(
                    x, w, Trace, IceGathererSendIceTransportPacketFailed, ol, IceGatherer, Send,
                    puid, id, mID,
                    puid, iceTransportId, transport.getID(),
                    puid, routerRouteId, routerRoute->mID,
                    buffer, packet, buffer,
                    size, size, bufferSizeInBytes
                    );
      return false;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::notifyLikelyReflexiveActivity(RouterRoutePtr routerRoute)
    {
      AutoRecursiveLock lock(*this);

      auto found = mRoutes.find(routerRoute->mID);
      if (found == mRoutes.end()) {
        ZS_LOG_WARNING(Debug, log("no route found at this time (will not attempt to install a route since one should have been present due to recent previous activity)") + routerRoute->toDebug())
        return;
      }

      auto route = (*found).second;
      auto hostPort = route->mHostPort;

      ZS_EVENTING_3(
                    x, i, Trace, IceGathererInternalLikelyReflexiveActivityEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    puid, routeId, route->mID,
                    puid, routerRouteId, routerRoute->mID
                    );

      if (!hostPort) {
        ZS_LOG_ERROR(Debug, log("there must be a host port on this route") + route->toDebug())
        return;
      }

      bool foundReflexive = false;

      auto now = zsLib::now();

      for (auto iter = hostPort->mReflexivePorts.begin(); iter != hostPort->mReflexivePorts.end(); ++iter) {
        auto reflexivePort = (*iter);

        reflexivePort->mLastActivity = now;
        foundReflexive = true;

        ZS_LOG_INSANE(log("activity found on reflexive port") + reflexivePort->toDebug() + hostPort->toDebug())
      }

      if (foundReflexive) return;

      for (auto iter = hostPort->mRelayPorts.begin(); iter != hostPort->mRelayPorts.end(); ++iter) {
        auto relayPort = (*iter);

        relayPort->mLastActivity = now;
        ZS_LOG_INSANE(log("activity found on relay port") + relayPort->toDebug() + hostPort->toDebug())
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IGathererAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onNotifyDeliverRouteBufferedPackets(
                                                          UseICETransportPtr transport,
                                                          PUID routerRouteID
                                                          )
    {
      BufferedPacketList deliverPackets;
      RoutePtr route;

      // check though buffer to see if there are any other packets fitting this criteria that can now be delivered
      {
        AutoRecursiveLock lock(*this);

        auto found = mRoutes.find(routerRouteID);
        if (found == mRoutes.end()) {
          ZS_LOG_WARNING(Detail, log("route was not found") + ZS_PARAM("router route id", routerRouteID))
          return;
        }

        route = (*found).second;

        for (auto iter_doNotUse = mBufferedPackets.begin(); iter_doNotUse != mBufferedPackets.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto bufferedPacket = (*current);

          if (bufferedPacket->mRouterRoute->mID != routerRouteID) continue;

          deliverPackets.push_back(bufferedPacket);

          mBufferedPackets.erase(current);
        }
      }

      // scope: deliver buffered packets now
      {
        for (auto iter = deliverPackets.begin(); iter != deliverPackets.end(); ++iter) {
          auto bufferedPacket = (*iter);

          if (bufferedPacket->mSTUNPacket) {
            ZS_EVENTING_5(
                          x, i, Trace, IceGathererDeliverIceTransportIncomingStunPacket, ol, IceGatherer, Deliver,
                          puid, id, mID,
                          puid, iceTransportId, transport->getID(),
                          puid, routeId, route->mID,
                          puid, routerRouteId, routerRouteID,
                          bool, wasBuffered, true
                          );

            ZS_LOG_TRACE(log("delivering buffered stun packet") + ZS_PARAM("transport", transport->getID()) + bufferedPacket->mSTUNPacket->toDebug())
            transport->notifyPacket(route->mRouterRoute, bufferedPacket->mSTUNPacket);
            continue;
          }

          ZS_THROW_INVALID_ASSUMPTION_IF(!bufferedPacket->mBuffer);

          ZS_EVENTING_7(
                        x, i, Trace, IceGathererDeliverIceTransportIncomingPacket, ol, IceGatherer, Deliver,
                        puid, id, mID,
                        puid, iceTransportId, transport->getID(),
                        puid, routeId, route->mID,
                        puid, routerRouteId, routerRouteID,
                        bool, wasBuffered, true,
                        buffer, packet, bufferedPacket->mBuffer->BytePtr(),
                        size, size, bufferedPacket->mBuffer->SizeInBytes()
                        );

          ZS_LOG_TRACE(log("delivering buffered packet") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("buffer size", bufferedPacket->mBuffer->SizeInBytes()))
          transport->notifyPacket(route->mRouterRoute, *(bufferedPacket->mBuffer), bufferedPacket->mBuffer->SizeInBytes());
          continue;
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise)
    {
      AutoRecursiveLock lock(*this);

      if (isShutdown()) {
        ZS_LOG_WARNING(Detail, log("requesting stats after shutdown"))
        promise->reject();
        return;
      }

#define TODO_RESOLVE_STATS_PROMISE_WHEN_STATS_ARE_DEFINED 1
#define TODO_RESOLVE_STATS_PROMISE_WHEN_STATS_ARE_DEFINED 2
      promise->reject();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, IceGathererInternalWakeEvent, ol, IceGatherer, InternalEvent, puid, id, mID);
      ZS_LOG_TRACE(log("on wake"))
      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IDNSDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onLookupCompleted(IDNSQueryPtr query)
    {
      ZS_EVENTING_2(x, i, Debug, IceGathererInternalDnsLoookupCompleteEvent, ol, IceGatherer, InternalEvent, puid, id, mID, puid, dnsQueryId, query->getID());

      ZS_LOG_TRACE(log("on lookup complete") + ZS_PARAM("query", query->getID()))
      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onTimer(ITimerPtr timer)
    {
      ZS_EVENTING_3(
                    x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    string, timerType, (const char *)NULL,
                    puid, relatedObjectId, 0
                    );

      Time now = zsLib::now();

      ZS_LOG_TRACE(log("on timer fired") + ZS_PARAM("timer", timer->getID()));
      AutoRecursiveLock lock(*this);

      if (mWarmUpAterNewInterfaceBindingTimer == timer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                      puid, id, mID,
                      string, timerType, "warm up after new interface binding timer",
                      puid, relatedObjectId, 0
                      );

        ZS_LOG_DEBUG(log("no longer need to keep warm after interface binding"));
        mWarmUpAterNewInterfaceBindingTimer->cancel();
        mWarmUpAterNewInterfaceBindingTimer.reset();

        mWarmUpAfterNewInterfaceBindingUntil = Time();

        for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
          auto hostPort = (*iter).second;

          // clear out flag to say this particular interface needs to be warm
          hostPort->mWarmUpAfterBinding = false;
        }
        step();
        return;
      }

      if (mRecheckIPsTimer == timer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                      puid, id, mID,
                      string, timerType, "recheck ips timer",
                      puid, relatedObjectId, 0
                      );

        ZS_LOG_TRACE(log("recheck IPs now timer fired"))
        mGetLocalIPsNow = true;
        step();
        return;
      }

      if (mCleanUpBufferingTimer == timer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                      puid, id, mID,
                      string, timerType, "clean up buffers timer",
                      puid, relatedObjectId, 0
                      );

        ZS_LOG_TRACE(log("cleaning packet buffering"))

        while (mBufferedPackets.size() > 0) {
          auto buffer = mBufferedPackets.front();
          if ((buffer->mTimestamp + mMaxBufferingTime > now) &&
              (mBufferedPackets.size() < mMaxTotalBuffers)) {
            ZS_LOG_TRACE(log("buffering still needed") + buffer->toDebug())
            return;
          }

          if (buffer->mBuffer) {
            ZS_EVENTING_4(
                          x, i, Trace, IceGathererDisposeBufferedIceTransportIncomingPacket, ol, IceGatherer, Dispose,
                          puid, id, mID,
                          puid, routerRouteId, buffer->mRouterRoute->mID,
                          buffer, packet, buffer->mBuffer->BytePtr(),
                          size, size, buffer->mBuffer->SizeInBytes()
                          );
          }
          if (buffer->mSTUNPacket) {
            ZS_EVENTING_2(
                          x, i, Trace, IceGathererDisposeBufferedIceTransportIncomingStunPacket, ol, IceGatherer, Dispose,
                          puid, id, mID,
                          puid, routerRouteId, buffer->mRouterRoute->mID
                          );
            buffer->mSTUNPacket->trace(__func__);
          }

          ZS_LOG_TRACE(log("buffering for too long (or too many buffered packets)") + ZS_PARAM("buffer time", (now - buffer->mTimestamp)) + buffer->toDebug())
          mBufferedPackets.pop_front();
        }

        if (mBufferedPackets.empty()) {
          ZS_LOG_TRACE(log("no longer need buffer cleanup timer as all buffers are gone"))
          mCleanUpBufferingTimer->cancel();
          mCleanUpBufferingTimer.reset();
        }
        return;
      }

      if (mCleanUnusedRoutesTimer == timer) {
        ZS_EVENTING_3(
                      x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                      puid, id, mID,
                      string, timerType, "clean unused routes timer",
                      puid, relatedObjectId, 0
                      );

        ZS_LOG_DEBUG(log("cleaning unused routes"))
        for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end();)
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto route = (*current).second;
          if (route->mLastUsed + mCleanUnusedRoutesDuration < now) {
            ZS_LOG_DEBUG(log("route is no longer in use") + route->toDebug())
            removeRoute(route->mRouterRoute);
          }
        }
        return;
      }

      // scope: check to see if it is an activity timer
      {
        auto found = mReflexiveInactivityTimers.find(timer);
        if (found != mReflexiveInactivityTimers.end()) {
          HostPortPtr hostPort = (*found).second.first;
          ReflexivePortPtr reflexivePort = (*found).second.second;

          ZS_EVENTING_3(
                        x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                        puid, id, mID,
                        string, timerType, "reflexive inactivity timer",
                        puid, relatedObjectId, 0
                        );

          {
            ZS_LOG_TRACE(log("reflexive inactivity timer fired") + hostPort->toDebug() + reflexivePort->toDebug())

            if (reflexivePort->mInactivityTimer) {
              reflexivePort->mInactivityTimer->cancel();
              reflexivePort->mInactivityTimer.reset();
            }

            mReflexiveInactivityTimers.erase(found);

            if (shouldKeepWarm()) {
              ZS_LOG_WARNING(Trace, log("no need to shutdown TURN socket as paths need to be kept warm") + reflexivePort->toDebug());
              return;
            }

            if (Time() == reflexivePort->mLastActivity) goto inactivity_shutdown_reflexive;
            if (reflexivePort->mLastActivity + mReflexiveInactivityTime <= now) goto inactivity_shutdown_reflexive;

            ZS_LOG_TRACE(log("no need to shutdown reflexive port at this time (still active)") + reflexivePort->toDebug() + ZS_PARAM("now", now));

            auto fireAt = reflexivePort->mLastActivity + mReflexiveInactivityTime;
            reflexivePort->mInactivityTimer = ITimer::create(mThisWeak.lock(), fireAt);
            mReflexiveInactivityTimers[reflexivePort->mInactivityTimer] = HostAndReflexivePortPair(hostPort, reflexivePort);
            return;
          }

        inactivity_shutdown_reflexive:
          {
            ZS_LOG_DEBUG(log("need to shutdown reflexive port as it is inactive") + reflexivePort->toDebug())

            shutdown(reflexivePort, hostPort);
            return;
          }
          
        }
      }

      // scope: check to see if it is an activity timer
      {
        auto found = mRelayInactivityTimers.find(timer);
        if (found != mRelayInactivityTimers.end()) {
          HostPortPtr hostPort = (*found).second.first;
          RelayPortPtr relayPort = (*found).second.second;

          ZS_EVENTING_3(
                        x, i, Trace, IceGathererInternalTimerEvent, ol, IceGatherer, InternalEvent,
                        puid, id, mID,
                        string, timerType, "relay inactivity timer",
                        puid, relatedObjectId, relayPort->mID
                        );

          {
            ZS_LOG_TRACE(log("relay inactivity timer fired") + hostPort->toDebug() + relayPort->toDebug())

            if (relayPort->mInactivityTimer) {
              relayPort->mInactivityTimer->cancel();
              relayPort->mInactivityTimer.reset();
            }

            mRelayInactivityTimers.erase(found);

            if (shouldKeepWarm()) {
              ZS_LOG_WARNING(Trace, log("no need to shutdown TURN socket as paths need to be kept warm") + relayPort->toDebug())
              return;
            }

            if (Time() == relayPort->mLastActivity) goto inactivity_shutdown_relay;
            if (relayPort->mLastActivity + mRelayInactivityTime <= now) goto inactivity_shutdown_relay;

            ZS_LOG_TRACE(log("no need to shutdown relay port at this time (still active)") + relayPort->toDebug() + ZS_PARAM("now", now))

            auto fireAt = relayPort->mLastActivity + mRelayInactivityTime;
            relayPort->mInactivityTimer = ITimer::create(mThisWeak.lock(), fireAt);
            mRelayInactivityTimers[relayPort->mInactivityTimer] = HostAndRelayPortPair(hostPort, relayPort);
            return;
          }

        inactivity_shutdown_relay:
          {
            ZS_LOG_DEBUG(log("need to shutdown relay port as it is inactive") + relayPort->toDebug())

            shutdown(relayPort, hostPort);
            return;
          }

        }
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => ISocketDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onReadReady(SocketPtr socket)
    {
      ZS_LOG_INSANE(log("socket read ready") + ZS_PARAM("socket", string(socket)));

      HostPortPtr hostPort;
      TCPPortPtr tcpPort;

      // scope: figure out which host port fired the event
      {
        AutoRecursiveLock lock(*this);

        // first check host ports
        {
          auto found = mHostPortSockets.find(socket);
          if (found != mHostPortSockets.end()) {
            hostPort = (*found).second;
            ZS_EVENTING_2(
                          x, i, Trace, IceGathererInternalHostSocketReadReadyEvent, ol, IceGatherer, InternalEvent,
                          puid, id, mID,
                          puid, hostPortId, hostPort->mID
                          );
            ZS_LOG_INSANE(log("read found host port") + hostPort->toDebug());
            goto found_host_port;
          }
        }

        // next check tcp ports
        {
          auto found = mTCPPorts.find(socket);
          if (found != mTCPPorts.end()) {
            hostPort = (*found).second.first;
            tcpPort = (*found).second.second;
            ZS_EVENTING_2(
                          x, i, Trace, IceGathererInternalTcpSocketReadReadyEvent, ol, IceGatherer, InternalEvent,
                          puid, id, mID,
                          puid, tcpPortId, tcpPort->mID
                          );
            ZS_LOG_INSANE(log("read found tcp port") + tcpPort->toDebug())
            goto found_tcp_socket;
          }
        }
      }

      goto not_found;

    found_host_port:
      {
        // warning: do NOT call from within a lock
        while (read(hostPort, socket)) {}
        return;
      }

    found_tcp_socket:
      {
        read(*hostPort, *tcpPort);
        return;
      }

    not_found:
      {
        ZS_LOG_WARNING(Debug, log("socket read ready failed to find associated port") + ZS_PARAM("socket", string(socket)));
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::onWriteReady(SocketPtr socket)
    {
      ZS_LOG_INSANE(log("socket write ready") + ZS_PARAM("socket", string(socket)));

      HostPortPtr hostPort;
      TCPPortPtr tcpPort;

      // scope: figure out which host port fired the event
      {
        AutoRecursiveLock lock(*this);

        // first check host ports
        {
          auto found = mHostPortSockets.find(socket);
          if (found != mHostPortSockets.end()) {
            hostPort = (*found).second;
            ZS_EVENTING_2(
                          x, i, Trace, IceGathererInternalHostSocketWriteReadyEvent, ol, IceGatherer, InternalEvent,
                          puid, id, mID,
                          puid, hostPortId, hostPort->mID
                          );
            ZS_LOG_INSANE(log("write found host port") + hostPort->toDebug())
            goto found_host_port;
          }
        }

        // next check tcp ports
        {
          if (writeIfTCPPort(socket)) return;
        }
      }

      goto not_found;

    found_host_port:
      {
        // warning: do NOT call from within a lock
        write(*hostPort, socket);
        return;
      }

    not_found:
      {
        ZS_LOG_WARNING(Debug, log("socket write ready failed to find associated port") + ZS_PARAM("socket", string(socket)))
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::onException(SocketPtr socket)
    {
      ZS_LOG_WARNING(Debug, log("socket exception") + ZS_PARAM("socket", string(socket)))

      HostPortPtr hostPort;
      TCPPortPtr tcpPort;

      // scope: figure out which host port fired the event
      {
        AutoRecursiveLock lock(*this);

        // first check host ports
        {
          auto found = mHostPortSockets.find(socket);
          if (found != mHostPortSockets.end()) {
            hostPort = (*found).second;
            ZS_EVENTING_2(
                          x, e, Trace, IceGathererInternalHostSocketExceptionEvent, ol, IceGatherer, InternalEvent,
                          puid, id, mID,
                          puid, hostPortId, hostPort->mID
                          );
            ZS_LOG_INSANE(log("exception found host port") + hostPort->toDebug());
            goto found_host_port;
          }
        }

        // next check tcp ports
        {
          auto found = mTCPPorts.find(socket);
          if (found != mTCPPorts.end()) {
            hostPort = (*found).second.first;
            tcpPort = (*found).second.second;
            ZS_EVENTING_2(
                          x, e, Trace, IceGathererInternalTcpSocketExceptionEvent, ol, IceGatherer, InternalEvent,
                          puid, id, mID,
                          puid, tcpPortId, tcpPort->mID
                          );
            ZS_LOG_INSANE(log("exception found tcp port") + tcpPort->toDebug());
            goto found_tcp_socket;
          }
        }
      }

      goto not_found;

    found_host_port:
      {
        // warning: do NOT call from within a lock
        close(hostPort, socket);
        return;
      }

    found_tcp_socket:
      {
        close(tcpPort, hostPort);
        return;
      }

    not_found:
      {
        ZS_LOG_WARNING(Debug, log("socket exception failed to find associated port") + ZS_PARAM("socket", string(socket)))
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IBackOffDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onBackOffTimerStateChanged(
                                                 IBackOffTimerPtr timer,
                                                 IBackOffTimer::States state
                                                 )
    {
      ZS_EVENTING_3(
                    x, i, Trace, IceGathererInternalBackOffTimerStateChangedEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    puid, backOffTimerId, timer->getID(),
                    string, state, IBackOffTimer::toString(state)
                    );
      AutoRecursiveLock lock(*this);
      ZS_LOG_TRACE(log("back off timer attempt again now fired") + UseBackOffTimer::toDebug(timer));
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => ISTUNDiscoveryDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onSTUNDiscoverySendPacket(
                                                ISTUNDiscoveryPtr discovery,
                                                IPAddress destination,
                                                SecureByteBlockPtr packet
                                                )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!packet);

      ZS_EVENTING_5(
                    x, i, Trace, IceGathererInternalStunDiscoverySendPacket, ol, IceGatherer, Send,
                    puid, id, mID,
                    puid, stunDiscoveryId, discovery->getID(),
                    string, destination, destination.string(),
                    buffer, packet, packet->BytePtr(),
                    size, size, packet->SizeInBytes()
                    );

      ZS_LOG_DEBUG(log("stun discovery needs to send packet") + UseSTUNDiscovery::toDebug(discovery) + ZS_PARAM("destination", destination.string()) + ZS_PARAM("packet length", packet->SizeInBytes()))

      AutoRecursiveLock lock(*this);

      auto found = mSTUNDiscoveries.find(discovery);
      if (found == mSTUNDiscoveries.end()) {
        ZS_LOG_WARNING(Debug, log("notified about obsolute stun discovery") + UseSTUNDiscovery::toDebug(discovery))
        return;
      }

      auto hostPort = (*found).second.first;
      if (!hostPort->mBoundUDPSocket) {
        ZS_LOG_WARNING(Debug, log("failed to send stun packet as bound udp socket is gone") + hostPort->toDebug())
        return;
      }

      auto result = sendUDPPacket(hostPort->mBoundUDPSocket, hostPort->mBoundUDPIP, destination, packet->BytePtr(), packet->SizeInBytes());
      if (!result) {
        ZS_LOG_WARNING(Debug, log("failed to send stun packet on bound socket") + hostPort->toDebug())
        return;
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::onSTUNDiscoveryCompleted(ISTUNDiscoveryPtr discovery)
    {
      AutoRecursiveLock lock(*this);
      ZS_LOG_DEBUG(log("stun discovery complete notification") + UseSTUNDiscovery::toDebug(discovery))

      auto found = mSTUNDiscoveries.find(discovery);
      if (found == mSTUNDiscoveries.end()) {
        ZS_LOG_WARNING(Debug, log("notified about obsolute stun discovery") + UseSTUNDiscovery::toDebug(discovery))
        return;
      }

      auto hostPort = (*found).second.first;

      ZS_EVENTING_4(
                    x, i, Debug, IceGathererInternalStunDiscoveryCompleteEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    puid, stunDiscoveryId, discovery->getID(),
                    puid, hostPortId, hostPort->mID,
                    string, mappedAddress, discovery->getMappedAddress().string()
                    );

      hostPort->mReflexiveOptionsHash.clear();  // force the reflexive ports to recalculate their state
      mLastReflexiveHostsHash.clear();          // force the reflexive ports to be processed again

      step();
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => ITURNSocketDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::onTURNSocketStateChanged(
                                               ITURNSocketPtr socket,
                                               TURNSocketStates state
                                               )
    {
      ZS_EVENTING_3(
                    x, i, Debug, IceGathererInternalTurnSocketStateChangeEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    puid, turnSocketId, socket->getID(),
                    string, state, ITURNSocket::toString(state)
                    );

      AutoRecursiveLock lock(*this);
      ZS_LOG_DEBUG(log("turn state changed notification") + UseTURNSocket::toDebug(socket) + ZS_PARAM("state", UseTURNSocket::toString(state)))

      HostPortPtr hostPort;

      bool closedSocket = false;

      // scope: first check active turn sockets
      {
        auto found = mTURNSockets.find(socket);
        if (found != mTURNSockets.end()) {
          hostPort = (*found).second.first;
        }
      }

      if (!hostPort) {  // check closing turn sockets
        auto found = mShutdownTURNSockets.find(socket);
        if (found == mShutdownTURNSockets.end()) {
          ZS_LOG_WARNING(Debug, log("notified about obsolute turn socket") + UseTURNSocket::toDebug(socket))
          return;
        }
        hostPort = (*found).second.first;
        closedSocket = true;
      }

      if (!closedSocket) {
        hostPort->mRelayOptionsHash.clear();  // force the relay ports to recalculate their state
        mLastRelayHostsHash.clear();          // force the relay ports to be processed again
      }
      
      step();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::handleTURNSocketReceivedPacket(
                                                     ITURNSocketPtr socket,
                                                     IPAddress source,
                                                     const BYTE *packet,
                                                     size_t packetLengthInBytes
                                                     )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!packet);
      ZS_THROW_INVALID_ARGUMENT_IF(0 == packetLengthInBytes);

      ZS_EVENTING_5(
                    x, i, Trace, IceGathererTurnSocketReceivedPacket, ol, IceGatherer, Receive,
                    puid, id, mID,
                    puid, turnSocketId, socket->getID(),
                    string, sourceIp, source.string(),
                    buffer, packet, packet,
                    size, size, packetLengthInBytes
                    );

      STUNPacketPtr stunPacket;
      CandidatePtr localCandidate;

      {
        AutoRecursiveLock lock(*this);
        ZS_LOG_DEBUG(log("turn received incoming packet") + UseTURNSocket::toDebug(socket) + ZS_PARAM("packet length", packetLengthInBytes))

        HostPortPtr hostPort;
        RelayPortPtr relayPort;

        bool closingSocket = false;

        // scope: first check active turn sockets
        {
          auto found = mTURNSockets.find(socket);
          if (found != mTURNSockets.end()) {
            hostPort = (*found).second.first;
            relayPort = (*found).second.second;
          }
        }

        if (!hostPort) {  // check closing turn sockets
          auto found = mShutdownTURNSockets.find(socket);
          if (found == mShutdownTURNSockets.end()) {
            ZS_LOG_WARNING(Debug, log("notified about obsolute turn socket") + UseTURNSocket::toDebug(socket))
            return;
          }
          hostPort = (*found).second.first;
          relayPort = (*found).second.second;
          closingSocket = true;
        }

        localCandidate = relayPort->mRelayCandidate;
        relayPort->mLastActivity = zsLib::now();

        stunPacket = STUNPacket::parseIfSTUN(packet, packetLengthInBytes, mSTUNPacketParseOptions);
        fixSTUNParserOptions(stunPacket);

        if (closingSocket) {
          ZS_LOG_WARNING(Detail, log("turn socket is closing (thus cannot handle incoming packet)") + hostPort->toDebug() + relayPort->toDebug())
          goto unhandled_packet;
        }

        if (!localCandidate) {
          ZS_LOG_WARNING(Detail, log("local relay candidate is gone (thus cannot handle incoming packet)") + hostPort->toDebug() + relayPort->toDebug())
          goto unhandled_packet;
        }

        if (stunPacket) goto found_stun_packet;

        goto found_packet;
      }

    unhandled_packet:
      {
        if (stunPacket) {
          if (ISTUNRequester::handleSTUNPacket(source, stunPacket)) {
            ZS_LOG_TRACE(log("handled by stun requester") + ZS_PARAM("from ip", source.string()) + stunPacket->toDebug())
            return;
          }
        }
        return;
      }

    found_stun_packet:
      {
        if (ISTUNRequester::handleSTUNPacket(source, stunPacket)) {
          ZS_LOG_TRACE(log("handled by stun requester") + ZS_PARAM("from ip", source.string()) + stunPacket->toDebug())
          return;
        }

        auto response = handleIncomingPacket(localCandidate, source, stunPacket);
        if (response) {
          ZS_LOG_TRACE(log("sending response packet") + localCandidate->toDebug() + ZS_PARAM("to", source.string()) + ZS_PARAM("packet length", response->SizeInBytes()))
          socket->sendPacket(source, *response, response->SizeInBytes());
        }
        return;
      }

    found_packet:
      {
        handleIncomingPacket(localCandidate, source, packet, packetLengthInBytes);
      }
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::notifyTURNSocketSendPacket(
                                                 ITURNSocketPtr socket,
                                                 IPAddress destination,
                                                 const BYTE *packet,
                                                 size_t packetLengthInBytes
                                                 )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!packet);
      ZS_THROW_INVALID_ARGUMENT_IF(0 == packetLengthInBytes);

      ZS_EVENTING_5(
                    x, i, Trace, IceGathererTurnSocketSendPacket, ol, IceGatherer, Send,
                    puid, id, mID,
                    puid, turnSocketId, socket->getID(),
                    string, destinationIp, destination.string(),
                    buffer, packet, packet,
                    size, size, packetLengthInBytes
                    );

      ZS_LOG_DEBUG(log("turn socket needs to send packet") + UseTURNSocket::toDebug(socket) + ZS_PARAM("destination", destination.string()) + ZS_PARAM("packet length", packetLengthInBytes));

      AutoRecursiveLock lock(*this);

      HostPortPtr hostPort;

      // scope: first check active turn sockets
      {
        auto found = mTURNSockets.find(socket);
        if (found != mTURNSockets.end()) {
          hostPort = (*found).second.first;
        }
      }

      if (!hostPort) {  // check closing turn sockets
        auto found = mShutdownTURNSockets.find(socket);
        if (found == mShutdownTURNSockets.end()) {
          ZS_LOG_WARNING(Debug, log("notified about obsolute turn socket") + UseTURNSocket::toDebug(socket))
          return false;
        }
        hostPort = (*found).second.first;
      }

      if (!hostPort->mBoundUDPSocket) {
        ZS_LOG_WARNING(Debug, log("failed to send stun packet as bound udp socket is gone") + hostPort->toDebug())
        return false;
      }

      auto result = sendUDPPacket(hostPort->mBoundUDPSocket, hostPort->mBoundUDPIP, destination, packet, packetLengthInBytes);
      if (!result) {
        ZS_LOG_WARNING(Debug, log("failed to send stun packet on bound socket") + hostPort->toDebug())
        return false;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::onTURNSocketWriteReady(ITURNSocketPtr socket)
    {
      // ignored

      // NOTE: if something at a higher level ever needs to know that a socket
      // is available for writting this would need to forward as "route" that
      // is available for writting.
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ICEGatherer::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICEGatherer");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICEGatherer::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::ICEGatherer");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICEGatherer::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::ICEGatherer");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "gatherer router", ICEGathererRouter::toDebug(mGathererRouter));

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      IHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      IHelper::debugAppend(resultEl, "internal state", toString(mCurrentState));
      IHelper::debugAppend(resultEl, "state", IICEGathererTypes::toString(toState(mCurrentState)));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      IHelper::debugAppend(resultEl, "component", mComponent);
      IHelper::debugAppend(resultEl, "username frag", mUsernameFrag);
      IHelper::debugAppend(resultEl, "password", mPassword);

      IHelper::debugAppend(resultEl, "options", mOptions.toDebug());


      IHelper::debugAppend(resultEl, "interface mappings", mInterfaceMappings.size());
      IHelper::debugAppend(resultEl, "create tcp candidates", mCreateTCPCandidates);

      IHelper::debugAppend(resultEl, "get local ips now", mGetLocalIPsNow);
      IHelper::debugAppend(resultEl, "recheck ips duration", mRecheckIPsDuration);
      IHelper::debugAppend(resultEl, "recheck ips timer", mRecheckIPsTimer ? mRecheckIPsTimer->getID() : 0);
      IHelper::debugAppend(resultEl, "pending host ips", mPendingHostIPs.size());
      IHelper::debugAppend(resultEl, "resolved host ips", mResolvedHostIPs.size());
      IHelper::debugAppend(resultEl, "resolve host ip queries", mResolveHostIPQueries.size());

      IHelper::debugAppend(resultEl, "hosts hash", mHostsHash);
      IHelper::debugAppend(resultEl, "options hash", mOptionsHash);

      IHelper::debugAppend(resultEl, "default port", mDefaultPort);

      IHelper::debugAppend(resultEl, "last fixed host ports host hash", mLastFixedHostPortsHostsHash);
      IHelper::debugAppend(resultEl, "last bound host ports host hash", mLastBoundHostPortsHostHash);
      IHelper::debugAppend(resultEl, "last reflexive host hash", mLastReflexiveHostsHash);
      IHelper::debugAppend(resultEl, "last relay host hash", mLastRelayHostsHash);

      IHelper::debugAppend(resultEl, "host ports", mHostPorts.size());
      IHelper::debugAppend(resultEl, "host port sockets", mHostPortSockets.size());

      IHelper::debugAppend(resultEl, "stun discoveries", mSTUNDiscoveries.size());
      IHelper::debugAppend(resultEl, "turn sockets", mTURNSockets.size());
      IHelper::debugAppend(resultEl, "turn sockets shutting down", mShutdownTURNSockets.size());

      IHelper::debugAppend(resultEl, "has stun servers options hash", mHasSTUNServersOptionsHash);
      IHelper::debugAppend(resultEl, "has stun servers", mHasSTUNServers);

      IHelper::debugAppend(resultEl, "has turn servers options hash", mHasTURNServersOptionsHash);
      IHelper::debugAppend(resultEl, "has turn servers", mHasSTUNServers);

      IHelper::debugAppend(resultEl, "last local preference", mLastLocalPreference.size());

      IHelper::debugAppend(resultEl, "notified candidates", mNotifiedCandidates.size());
      IHelper::debugAppend(resultEl, "local candidates", mLocalCandidates.size());

      IHelper::debugAppend(resultEl, "keep warm since just created", mKeepWarmSinceJustCreated);

      IHelper::debugAppend(resultEl, "warm up after new interface binding hosts hash", mWarmUpAfterNewInterfaceBindingHostsHash);
      IHelper::debugAppend(resultEl, "warm up after new interface binding until", mWarmUpAfterNewInterfaceBindingUntil);
      IHelper::debugAppend(resultEl, "warm up after new interface binding timre", mWarmUpAterNewInterfaceBindingTimer ? mWarmUpAterNewInterfaceBindingTimer->getID() : 0);

      IHelper::debugAppend(resultEl, "reflexive inactive time", mReflexiveInactivityTime);
      IHelper::debugAppend(resultEl, "reflexive inactive timers", mReflexiveInactivityTimers.size());

      IHelper::debugAppend(resultEl, "relay inactive time", mRelayInactivityTime);
      IHelper::debugAppend(resultEl, "relay inactive timers", mRelayInactivityTimers.size());

      IHelper::debugAppend(resultEl, "tcp ports", mTCPPorts.size());
      IHelper::debugAppend(resultEl, "tcp candidate to tcp ports", mTCPCandidateToTCPPorts.size());
      IHelper::debugAppend(resultEl, "max tcp buffering size pending connection", mMaxTCPBufferingSizePendingConnection);
      IHelper::debugAppend(resultEl, "max tcp buffering size connected", mMaxTCPBufferingSizeConnected);

      IHelper::debugAppend(resultEl, "clean up buffering timer", mCleanUpBufferingTimer ? mCleanUpBufferingTimer->getID() : 0);
      IHelper::debugAppend(resultEl, "max buffering time", mMaxBufferingTime);
      IHelper::debugAppend(resultEl, "max total buffers", mMaxTotalBuffers);
      IHelper::debugAppend(resultEl, "buffered packets", mBufferedPackets.size());

      IHelper::debugAppend(resultEl, "quick search routes", mQuickSearchRoutes.size());
      IHelper::debugAppend(resultEl, "routes", mRoutes.size());
      IHelper::debugAppend(resultEl, "clean unused routes timer", mCleanUnusedRoutesTimer ? mCleanUnusedRoutesTimer->getID() : 0);
      IHelper::debugAppend(resultEl, "clean unused routes duration", mCleanUnusedRoutesDuration);

      IHelper::debugAppend(resultEl, "installed transports", mInstalledTransports.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::isComplete() const
    {
      AutoRecursiveLock lock(*this);
      return State_Complete == toState(mCurrentState);
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::isShuttingDown() const
    {
      AutoRecursiveLock lock(*this);
      return InternalState_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::isShutdown() const
    {
      AutoRecursiveLock lock(*this);
      return InternalState_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::step()
    {
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_TRACE(log("step - shutting down (thus calling cancel)"))
        cancel();
        return;
      }

      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      ZS_LOG_DEBUG(debug("step"))

      // do not start gathering process until gather is called
      if (InternalState_Pending == mCurrentState) goto done;

      if (!stepRecheckIPTimer()) goto done;
      if (!stepCalculateOptionsHash()) goto done;
      if (!stepResolveHostIPs()) goto done;
      if (!stepGetHostIPs()) goto done;
      if (!stepCalculateHostsHash()) goto done;
      if (!stepFixHostPorts()) goto not_complete;
      if (!stepBindHostPorts()) goto not_complete;
      if (!stepCheckTransportsNeedWarmth()) goto not_complete;
      if (!stepWarmUpAfterInterfaceBinding()) goto not_complete;
      if (!stepSetupReflexive()) goto not_complete;
      if (!stepTearDownReflexive()) goto not_complete;
      if (!stepSetupRelay()) goto not_complete;
      if (!stepTearDownRelay()) goto not_complete;

      if (!stepCheckIfReady()) goto not_complete;

      setState(InternalState_Ready);
      goto done;

    not_complete:
      if (InternalState_Ready != mCurrentState) {
        setState(InternalState_Gathering);
      }

    done:
      ZS_LOG_TRACE(debug("step complete"))
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepRecheckIPTimer()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (Seconds() == mRecheckIPsDuration) {
        ZS_LOG_TRACE(log("do not need a recheck IP timer"))
        return true;
      }

      if ((isComplete()) &&
          (!mOptions.mContinuousGathering)) {
        ZS_LOG_TRACE(log("do not need to recheck IPs since already complete"))

        if (mRecheckIPsTimer) {
          mRecheckIPsTimer->cancel();
          mRecheckIPsTimer.reset();
        }
        return true;
      }

      if (mRecheckIPsTimer) {
        ZS_LOG_TRACE(log("already have a recheck IP timer"))
        return true;
      }

      ZS_LOG_DEBUG(log("creating a recheck ip timer") + ZS_PARAM("recheck duration (s)", mRecheckIPsDuration))
      mRecheckIPsTimer = ITimer::create(mThisWeak.lock(), mRecheckIPsDuration);
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCalculateOptionsHash()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (mOptionsHash.hasData()) {
        ZS_LOG_TRACE(log("options hash already calculated"))
        return true;
      }

      mOptionsHash = mOptions.hash();
      ZS_LOG_DEBUG(log("step options hash") + ZS_PARAMIZE(mOptionsHash))

      return true;
    }
    
    //-------------------------------------------------------------------------
    bool ICEGatherer::stepResolveHostIPs()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      ZS_LOG_TRACE(log("step resolve local IPs"))

      if ((mPendingHostIPs.size() < 1) &&
          (mResolveHostIPQueries.size() < 1)) {
        ZS_LOG_TRACE(log("nothing to resolve at this time"))
        return true;
      }

      HostIPSorter::QueryMap resolvedQueries;

      for (auto iter = mPendingHostIPs.begin(); iter != mPendingHostIPs.end(); ++iter) {
        auto data = (*iter);

        HostIPSorter::QueryData query;
        query.mOriginalData = data;

        ZS_THROW_INVALID_ASSUMPTION_IF(!data)

        if (data->mHostName.isEmpty()) {
          ZS_LOG_TRACE(log("no host name to resolve (attempt to add pre-resolved)"))
          goto add_if_valid;
        }

        query.mQuery = IDNS::lookupAorAAAA(mThisWeak.lock(), data->mHostName);
        if (!query.mQuery) {
          ZS_LOG_TRACE(log("did not create DNS query (attempt to add pre-resolved)"))
          goto add_if_valid;
        }

        {
          ZS_LOG_TRACE(log("waiting for query to resolve") + ZS_PARAM("query", query.mQuery->getID()));
          ZS_EVENTING_3(
                        x, i, Debug, IceGathererResolveHostIP, ol, IceGatherer, Info,
                        puid, id, mID,
                        puid, dnsQueryId, query.mQuery->getID(),
                        string, hostName, data->mHostName
                        );
          mResolveHostIPQueries[query.mQuery->getID()] = query;
          continue;
        }

      add_if_valid:
        {
          // immediately resolve if IP is appropriate
          if (data->mIP.isEmpty()) continue;
          if (data->mIP.isAddressEmpty()) continue;
          if (data->mIP.isLoopback()) continue;
          if (data->mIP.isAddrAny()) continue;

          query.mResolvedIPs.push_back(data->mIP);

          resolvedQueries[zsLib::createPUID()] = query;
        }
      }

      mPendingHostIPs.clear();

      for (auto iter_doNotUse = mResolveHostIPQueries.begin(); iter_doNotUse != mResolveHostIPQueries.end();) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        HostIPSorter::QueryID id = (*current).first;
        HostIPSorter::QueryData &query = (*current).second;

        if (!query.mQuery) {
          ZS_LOG_TRACE(log("query previously resolved") + ZS_PARAM("query id", id))
          continue;
        }

        if (!query.mQuery->isComplete()) {
          ZS_LOG_TRACE(log("still waiting for DNS to resolve") + ZS_PARAM("query id", id))
          continue;
        }

        IDNS::AResultPtr aRecords = query.mQuery->getA();
        IDNS::AAAAResultPtr aaaaRecords = query.mQuery->getAAAA();

        IPAddressList ips;
        if (aRecords) {
          ips = aRecords->mIPAddresses;
        }
        if (aaaaRecords) {
          for (auto ipIter = aaaaRecords->mIPAddresses.begin(); ipIter != aaaaRecords->mIPAddresses.end(); ++ipIter) {
            ips.push_back(*ipIter);
          }
        }

        query.mQuery.reset();

        HostIPSorter::QueryData tempQuery;
        tempQuery.mOriginalData = query.mOriginalData;
        tempQuery.mResolvedIPs = ips;

        // query is now finished
        for (auto ipIter = ips.begin(); ipIter != ips.end(); ++ipIter) {
          IPAddress &ip = (*ipIter);

          if (ip.isEmpty()) continue;
          if (ip.isAddressEmpty()) continue;
          if (ip.isLoopback()) continue;
          if (ip.isAddrAny()) continue;

          {
            ZS_LOG_TRACE(log("resolved host IP") + ZS_PARAM("ip", ip.string()))
            tempQuery.mResolvedIPs.push_back(ip);
          }
        }

        if (tempQuery.mResolvedIPs.size() > 1) {
          resolvedQueries[zsLib::createPUID()] = tempQuery;
        }

        mResolveHostIPQueries.erase(current);
      }

      for (auto iter = resolvedQueries.begin(); iter != resolvedQueries.end(); ++iter) {
        HostIPSorter::QueryData &query = (*iter).second;

        for (auto iterIP = query.mResolvedIPs.begin(); iterIP != query.mResolvedIPs.end(); ++iterIP) {
          auto ip = (*iterIP);

          for (auto iterResolved = mResolvedHostIPs.begin(); iterResolved != mResolvedHostIPs.end(); ++iterResolved) {
            const IPAddress &existingIP = (*iterResolved)->mIP;
            if (ip.isAddressEqualIgnoringIPv4Format(existingIP)) goto already_added;
          }

          goto add_newly_resolved;

        already_added:
          {
            ZS_LOG_WARNING(Trace, log("found duplicate resolved IP (thus ignoring)") + ZS_PARAM("ip", ip.string()))
            continue;
          }

        add_newly_resolved:
          {
            ZS_EVENTING_5(
                          x, i, Trace, IceGathererResolveFoundHostIP, ol, IceGatherer, Info,
                          puid, id, mID,
                          string, hostIp, ip.string(),
                          string, hostName, query.mOriginalData->mHostName,
                          string, interfaceName, query.mOriginalData->mInterfaceName,
                          ulong, adapterSpecific, query.mOriginalData->mIndex
                          );
            ZS_LOG_TRACE(log("found new host ip") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", query.mOriginalData->mInterfaceName) + ZS_PARAM("host", query.mOriginalData->mHostName));

            HostIPSorter::DataPtr data(make_shared<HostIPSorter::Data>());
            (*data) = (*query.mOriginalData);
            data->mIP = ip;
            mResolvedHostIPs.push_back(data);
          }
        }
      }

      // clear out all resolved queries
      resolvedQueries.clear();

      ZS_LOG_TRACE(log("query resolution") + ZS_PARAM("remaining", mResolveHostIPQueries.size()))
      return mResolveHostIPQueries.size() < 1;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepGetHostIPs()
    {
      typedef HostIPSorter::DataList DataList;

      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (!mGetLocalIPsNow) {
        ZS_LOG_TRACE(log("no need to check host IPs at this moment"))
        return true;
      }

      mGetLocalIPsNow = false;
      mHostsHash.clear();

      mResolvedHostIPs.clear();

      stepGetHostIPs_WinRT();
      stepGetHostIPs_Win32();
      stepGetHostIPs_ifaddr();

      if (mPendingHostIPs.size() > 0) {
        ZS_LOG_TRACE(log("not all host IPs resolved") + ZS_PARAM("pending size", mPendingHostIPs.size()))
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
        return false;
      }

      ZS_LOG_TRACE(log("all host IPs resolved") + ZS_PARAM("size", mResolvedHostIPs.size()))
      return true;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::stepGetHostIPs_WinRT()
    {
#if defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      // http://stackoverflow.com/questions/10336521/query-local-ip-address

      // Use WinRT GetHostNames to search for IP addresses
      {
        typedef std::map<String, bool> HostNameMap;
        typedef std::list<ConnectionProfile ^> ConnectionProfileList;

        HostNameMap previousFound;
        ConnectionProfileList profiles;

        // discover connection profiles
        {
          auto connectionProfiles = NetworkInformation::GetConnectionProfiles();
          for (auto iter = connectionProfiles->First(); iter->HasCurrent; iter->MoveNext()) {
            auto profile = iter->Current;
            if (nullptr == profile) {
              ZS_LOG_WARNING(Trace, log("found null profile"))
              continue;
            }
            profiles.push_back(profile);
          }

          ConnectionProfile ^current = NetworkInformation::GetInternetConnectionProfile();
          if (current) {
            ZS_LOG_INSANE("found current profile")
            profiles.push_back(current);
          }
        }

        // search connection profiles with host names found
        {
          auto hostnames = NetworkInformation::GetHostNames();
          for (auto iter = hostnames->First(); iter->HasCurrent; iter->MoveNext()) {
            auto hostname = iter->Current;
            if (nullptr == hostname) continue;

            String canonicalName;
            if (hostname->CanonicalName) {
              canonicalName = String(hostname->CanonicalName->Data());
            }

            String displayName;
            if (hostname->DisplayName) {
              displayName = String(hostname->DisplayName->Data());
            }

            String rawName;
            if (hostname->RawName) {
              rawName = String(hostname->RawName->Data());
            }

            String useName = rawName;

            auto found = previousFound.find(useName);
            if (found != previousFound.end()) {
              ZS_LOG_INSANE(log("already found IP") + ZS_PARAMIZE(useName))
              continue;
            }

            ConnectionProfile ^hostProfile = nullptr;
            if (hostname->IPInformation) {
              if (hostname->IPInformation->NetworkAdapter) {
                auto hostNetworkAdapter = hostname->IPInformation->NetworkAdapter;
                for (auto profileIter = profiles.begin(); profileIter != profiles.end(); ++profileIter) {
                  auto profile = (*profileIter);
                  auto adapter = profile->NetworkAdapter;
                  if (nullptr == adapter) {
                    ZS_LOG_WARNING(Insane, log("found null adapter"))
                    continue;
                  }
                  if (adapter->NetworkAdapterId != hostNetworkAdapter->NetworkAdapterId) {
                    ZS_LOG_INSANE(log("adapter does not match host adapter"))
                    continue;
                  }
                  // match found
                  hostProfile = profile;
                  break;
                }
              }
            }

            previousFound[useName] = true;

            IPAddress ip;

            if (IPAddress::isConvertable(useName)) {
              try {
                IPAddress temp(useName);
                ip = temp;
              } catch(IPAddress::Exceptions::ParseError &) {
                ZS_LOG_TRACE(log("name failed to resolve as IP") + ZS_PARAM("name", useName))
              }

              if (ip.isAddressEmpty()) continue;
              if (ip.isLoopback()) continue;
              if (ip.isAddrAny()) continue;
            }

            String profileName;

            if (hostProfile) {
              if (hostProfile->ProfileName) {
                profileName = String(hostProfile->ProfileName->Data());
              }
            }

            ZS_EVENTING_4(
                          x, i, Trace, IceGathererPendingResolveHostIP, ol, IceGatherer, Info,
                          puid, id, mID,
                          string, profileName, profileName,
                          string, useName, useName,
                          string, ip, ip.string()
                          );

            if (profileName.hasData()) {
              mPendingHostIPs.push_back(HostIPSorter::prepare(profileName, useName, ip, mInterfaceMappings, mOptions));
            } else {
              mPendingHostIPs.push_back(HostIPSorter::prepare(useName, ip, mOptions));
            }
          }
        }
      }
#endif //defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
    }
    
    //-------------------------------------------------------------------------
    void ICEGatherer::stepGetHostIPs_Win32()
    {
#ifdef HAVE_GETADAPTERADDRESSES
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx

#undef MALLOC
#undef FREE

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

      // scope: use GetAdaptersAddresses
      {
        ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_MULTICAST;

        DWORD dwSize = 0;
        DWORD dwRetVal = 0;

        ULONG family = AF_UNSPEC;

        LPVOID lpMsgBuf = NULL;

        PIP_ADAPTER_ADDRESSES pAddresses = NULL;

        // Allocate a 15 KB buffer to start with.
        ULONG outBufLen = WORKING_BUFFER_SIZE;
        ULONG Iterations = 0;

        PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
        //PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
        //PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
        //IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
        //IP_ADAPTER_PREFIX *pPrefix = NULL;

        outBufLen = WORKING_BUFFER_SIZE;

        do {

          pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
          ZS_THROW_BAD_STATE_IF(NULL == pAddresses)

          dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

          if (dwRetVal != ERROR_BUFFER_OVERFLOW) break;

          FREE(pAddresses);
          pAddresses = NULL;

          Iterations++;
        } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

        if (NO_ERROR == dwRetVal) {
          pCurrAddresses = pAddresses;
          while (pCurrAddresses) {

            // discover information about the adapter
            {
              switch (pCurrAddresses->OperStatus) {
                case IfOperStatusDown:           goto next_address;
                case IfOperStatusNotPresent:     goto next_address;
                case IfOperStatusLowerLayerDown: goto next_address;
              }

              IPAddress ip;

              ULONG adapterMetric = ULONG_MAX;

              pUnicast = pCurrAddresses->FirstUnicastAddress;
              while (pUnicast) {
                // scan unicast addresses
                {
                  if (pUnicast->Address.lpSockaddr) {
                    switch (pUnicast->Address.lpSockaddr->sa_family) {
                      case AF_INET: {
                        ip = IPAddress(*((sockaddr_in *)pUnicast->Address.lpSockaddr));
                        adapterMetric = pCurrAddresses->Ipv4Metric;
                        break;
                      }
                      case AF_INET6: {
                        ip = IPAddress(*((sockaddr_in6 *)pUnicast->Address.lpSockaddr));
                        adapterMetric = pCurrAddresses->Ipv6Metric;
                        break;
                      }
                    }
                  }

                  if (ip.isAddressEmpty()) goto next_unicast;
                  if (ip.isLoopback()) goto next_unicast;
                  if (ip.isAddrAny()) goto next_unicast;

                  String friendlyName(pCurrAddresses->FriendlyName);
                  String description(pCurrAddresses->Description);

                  ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", friendlyName) + ZS_PARAM("description", description));

                  ZS_EVENTING_5(
                                x, i, Trace, IceGathererResolveFoundHostIP, ol, IceGatherer, Info,
                                puid, id, mID,
                                string, hostIp, ip.string(),
                                string, hostName, friendlyName,
                                string, interfaceName, description,
                                ulong, adapterSpecific, adapterMetric
                                );

                  mResolvedHostIPs.push_back(HostIPSorter::prepare(friendlyName, description, ip, adapterMetric, mInterfaceMappings, mOptions));
                }

              next_unicast:
                {
                  pUnicast = pUnicast->Next;
                }
              }
            }

          next_address:
            {
              pCurrAddresses = pCurrAddresses->Next;
            }
          }
        } else {
          ZS_LOG_WARNING(Detail, log("failed to obtain IP address information") + ZS_PARAMIZE(dwRetVal))
        }

        FREE(pAddresses);
      }

#if 0
      // http://tangentsoft.net/wskfaq/examples/ipaddr.html

      // scope: use GetIpAddrTable (OLD CODE USED FOR IPv4 ONLY)
      {
        ULONG size = 0;

        // the 1st call is just to get the table size
        if(GetIpAddrTable(NULL, &size, FALSE) == ERROR_INSUFFICIENT_BUFFER)
        {
          // now that you know the size, allocate a pointer
          MIB_IPADDRTABLE *ipAddr = (MIB_IPADDRTABLE *) new BYTE[size];
          // the 2nd call is to retrieve the info for real
          if(GetIpAddrTable(ipAddr, &size, TRUE) == NO_ERROR)
          {
            // need to loop it to handle multiple interfaces
            for(DWORD i = 0; i < ipAddr->dwNumEntries; i++)
            {
              // this is the IP address
              DWORD dwordIP = ntohl(ipAddr->table[i].dwAddr);
              IPAddress ip(dwordIP);

              if (ip.isAddressEmpty()) continue;
              if (ip.isLoopback()) continue;
              if (ip.isAddrAny()) continue;
              
              ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()));

              ZS_EVENTING_5(
                            x, i, Trace, IceGathererResolveFoundHostIP, ol, IceGatherer, Info,
                            puid, id, mID,
                            string, hostIp, ip.string(),
                            string, hostName, (const char *)NULL,
                            string, interfaceName, (const char *)NULL,
                            ulong, adapterSpecific, 0
                            );

              mResolvedHostIPs.push_back(HostIPSorter::prepare(ip, mOptions));
            }
          }
        }
      }
#endif //0
#endif //HAVE_GETADAPTERADDRESSES
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::stepGetHostIPs_ifaddr()
    {
#ifdef HAVE_GETIFADDRS
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      // scope: use getifaddrs
      {
        ifaddrs *ifAddrStruct = NULL;
        ifaddrs *ifa = NULL;

        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
        {
          IPAddress ip;
          if (AF_INET == ifa->ifa_addr->sa_family) {
            ip = IPAddress(*((sockaddr_in *)ifa->ifa_addr));      // this is an IPv4 address
          } else if (AF_INET6 == ifa->ifa_addr->sa_family) {
            ip = IPAddress(*((sockaddr_in6 *)ifa->ifa_addr));     // this is an IPv6 address
          }

          // do not add these addresses...
          if (ip.isAddressEmpty()) continue;
          if (ip.isLoopback()) continue;
          if (ip.isAddrAny()) continue;

          ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", ifa->ifa_name));

          ZS_EVENTING_5(
                        x, i, Trace, IceGathererResolveFoundHostIP, ol, IceGatherer, Info,
                        puid, id, mID,
                        string, hostIp, ip.string(),
                        string, hostName, (const char *)NULL,
                        string, interfaceName, ifa->ifa_name,
                        ulong, adapterSpecific, ifa->ifa_flags
                        );

          auto data = HostIPSorter::prepare(ifa->ifa_name, ip, mInterfaceMappings, mOptions);

#ifdef HAVE_NETINIT6_IN6_VAR_H
          if (0 != (IN6_IFF_TEMPORARY & ifa->ifa_flags)) {
            data->mIsTemporaryIP = true;
          }
#endif //HAVE_NETINIT6_IN6_VAR_H

          mResolvedHostIPs.push_back(data);
        }

        if (ifAddrStruct) {
          freeifaddrs(ifAddrStruct);
          ifAddrStruct = NULL;
        }
      }
#endif //HAVE_GETIFADDRS
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCalculateHostsHash()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (mHostsHash.hasData()) {
        ZS_LOG_TRACE(log("host hash already calculated"))
        return true;
      }

      HostIPSorter::sort(mResolvedHostIPs);

      ZS_LOG_DEBUG(log("--- GATHERING HOST IPs: START ---"))

      auto hasher = IHasher::sha1();

      for (auto iter = mResolvedHostIPs.begin(); iter != mResolvedHostIPs.end(); ++iter) {
        auto data = (*iter);
        ZS_LOG_DEBUG(log("found host IP") + ZS_PARAM("ip", data->mIP.string()))

        hasher->update(data->mIP.string());
        hasher->update(":");
      }

      mHostsHash = hasher->finalizeAsString();

      ZS_LOG_DEBUG(log("--- GATHERING HOST IPs: END ---") + ZS_PARAM("hash", mHostsHash))
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepFixHostPorts()
    {
      typedef std::map<IPAddress, HostIPSorter::DataPtr> IPDataMap;

      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (mHostsHash == mLastFixedHostPortsHostsHash) {
        ZS_LOG_TRACE(log("host ports have not changed (thus no need to fix hosts)"))
        return true;
      }

      ZS_LOG_DEBUG(log("host ports has changed") + ZS_PARAMIZE(mHostsHash) + ZS_PARAMIZE(mLastFixedHostPortsHostsHash))

      IPDataMap useHosts;
      for (auto iterResolved = mResolvedHostIPs.begin(); iterResolved != mResolvedHostIPs.end(); ++iterResolved) {
        auto hostData = (*iterResolved);
        if (!needsHostPort(hostData)) {
          ZS_LOG_TRACE(log("resolved host ip will not be used (filtered out entirely)") + hostData->toDebug())
          continue;
        }
        ZS_LOG_TRACE(log("resolved IP can be used") + hostData->toDebug())
        useHosts[hostData->mIP] = hostData;
      }

      ZS_LOG_DEBUG(log("discovered all hosts that must be bound") + ZS_PARAM("total", useHosts.size()))

      // first shutdown any host ports which are now gone
      for (auto iter_doNotUse = mHostPorts.begin(); iter_doNotUse != mHostPorts.end();) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto hostIP = (*current).first;
        auto hostPort = (*current).second;

        auto found = useHosts.find(hostIP);
        if (found != useHosts.end()) {
          ZS_LOG_TRACE(log("host is still in use") + hostPort->toDebug())
          continue;  // this host is still in use
        }

        ZS_LOG_TRACE(log("host IP needs to be shutdown since host IP is no longer in used") + hostPort->toDebug())
        shutdown(hostPort);
      }

      ZS_LOG_DEBUG(log("host ports remaining active (after shutting down unused hosts)") + ZS_PARAM("total", mHostPorts.size()))

      // set up missing host ports
      for (auto iter = useHosts.begin(); iter != useHosts.end(); ++iter) {
        auto hostData = (*iter).second;
        auto found = mHostPorts.find(hostData->mIP);
        if (found != mHostPorts.end()) {
          ZS_LOG_TRACE(log("already have host port information") + hostData->toDebug())
          continue;
        }

        HostPortPtr hostPort(make_shared<HostPort>());

        hostPort->mHostData = hostData;

        ZS_EVENTING_3(
                      x, i, Trace, OrtcIceGathererHostPortCreate, ol, IceGatherer, Info,
                      puid, id, mID,
                      puid, hostPortId, hostPort->mID,
                      string, ip, hostData->mIP.string()
                      );

        mHostPorts[hostData->mIP] = hostPort;
      }

      ZS_LOG_DEBUG(log("total hosts now set up") + ZS_PARAM("total", mHostPorts.size()))

      mLastFixedHostPortsHostsHash = mHostsHash;
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepBindHostPorts()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (mHostsHash == mLastBoundHostPortsHostHash) {
        ZS_LOG_TRACE(log("host ports have not changed (thus no need to bind ports)"));
        return true;
      }

      if ((isComplete()) &&
          (!mOptions.mContinuousGathering)) {
        ZS_LOG_TRACE(log("will not bind to new hosts because already complete"));
        return true;
      }

      bool allBound = true;

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        if (hostPort->mBoundOptionsHash == mOptionsHash) {
          ZS_LOG_TRACE(log("already bound udp / tcp socket") + hostPort->toDebug());
          continue;
        }

        if (!hostPort->mBoundUDPSocket) {
          bool firstAttempt = false;
          if (!hostPort->mBindUDPBackOffTimer) {
            hostPort->mBindUDPBackOffTimer = UseBackOffTimer::create(ISettings::getString(ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER), mThisWeak.lock());
            firstAttempt = true;
          }

          if (hostPort->mBindUDPBackOffTimer->shouldAttemptNow()) {
            hostPort->mBindUDPBackOffTimer->notifyAttempting();

            IPAddress bindIP(hostPort->mHostData->mIP);
            hostPort->mBoundUDPSocket = bind(firstAttempt, bindIP, IICETypes::Protocol_UDP);
            if (hostPort->mBoundUDPSocket) {
              ZS_EVENTING_4(
                            x, i, Debug, IceGathererHostPortBind, ol, IceGatherer, HostSocketBind,
                            puid, id, mID,
                            puid, hostPortId, hostPort->mID,
                            string, hostIp, bindIP.string(),
                            string, protocolType, IICETypes::toString(IICETypes::Protocol_UDP)
                            );

              ZS_LOG_DEBUG(log("successfully bound UDP socket") + hostPort->toDebug())

              hostPort->mBindUDPBackOffTimer->cancel();
              hostPort->mBindUDPBackOffTimer.reset();

              hostPort->mBoundUDPIP = bindIP;
              mHostPortSockets[hostPort->mBoundUDPSocket] = hostPort;
              hostPort->mCandidateUDP = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Host, bindIP);
            } else {
              ZS_EVENTING_4(
                            x, e, Debug, IceGathererHostPortBindFailed, ol, IceGatherer, HostSocketBind,
                            puid, id, mID,
                            puid, hostPortId, hostPort->mID,
                            string, hostIp, bindIP.string(),
                            string, protocolType, IICETypes::toString(IICETypes::Protocol_UDP)
                            );

              hostPort->mBindUDPBackOffTimer->notifyAttemptFailed();
              ZS_LOG_WARNING(Debug, log("failed to bind UDP socket") + hostPort->toDebug())
            }
          } else {
            ZS_LOG_WARNING(Trace, log("not ready to retry binding UDP just yet") + hostPort->toDebug())
          }
        } else {
          addCandidate(*(hostPort->mHostData), hostPort->mBoundUDPIP, hostPort->mCandidateUDP);
        }

        // scope: create TCP socket
        if (mCreateTCPCandidates) {
          if (!hostPort->mBoundTCPSocket) {
            bool firstAttempt = false;
            if (!hostPort->mBindTCPBackOffTimer) {
              hostPort->mBindTCPBackOffTimer = UseBackOffTimer::create(ISettings::getString(ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER), mThisWeak.lock());
              firstAttempt = true;
            }

            if (hostPort->mBindTCPBackOffTimer->shouldAttemptNow()) {
              hostPort->mBindTCPBackOffTimer->notifyAttempting();

              IPAddress bindIP(hostPort->mHostData->mIP);
              hostPort->mBoundTCPSocket = bind(firstAttempt, bindIP, IICETypes::Protocol_TCP);
              if (hostPort->mBoundTCPSocket) {
                ZS_EVENTING_4(
                              x, i, Debug, IceGathererHostPortBind, ol, IceGatherer, HostSocketBind,
                              puid, id, mID,
                              puid, hostPortId, hostPort->mID,
                              string, hostIp, bindIP.string(),
                              string, protocolType, IICETypes::toString(IICETypes::Protocol_TCP)
                              );
                ZS_LOG_DEBUG(log("successfully bound TCP socket") + ZS_PARAM("bind ip", bindIP.string()))

                hostPort->mBindTCPBackOffTimer->cancel();
                hostPort->mBindTCPBackOffTimer.reset();

                hostPort->mBoundTCPIP = bindIP;
                mHostPortSockets[hostPort->mBoundTCPSocket] = hostPort;
                if (mGatherPassiveTCP) {
                  hostPort->mCandidateTCPPassive = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Host, bindIP, IICETypes::Protocol_TCP, IICETypes::TCPCandidateType_Passive);
                }
                IPAddress bindActiveIP(bindIP);
                bindActiveIP.setPort(9);
                hostPort->mCandidateTCPActive = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Host, bindActiveIP, IICETypes::Protocol_TCP, IICETypes::TCPCandidateType_Active);
              } else {
                ZS_EVENTING_4(
                              x, e, Debug, IceGathererHostPortBindFailed, ol, IceGatherer, HostSocketBind,
                              puid, id, mID,
                              puid, hostPortId, hostPort->mID,
                              string, hostIp, bindIP.string(),
                              string, protocolType, IICETypes::toString(IICETypes::Protocol_TCP)
                              );
                hostPort->mBindTCPBackOffTimer->notifyAttemptFailed();
                ZS_LOG_WARNING(Debug, log("failed to bind TCP socket") + hostPort->toDebug() + ZS_PARAM("bind ip", bindIP.string()))
              }
            } else {
              ZS_LOG_WARNING(Trace, log("not ready to retry binding TCP just yet") + hostPort->toDebug())
            }
          } else {
            addCandidate(*(hostPort->mHostData), hostPort->mBoundTCPIP, hostPort->mCandidateTCPPassive);
            addCandidate(*(hostPort->mHostData), hostPort->mBoundTCPIP, hostPort->mCandidateTCPActive);
          }
        }

        bool udpComplete = (hostPort->mBoundUDPSocket) || (hostPort->mBindUDPBackOffTimer ? hostPort->mBindUDPBackOffTimer->isComplete() : true);
        bool tcpComplete = (!mCreateTCPCandidates) || (hostPort->mBoundTCPSocket) || (hostPort->mBindTCPBackOffTimer ? hostPort->mBindTCPBackOffTimer->isComplete() : true);

        if ((udpComplete) &&
            (tcpComplete)) {

          if ((hostPort->mBindUDPBackOffTimer ? hostPort->mBindUDPBackOffTimer->haveAllAttemptsFailed() : false) ||
              (hostPort->mBindTCPBackOffTimer ? hostPort->mBindTCPBackOffTimer->haveAllAttemptsFailed() : false)) {
            goto host_port_max_bind_failures_reached;
          }

          goto host_bound;

        }

        goto host_not_bound;

      host_not_bound:
        {
          ZS_LOG_WARNING(Trace, log("failed to bind udp / tcp socket") + hostPort->toDebug())
          allBound = false;
          continue;
        }
      host_port_max_bind_failures_reached:
        {
          ZS_LOG_TRACE(log("host port bind reach maximum failures") + hostPort->toDebug())
          hostPort->mBoundOptionsHash = mOptionsHash;
          continue;
        }
      host_bound:
        {
          ZS_LOG_TRACE(log("bound udp / tcp socket") + hostPort->toDebug())
          hostPort->mBoundOptionsHash = mOptionsHash;
          continue;
        }
      }

      if (allBound) {
        mLastBoundHostPortsHostHash = mHostsHash;

        ZS_LOG_DEBUG(log("all sockets have been bound (or reached maximum attempt failures)"))
        return true;
      }

      ZS_LOG_WARNING(Debug, log("not all sockets bound"))

      // allow creation of reflexive and relay candidates (even though not everything was bound)
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCheckTransportsNeedWarmth()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (!mTransportsChanged) {
        ZS_LOG_TRACE(log("transport information has not changed"))
        return true;
      }

      bool wasNeedingMoreCandidates = mTransportsStillNeedsCandidates;

      mTransportsChanged = false;
      mTransportsStillNeedsCandidates = mKeepWarmSinceJustCreated;

      for (auto iter_doNotUse = mInstalledTransports.begin(); iter_doNotUse != mInstalledTransports.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto installedTransport = (*current).second;

        auto transport = installedTransport->mTransport.lock();
        if (!transport) {
          removeAllRelatedRoutes(installedTransport->mTransportID, UseICETransportPtr());
          mInstalledTransports.erase(current);
          continue;
        }

        mTransportsStillNeedsCandidates = transport->needsMoreCandidates();
        if (mTransportsStillNeedsCandidates) {
          ZS_LOG_TRACE(log("found at least one transport needing more candidates") + installedTransport->toDebug())
          goto check_if_changed;
        }
      }

      for (auto iter_doNotUse = mPendingTransports.begin(); iter_doNotUse != mPendingTransports.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto installedTransport = (*current);

        auto transport = installedTransport->mTransport.lock();
        if (!transport) {
          removeAllRelatedRoutes(installedTransport->mTransportID, UseICETransportPtr());
          mPendingTransports.erase(current);
          continue;
        }

        mTransportsStillNeedsCandidates = transport->needsMoreCandidates();
        if (mTransportsStillNeedsCandidates) {
          ZS_LOG_TRACE(log("found at least one transport needing more candidates") + installedTransport->toDebug())
          goto check_if_changed;
        }
      }

      ZS_LOG_TRACE(log("did not find any transports needing candidates at this time"))
      goto check_if_changed;

    check_if_changed:
      {
        if (wasNeedingMoreCandidates == mTransportsStillNeedsCandidates) {
          ZS_LOG_TRACE(log("no change in need for candidates") + ZS_PARAM("needing more candidates", mTransportsStillNeedsCandidates))
          return true;
        }

        mLastReflexiveHostsHash.clear();
        mLastRelayHostsHash.clear();
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepWarmUpAfterInterfaceBinding()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (mWarmUpAfterNewInterfaceBindingHostsHash == mHostsHash) {
        ZS_LOG_TRACE(log("no new interfaces to bind"))
        return true;
      }

      if (mWarmUpAterNewInterfaceBindingTimer) {
        mWarmUpAterNewInterfaceBindingTimer->cancel();
        mWarmUpAterNewInterfaceBindingTimer.reset();
      }
      mWarmUpAfterNewInterfaceBindingUntil = Time();

      if ((isComplete()) &&
          (!mOptions.mContinuousGathering)) {
        ZS_LOG_TRACE(log("already complete so cannot warm up after interface binding"))
        return true;
      }

      mWarmUpAfterNewInterfaceBindingHostsHash = mHostsHash;

      mWarmUpAfterNewInterfaceBindingUntil = zsLib::now() + Seconds(ISettings::getUInt(ORTC_SETTING_GATHERER_WARM_UP_TIME_AFTER_NEW_INTERFACE_IN_SECONDS));

      mWarmUpAterNewInterfaceBindingTimer = ITimer::create(mThisWeak.lock(), mWarmUpAfterNewInterfaceBindingUntil);

      // force the reflexive / relay to be setup again (if needed)
      mLastReflexiveHostsHash.clear();
      mLastRelayHostsHash.clear();

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepSetupReflexive()
    {
      typedef std::map<String, bool> HashMap;

      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      bool keepWarm = shouldKeepWarm();

      if ((!keepWarm) &&
          (!shouldWarmUpAfterInterfaceBinding())) {
        ZS_LOG_TRACE(log("no reflexive candidates should be setup at this time (not being kept warm)"))
        return true;
      }

      if (mHostsHash == mLastReflexiveHostsHash) {
        ZS_LOG_TRACE(log("hosts have not changed (thus no need to setup reflexive ports)"))
        return true;
      }

      if ((isComplete()) &&
          (!mOptions.mContinuousGathering)) {
        ZS_LOG_TRACE(log("will not setup reflexive because already complete"))
        return true;
      }

      bool allSetup = true;

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        if (!hostPort->mBoundUDPSocket) {
          ZS_LOG_WARNING(Trace, log("cannot setup reflexive until local socket is bound") + hostPort->toDebug())
          allSetup = false;
          continue;
        }

        bool filterOut = false;

        if (hostPort->mHostData->mIP.isIPv4()) {
          if (0 != (hostPort->mHostData->mFilterPolicy & FilterPolicy_NoIPv4Srflx)) {
            ZS_LOG_TRACE(log("filtering out IPv4 server reflexive (thus do not setup)"))
            filterOut = true;
          }
        } else {
          if (0 != (hostPort->mHostData->mFilterPolicy & FilterPolicy_NoIPv6Srflx)) {
            ZS_LOG_TRACE(log("filtering out IPv6 server reflexive (thus do not setup)"))
            filterOut = true;
          }
        }

        if (filterOut) {
          for (auto iterReflex_doNotUse = hostPort->mReflexivePorts.begin(); iterReflex_doNotUse != hostPort->mReflexivePorts.end(); ) {
            auto currentReflex = iterReflex_doNotUse;
            ++iterReflex_doNotUse;

            auto reflexivePort = (*currentReflex);

            ZS_LOG_TRACE(log("shutting down reflexive port (since being filtered)") + reflexivePort->toDebug())
            shutdown(reflexivePort, hostPort);
          }
          hostPort->mReflexivePorts.clear();
          continue;
        }

        bool hostPortSetup = true;

        if (hostPort->mReflexiveOptionsHash == mOptionsHash) {
          ZS_LOG_TRACE(log("already setup reflexive candidates") + hostPort->toDebug())
          continue;
        }

        HashMap foundServers;

        for (auto iterOptions = mOptions.mICEServers.begin(); iterOptions != mOptions.mICEServers.end(); ++iterOptions) {
          auto server = (*iterOptions);
          if (!isServerType(server, "stun:")) continue;

          String serverHash = server.hash();

          foundServers[serverHash] = true;

          ReflexivePortPtr reflexivePort;

          for (auto iterReflex_doNotUse = hostPort->mReflexivePorts.begin(); iterReflex_doNotUse != hostPort->mReflexivePorts.end(); ) {
            auto currentReflex = iterReflex_doNotUse;
            ++iterReflex_doNotUse;

            reflexivePort = (*currentReflex);

            if (reflexivePort->mServer.hash() != serverHash) continue;

            if (mOptionsHash != reflexivePort->mOptionsHash) {
              shutdown(reflexivePort, hostPort);
              goto not_found_server;
            }

            goto found_server;
          }

          goto not_found_server;

        not_found_server:
          {
            if (!keepWarm) {
              if (!hostPort->mWarmUpAfterBinding) {
                ZS_LOG_TRACE(log("host port does not need reflexive candidates to be warmed up at this time") + hostPort->toDebug() + server.toDebug())
                continue;
              }
            }

            reflexivePort = make_shared<ReflexivePort>();
            reflexivePort->mServer = server;

            ZS_EVENTING_3(
                          x, i, Debug, IceGathererReflexivePortCreate, ol, IceGatherer, Start,
                          puid, id, mID,
                          puid, reflexivePortId, reflexivePort->mID,
                          string, serverIp, server.mURLs.size() > 0 ? server.mURLs.front() : String()
                          );

            hostPort->mReflexivePorts.push_back(reflexivePort);
            goto found_server;
          }

        found_server:
          {
            reflexivePort->mOptionsHash = mOptionsHash;

            if (!reflexivePort->mSTUNDiscovery) {
              ZS_LOG_DEBUG(log("setting up stun discovery") + server.toDebug())

              Seconds keepAliveTime {};

              auto keepAliveInSeconds = ISettings::getUInt(ORTC_SETTING_GATHERER_DEFAULT_STUN_KEEP_ALIVE_IN_SECONDS);
              if (0 != keepAliveInSeconds) {
                keepAliveTime = Seconds(keepAliveInSeconds);
              }

              UseDNS::SRVLookupTypes lookup = (UseDNS::SRVLookupTypes)(UseDNS::SRVLookupType_AutoLookupA | UseDNS::SRVLookupType_FallbackToALookup);
              if (hostPort->mBoundUDPIP.isIPv6()) {
                lookup = (UseDNS::SRVLookupTypes)(UseDNS::SRVLookupType_AutoLookupAAAA | UseDNS::SRVLookupType_FallbackToAAAALookup);
              }

              UseSTUNDiscovery::CreationOptions options;
              options.mServers = createDNSLookupString(server, "stun:");
              options.mLookupType = lookup;
              options.mKeepWarmPingTime = keepAliveTime;

              reflexivePort->mSTUNDiscovery = UseSTUNDiscovery::create(IHelper::getServiceQueue(), mThisWeak.lock(), options);
              ZS_THROW_UNEXPECTED_ERROR_IF(!reflexivePort->mSTUNDiscovery)

              mSTUNDiscoveries[reflexivePort->mSTUNDiscovery] = HostAndReflexivePortPair(hostPort, reflexivePort);
            }

            if (!reflexivePort->mSTUNDiscovery->isComplete()) {
              ZS_LOG_TRACE(log("still waiting for stun discovery to complete"))
              hostPortSetup = allSetup = false;
              continue;
            }

            IPAddress ip = reflexivePort->mSTUNDiscovery->getMappedAddress();

            if (reflexivePort->mCandidate) {
              ZS_LOG_TRACE(log("already have candidate") + reflexivePort->mCandidate->toDebug())
              addCandidate(*(hostPort->mHostData), hostPort->mBoundUDPIP, reflexivePort->mCandidate);
            }

            if (ip.isAddressEmpty()) {
              ZS_LOG_WARNING(Debug, log("failed to obtain server reflexive address") + hostPort->toDebug() + server.toDebug())
              continue;
            }

            if (reflexivePort->mCandidate) {

              if (reflexivePort->mCandidate->ip() == ip) {
                ZS_LOG_TRACE(log("candidate did not change") + reflexivePort->mCandidate->toDebug())
                continue;
              }

              ZS_LOG_WARNING(Debug, log("reflexive address has changed") + hostPort->toDebug() + server.toDebug() + ZS_PARAM("new ip", ip.string()) + reflexivePort->mCandidate->toDebug())

              removeCandidate(reflexivePort->mCandidate);
            }

            ZS_EVENTING_3(
                          x, i, Debug, IceGathererReflexivePortFoundMapped, ol, IceGatherer, Found,
                          puid, id, mID,
                          puid, reflexivePortId, reflexivePort->mID,
                          string, mappedIp, ip.string()
                          );

            reflexivePort->mCandidate = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Srflex, hostPort->mBoundUDPIP, hostPort->mBoundUDPIP, ip, server);

            ZS_LOG_DEBUG(log("found reflexive candidate") + reflexivePort->mCandidate->toDebug())
            continue;
          }
        }

        ZS_LOG_TRACE(log("scan through reflexive ports for servers no longer in use"))

        for (auto iterReflex_doNotUse = hostPort->mReflexivePorts.begin(); iterReflex_doNotUse != hostPort->mReflexivePorts.end(); ) {
          auto currentReflex = iterReflex_doNotUse;
          ++iterReflex_doNotUse;

          auto reflexivePort = (*currentReflex);
          String hash = reflexivePort->mServer.hash();

          auto found = foundServers.find(hash);
          if (found != foundServers.end()) continue;

          ZS_LOG_DEBUG(log("reflexive server no longer required (thus shutting down)") + reflexivePort->mServer.toDebug())

          shutdown(reflexivePort, hostPort);
        }

        if (hostPortSetup) {
          hostPort->mReflexiveOptionsHash = mOptionsHash;
        }
      }

      if (allSetup) {
        ZS_LOG_DEBUG(log("all reflexive candidates are prepared (or failed)"))
        mLastReflexiveHostsHash = mHostsHash;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepTearDownReflexive()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if ((shouldKeepWarm()) ||
          (shouldWarmUpAfterInterfaceBinding())) {
        ZS_LOG_TRACE(log("no reflexive candidates should be torn down at this time (being kept warm)"))
        return true;
      }

      if (mHostsHash == mLastReflexiveHostsHash) {
        ZS_LOG_TRACE(log("hosts have not changed (thus no need to tear down reflexive ports)"))
        return true;
      }

      bool allDone = true;

      Time now = zsLib::now();

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        bool allHostDone = true;

        for (auto iterReflex_doNotUse = hostPort->mReflexivePorts.begin(); iterReflex_doNotUse != hostPort->mReflexivePorts.end(); ) {
          auto currentReflex = iterReflex_doNotUse;
          ++iterReflex_doNotUse;

          auto reflexivePort = (*currentReflex);

          if (Time() == reflexivePort->mLastActivity) goto shutdown_reflexive;
          if (reflexivePort->mLastActivity + mReflexiveInactivityTime <= now) goto shutdown_reflexive;

          if (!reflexivePort->mInactivityTimer) {
            Time fireAt = reflexivePort->mLastActivity + mReflexiveInactivityTime;
            reflexivePort->mInactivityTimer = ITimer::create(mThisWeak.lock(), fireAt);
            mReflexiveInactivityTimers[reflexivePort->mInactivityTimer] = HostAndReflexivePortPair(hostPort, reflexivePort);
            ZS_LOG_TRACE(log("setup reflexive inactivity timeout") + ZS_PARAMIZE(fireAt) + reflexivePort->toDebug())
          }
          goto wait_until_inactive;

        wait_until_inactive:
          {
            ZS_LOG_TRACE(log("reflexive port still active thus cannot shutdown") + reflexivePort->toDebug())
            allHostDone = allDone = false;
            continue;
          }

        shutdown_reflexive:
          {
            ZS_LOG_DEBUG(log("reflexive port no longer being kept warm (and is currently inactive)") + reflexivePort->toDebug())
            shutdown(reflexivePort, hostPort);
          }

        }

        if (allHostDone) {
          hostPort->mReflexivePorts.clear();
        }
      }

      if (allDone) {
        ZS_LOG_DEBUG(log("all reflexive candidates are torn down (no longer being kept warm)"))
        mLastReflexiveHostsHash = mHostsHash;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepSetupRelay()
    {
      typedef std::map<String, bool> HashMap;

      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      bool keepWarm = shouldKeepWarm();

      if ((!keepWarm) &&
          (!shouldWarmUpAfterInterfaceBinding())) {
        ZS_LOG_TRACE(log("no relay candidates should be setup at this time (not being kept warm)"))
        return true;
      }

      if (mHostsHash == mLastRelayHostsHash) {
        ZS_LOG_TRACE(log("hosts have not changed (thus no need to setup relay ports)"))
        return true;
      }

      if ((isComplete()) &&
          (!mOptions.mContinuousGathering)) {
        ZS_LOG_TRACE(log("will not setup reflexive because already complete"))
        return true;
      }

      Time now = zsLib::now();

      bool allSetup = true;

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        if (!hostPort->mBoundUDPSocket) {
          ZS_LOG_WARNING(Trace, log("cannot setup relay until local socket is bound"))
          allSetup = false;
          continue;
        }

        bool filterOut = false;

        if (hostPort->mHostData->mIP.isIPv4()) {
          if (0 != (hostPort->mHostData->mFilterPolicy & FilterPolicy_NoIPv4Relay)) {
            ZS_LOG_TRACE(log("filtering out IPv4 relay (thus do not setup)"))
            filterOut = true;
          }
        } else {
          if (0 != (hostPort->mHostData->mFilterPolicy & FilterPolicy_NoIPv6Relay)) {
            ZS_LOG_TRACE(log("filtering out IPv6 relay (thus do not setup)"))
            filterOut = true;
          }
        }

        if (filterOut) {
          for (auto iterRelay_doNotUse = hostPort->mRelayPorts.begin(); iterRelay_doNotUse != hostPort->mRelayPorts.end(); ) {
            auto currentRelay = iterRelay_doNotUse;
            ++iterRelay_doNotUse;

            auto relayPort = (*currentRelay);

            ZS_LOG_TRACE(log("shutting down relay port (since being filtered)") + relayPort->toDebug())
            shutdown(relayPort, hostPort);
          }
          hostPort->mRelayPorts.clear();
          continue;
        }

        bool hostPortSetup = true;

        if (hostPort->mRelayOptionsHash == mOptionsHash) {
          ZS_LOG_TRACE(log("already setup relay candidates") + hostPort->toDebug())
          continue;
        }

        HashMap foundServers;

        for (auto iterOptions = mOptions.mICEServers.begin(); iterOptions != mOptions.mICEServers.end(); ++iterOptions) {
          auto server = (*iterOptions);
          if (!isServerType(server, "turn:")) continue;

          String serverHash = server.hash();

          foundServers[serverHash] = true;

          RelayPortPtr relayPort;

          for (auto iterRelay_doNotUse = hostPort->mRelayPorts.begin(); iterRelay_doNotUse != hostPort->mRelayPorts.end(); ) {
            auto currentRelay = iterRelay_doNotUse;
            ++iterRelay_doNotUse;

            relayPort = (*currentRelay);

            if (relayPort->mServer.hash() != serverHash) continue;

            if (mOptionsHash != relayPort->mOptionsHash) {
              shutdown(relayPort, hostPort);
              goto not_found_server;
            }

            goto found_server;
          }

          goto not_found_server;

        not_found_server:
          {
            if (!keepWarm) {
              if (!hostPort->mWarmUpAfterBinding) {
                ZS_LOG_TRACE(log("no need to set up relay port at this time") + hostPort->toDebug() + server.toDebug())
                continue;
              }
            }

            relayPort = make_shared<RelayPort>();
            relayPort->mServer = server;

            ZS_EVENTING_6(
                          x, i, Debug, IceGathererRelayPortCreate, ol, IceGatherer, Found,
                          puid, id, mID,
                          puid, relayPort, relayPort->mID,
                          string, firstServerUrl, server.mURLs.size() > 0 ? server.mURLs.front() : String(),
                          string, username, server.mUserName,
                          string, credential, server.mCredential,
                          string, credentialType, IICEGathererTypes::toString(server.mCredentialType)
                          );

            hostPort->mRelayPorts.push_back(relayPort);
          }

        found_server:
          {
            relayPort->mOptionsHash = mOptionsHash;

            if (!relayPort->mTURNSocket) {
              ZS_LOG_DEBUG(log("setting up turn socket") + server.toDebug() + hostPort->toDebug())

              UseDNS::SRVLookupTypes lookup = (UseDNS::SRVLookupTypes)(UseDNS::SRVLookupType_AutoLookupA | UseDNS::SRVLookupType_FallbackToALookup);
              if (hostPort->mBoundUDPIP.isIPv6()) {
                lookup = (UseDNS::SRVLookupTypes)(UseDNS::SRVLookupType_AutoLookupAAAA | UseDNS::SRVLookupType_FallbackToAAAALookup);
              }

              UseTURNSocket::CreationOptions options;
              options.mServers = createDNSLookupString(server, "turn:");
              options.mUsername = relayPort->mServer.mUserName;
              options.mPassword = relayPort->mServer.mCredential;
              options.mLookupType = lookup;
              options.mUseChannelBinding = true;

              relayPort->mTURNSocket = UseTURNSocket::create(IHelper::getServiceQueue(), mThisWeak.lock(), options);
              ZS_THROW_UNEXPECTED_ERROR_IF(!relayPort->mTURNSocket);

              ZS_LOG_DEBUG(log("turn socket created") + ZS_PARAM("turn socket", relayPort->mTURNSocket->getID()) + ZS_PARAM("source ip", hostPort->mBoundUDPIP.string()) + hostPort->toDebug())

              mTURNSockets[relayPort->mTURNSocket] = HostAndRelayPortPair(hostPort, relayPort);
            }

            bool ready = true;

            switch (relayPort->mTURNSocket->getState()) {
              case UseTURNSocket::TURNSocketState_Pending: {
                ZS_LOG_TRACE(log("still waiting for TURN socket to be ready") + relayPort->toDebug())
                ready = hostPortSetup = allSetup = false;
                break;
              }
              case UseTURNSocket::TURNSocketState_Ready: {
                ZS_LOG_TRACE(log("TURN socket is ready") + relayPort->toDebug())
                if (Time() == relayPort->mLastActivity) {
                  relayPort->mLastActivity = now;
                }
                if (relayPort->mServerResponseIP.isAddressEmpty()) {
                  relayPort->mServerResponseIP = relayPort->mTURNSocket->getServerResponseIP();
                  hostPort->mIPToRelayPortMapping[relayPort->mServerResponseIP] = relayPort;
                }
                break;
              }
              case UseTURNSocket::TURNSocketState_ShuttingDown:
              case UseTURNSocket::TURNSocketState_Shutdown: {
                ZS_LOG_WARNING(Trace, log("TURN socket is shutdown") + relayPort->toDebug())
                ready = false;
                if (!relayPort->mServerResponseIP.isAddressEmpty()) {
                  auto found = hostPort->mIPToRelayPortMapping.find(relayPort->mServerResponseIP);
                  if (found != hostPort->mIPToRelayPortMapping.end()) {
                    hostPort->mIPToRelayPortMapping.erase(found);
                  }
                }
                if (relayPort->mRelayCandidate) {
                  removeCandidate(relayPort->mRelayCandidate);
                  relayPort->mRelayCandidate.reset();
                }
                if (relayPort->mReflexiveCandidate) {
                  removeCandidate(relayPort->mReflexiveCandidate);
                  relayPort->mReflexiveCandidate.reset();
                }
                // NOTE: not going to do a shutdown because we want to leave TURN server in failed state to prevent reattempted connections
                break;
              }
            }

            if (!ready) continue;

            IPAddress relayIP = relayPort->mTURNSocket->getRelayedIP();
            IPAddress mappedIP = relayPort->mTURNSocket->getReflectedIP();

            if (relayPort->mReflexiveCandidate) {
              ZS_LOG_TRACE(log("already have reflexive candidate") + relayPort->mReflexiveCandidate->toDebug())
              addCandidate(*(hostPort->mHostData), hostPort->mBoundUDPIP, relayPort->mReflexiveCandidate);
            }

            if (relayPort->mRelayCandidate) {
              ZS_LOG_TRACE(log("already have candidate") + relayPort->mRelayCandidate->toDebug())
              addCandidate(*(hostPort->mHostData), hostPort->mBoundUDPIP, relayPort->mRelayCandidate);
            }

            if (relayIP.isAddressEmpty()) {
              ZS_LOG_WARNING(Debug, log("failed to obtain server relay address") + hostPort->toDebug() + server.toDebug())
              continue;
            }

            if (mappedIP.isAddressEmpty()) {
              ZS_LOG_WARNING(Debug, log("failed to obtain server relay mapped address") + hostPort->toDebug() + server.toDebug())
              continue;
            }

            if (relayPort->mRelayCandidate) {
              if (relayPort->mRelayCandidate->ip() != relayIP) {
                ZS_LOG_WARNING(Debug, log("relay address has changed") + hostPort->toDebug() + server.toDebug() + ZS_PARAM("new ip", relayIP.string()) + relayPort->mRelayCandidate->toDebug())
                removeCandidate(relayPort->mRelayCandidate);
                relayPort->mRelayCandidate.reset();
              }
            }
            if (relayPort->mReflexiveCandidate) {
              if (relayPort->mReflexiveCandidate->ip() != mappedIP) {
                ZS_LOG_WARNING(Debug, log("relay reflexive address has changed") + hostPort->toDebug() + server.toDebug() + ZS_PARAM("new ip", mappedIP.string()) + relayPort->mReflexiveCandidate->toDebug())
                removeCandidate(relayPort->mReflexiveCandidate);
                relayPort->mReflexiveCandidate.reset();
              }
            }

            if (!relayPort->mRelayCandidate) {
              ZS_EVENTING_4(
                            x, i, Debug, IceGathererRelayPortFoundIP, ol, IceGatherer, Cancel,
                            puid, id, mID,
                            puid, replayPortId, relayPort->mID,
                            string, candidateType, IICETypes::toString(IICETypes::CandidateType_Relay),
                            string, ip, relayIP.string()
                            );
              relayPort->mRelayCandidate = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Relay, hostPort->mBoundUDPIP, hostPort->mBoundUDPIP, relayIP, server);
              ZS_LOG_DEBUG(log("found relay candidate") + ZS_PARAM("relay", relayPort->mRelayCandidate->toDebug()));
            }
            if (!relayPort->mReflexiveCandidate) {
              ZS_EVENTING_4(
                            x, i, Debug, IceGathererRelayPortFoundIP, ol, IceGatherer, Cancel,
                            puid, id, mID,
                            puid, replayPortId, relayPort->mID,
                            string, candidateType, IICETypes::toString(IICETypes::CandidateType_Srflex),
                            string, ip, mappedIP.string()
                            );
              relayPort->mReflexiveCandidate = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Srflex, hostPort->mBoundUDPIP, hostPort->mBoundUDPIP, mappedIP, server);
              ZS_LOG_DEBUG(log("found relayed reflexive candidate") + ZS_PARAM("reflexive", relayPort->mReflexiveCandidate->toDebug()));
            }
            continue;
          }
        }

        ZS_LOG_TRACE(log("scan through relay ports for servers no longer in use"))

        for (auto iterRelay_doNotUse = hostPort->mRelayPorts.begin(); iterRelay_doNotUse != hostPort->mRelayPorts.end(); ) {
          auto currentRelay = iterRelay_doNotUse;
          ++iterRelay_doNotUse;

          auto relayPort = (*currentRelay);
          String hash = relayPort->mServer.hash();

          auto found = foundServers.find(hash);
          if (found != foundServers.end()) continue;

          ZS_LOG_DEBUG(log("relay server no longer required (thus shutting down)") + relayPort->mServer.toDebug())

          shutdown(relayPort, hostPort);
        }
        
        if (hostPortSetup) {
          hostPort->mRelayOptionsHash = mOptionsHash;
        }
      }

      if (allSetup) {
        ZS_LOG_DEBUG(log("all relay candidates are prepared (or failed)"))
        mLastRelayHostsHash = mHostsHash;
      }
      
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepTearDownRelay()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if ((shouldKeepWarm()) ||
          (shouldWarmUpAfterInterfaceBinding())) {
        ZS_LOG_TRACE(log("no relay candidates should be torn down at this time (being kept warm)"))
        return true;
      }

      if (mHostsHash == mLastRelayHostsHash) {
        ZS_LOG_TRACE(log("hosts have not changed (thus no need to tear down relay ports)"))
        return true;
      }

      bool allDone = true;

      Time now = zsLib::now();

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        bool allHostDone = true;

        for (auto iterRelay_doNotUse = hostPort->mRelayPorts.begin(); iterRelay_doNotUse != hostPort->mRelayPorts.end(); ) {
          auto currentRelay = iterRelay_doNotUse;
          ++iterRelay_doNotUse;

          auto relayPort = (*currentRelay);
          if (Time() == relayPort->mLastActivity) goto shutdown_relay;
          if (relayPort->mLastActivity + mRelayInactivityTime <= now) goto shutdown_relay;

          if (!relayPort->mInactivityTimer) {
            Time fireAt = relayPort->mLastActivity + mRelayInactivityTime;
            relayPort->mInactivityTimer = ITimer::create(mThisWeak.lock(), fireAt);
            mRelayInactivityTimers[relayPort->mInactivityTimer] = HostAndRelayPortPair(hostPort, relayPort);
            ZS_LOG_TRACE(log("setup relay inactivity timeout") + ZS_PARAMIZE(fireAt) + relayPort->toDebug())
          }
          goto wait_until_inactive;

        wait_until_inactive:
          {
            ZS_LOG_TRACE(log("relay port still active thus cannot shutdown") + relayPort->toDebug())
            allHostDone = allDone = false;
            continue;
          }

        shutdown_relay:
          {
            ZS_LOG_DEBUG(log("relay port no longer being kept warm (and is currently inactive)") + relayPort->toDebug())
            shutdown(relayPort, hostPort);
          }
        }

        if (allHostDone) {
          hostPort->mRelayPorts.clear();
        }
      }

      if (allDone) {
        ZS_LOG_DEBUG(log("all reflexive candidates are torn down (no longer being kept warm)"))
        mLastRelayHostsHash = mHostsHash;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCleanPendingShutdownTURNSockets()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if (mShutdownTURNSockets.size() < 1) {
        ZS_LOG_TRACE(log("no turn sockets pending shutdown"))
        return true;
      }

      for (auto iterShutdown_doNotUse = mShutdownTURNSockets.begin(); iterShutdown_doNotUse != mShutdownTURNSockets.end(); )
      {
        auto currentShutdown = iterShutdown_doNotUse;
        ++iterShutdown_doNotUse;

        auto turnSocket = (*currentShutdown).first;
        auto hostPort = (*currentShutdown).second.first;

        if (UseTURNSocket::TURNSocketState_Shutdown == turnSocket->getState()) goto remove_turn_socket;

        // check to see if it's still possible for the turn socket ot shutdown
        if (!hostPort->mBoundUDPSocket) {
          ZS_LOG_WARNING(Debug, log("performing hard shutdown on turn socket") + ZS_PARAM("turn socket", turnSocket->getID()) + hostPort->toDebug())
          goto remove_turn_socket;
        }
        goto still_pending;

      remove_turn_socket:
        {
          ZS_LOG_TRACE(log("removing shutdown turn socket") + ZS_PARAM("turn socket", turnSocket->getID()) + hostPort->toDebug())
          mShutdownTURNSockets.erase(currentShutdown);
          continue;
        }

      still_pending:
        {
          ZS_LOG_DEBUG(log("waiting for turn socket to shutdown") + ZS_PARAM("turn socket", turnSocket->getID()) + hostPort->toDebug())
          continue;
        }
      }

      if (mShutdownTURNSockets.size() < 1) {
        ZS_LOG_DEBUG(log("all turn sockets are not shutdown"))
      }
      
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCheckIfReady()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererStep, ol, IceGatherer, Step, puid, id, mID);

      if ((isComplete()) &&
          (!mOptions.mContinuousGathering)) {
        ZS_LOG_TRACE(log("gathering must be complete because it was complete before (and not in continuous gathering mode)"))
        return true;
      }

      if (mHostsHash != mLastFixedHostPortsHostsHash) {
        ZS_LOG_TRACE(log("still waiting to fix host ports"))
        return false;
      }

      if (mHostsHash != mLastBoundHostPortsHostHash) {
        ZS_LOG_TRACE(log("still waiting to bind host ports"))
        return false;
      }

      if (mHostsHash != mLastReflexiveHostsHash) {
        ZS_LOG_TRACE(log("still waiting for reflexive ports to gather"))
        return false;
      }

      if (mHostsHash != mLastRelayHostsHash) {
        ZS_LOG_TRACE(log("still waiting for relay ports to gather"))
        return false;
      }

      ZS_LOG_TRACE(log("gathering appears to be complete"))
      return true;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, IceGathererCancel, ol, IceGatherer, Cancel, puid, id, mID);

      if (isShutdown()) {
        ZS_LOG_TRACE(log("already shutdown"))
        return;
      }

      ZS_LOG_DEBUG(log("cancel called"))

      setState(InternalState_ShuttingDown);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      // scope: clear out pending queries
      {
        for (auto iter_doNotUse = mResolveHostIPQueries.begin(); iter_doNotUse != mResolveHostIPQueries.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto query = (*current).second;

          if (query.mQuery) {
            query.mQuery->cancel();
            query.mQuery.reset();
          }
        }

        mResolveHostIPQueries.clear();
      }

      if (mGracefulShutdownReference) {
        for (auto iterHost_doNotUse = mHostPorts.begin(); iterHost_doNotUse != mHostPorts.end(); ) {
          auto currentHost = iterHost_doNotUse;
          ++iterHost_doNotUse;

          auto hostPort = (*currentHost).second;
          for (auto iterRelay_doNotUse = hostPort->mRelayPorts.begin(); iterRelay_doNotUse != hostPort->mRelayPorts.end(); ) {
            auto currentRelay = iterRelay_doNotUse;
            ++iterRelay_doNotUse;

            auto relayPort = (*currentRelay);
            shutdown(relayPort, hostPort);
          }
        }

        stepCleanPendingShutdownTURNSockets();

        if (mShutdownTURNSockets.size() > 0) {
          ZS_LOG_DEBUG(log("still waiting for some turn sockets to shutdown") + ZS_PARAM("total", mShutdownTURNSockets.size()))
          return;
        }
      }

      ZS_LOG_TRACE(log("cancel complete"))

      setState(InternalState_Shutdown);

      // scope: clear out host ports
      {
        for (auto iter_doNotUse = mHostPorts.begin(); iter_doNotUse != mHostPorts.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto hostPort = (*current).second;
          shutdown(hostPort);
        }
      }

      // scope: remote all routes
      {
        for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end();)
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto route = (*current).second;
          removeRoute(route->mRouterRoute);
        }

        mRoutes.clear();
      }

      {
        for (auto iter_doNotUse = mInstalledTransports.begin(); iter_doNotUse != mInstalledTransports.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto installed = (*current).second;

          auto transport = installed->mTransport.lock();
          if (!transport) continue;

          removeTransport(*ICETransport::convert(transport));
        }

        mInstalledTransports.clear();
      }

      mInterfaceMappings.clear();

      if (mRecheckIPsTimer) {
        mRecheckIPsTimer->cancel();
        mRecheckIPsTimer.reset();
      }

      mPendingHostIPs.clear();
      mResolvedHostIPs.clear();
      mResolveHostIPQueries.clear();

      mHostPortSockets.clear();
      mSTUNDiscoveries.clear();
      mTURNSockets.clear();
      mShutdownTURNSockets.clear();

      mLastLocalPreference.clear();

      mNotifiedCandidates.clear();
      mLocalCandidates.clear();

      if (mWarmUpAterNewInterfaceBindingTimer) {
        mWarmUpAterNewInterfaceBindingTimer->cancel();
        mWarmUpAterNewInterfaceBindingTimer.reset();
      }

      mReflexiveInactivityTimers.clear();
      mRelayInactivityTimers.clear();

      mTCPPorts.clear();
      mTCPCandidateToTCPPorts.clear();

      if (mCleanUpBufferingTimer) {
        mCleanUpBufferingTimer->cancel();
        mCleanUpBufferingTimer.reset();
      }
      mBufferedPackets.clear();

      mQuickSearchRoutes.clear();
      mRoutes.clear();
      if (mCleanUnusedRoutesTimer) {
        mCleanUnusedRoutesTimer->cancel();
        mCleanUnusedRoutesTimer.reset();
      }

      mInstalledTransports.clear();
      mPendingTransports.clear();

      mSubscriptions.clear();

      mGracefulShutdownReference.reset();

      ZS_LOG_DETAIL(log("shutdown complete"))
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::setState(InternalStates state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DEBUG(log("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)));

      auto oldState = toState(mCurrentState);
      auto newState = toState(state);

      mCurrentState = state;

      ZS_EVENTING_2(
                    x, i, Debug, IceGathererStateChangedEvent, ol, IceGatherer, StateEvent,
                    puid, id, mID,
                    string, state, toString(mCurrentState)
                    );

      auto pThis = mThisWeak.lock();
      if ((pThis) &&
          (oldState != newState)) {

        if (IICEGatherer::State_Complete == newState) {
          CandidateCompletePtr complete(make_shared<CandidateComplete>());
          complete->mComponent = mComponent;
          mSubscriptions.delegate()->onICEGathererLocalCandidateComplete(pThis, complete);
        }

        ZS_LOG_TRACE(log("reporting state change to delegates") + ZS_PARAM("new state", IICEGathererTypes::toString(newState)) + ZS_PARAM("old state", IICEGathererTypes::toString(oldState)))
        mSubscriptions.delegate()->onICEGathererStateChange(pThis, newState);
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::setError(WORD error, const char *reason)
    {
      if (0 == error) return;
      if (0 != mLastError) return;

      mLastError = error;
      mLastErrorReason = String(reason);
      if (mLastErrorReason.isEmpty()) mLastErrorReason = UseHTTP::toString(UseHTTP::toStatusCode(mLastError));

      ZS_EVENTING_3(
                    x, e, Debug, IceGathererErrorEvent, ol, IceGatherer, ErrorEvent,
                    puid, id, mID,
                    word, error, mLastError,
                    string, reason, mLastErrorReason
                    );

      ZS_LOG_WARNING(Detail, log("error set") + ZS_PARAMIZE(mLastError) + ZS_PARAMIZE(mLastErrorReason))

      auto pThis = mThisWeak.lock();
      if (pThis) {
        //mSubscriptions.delegate()->onICEGathererError(mThisWeak.lock(), mLastError, mLastErrorReason);
      }
    }
    
    //-------------------------------------------------------------------------
    bool ICEGatherer::hasSTUNServers()
    {
      if (mOptionsHash == mHasSTUNServersOptionsHash) {
        return mHasSTUNServers;
      }

      mHasSTUNServersOptionsHash = mOptionsHash;
      mHasSTUNServers = false;

      for (auto iter = mOptions.mICEServers.begin(); iter != mOptions.mICEServers.end(); ++iter) {
        auto server = (*iter);
        if (isServerType(server, "stun:")) {
          ZS_LOG_TRACE(log("found stun server") + server.toDebug())
          mHasSTUNServers = true;
          return mHasSTUNServers;
        }
      }
      return mHasSTUNServers;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::hasTURNServers()
    {
      if (mOptionsHash == mHasTURNServersOptionsHash) {
        return mHasTURNServers;
      }

      mHasTURNServersOptionsHash = mOptionsHash;
      mHasTURNServers = false;

      for (auto iter = mOptions.mICEServers.begin(); iter != mOptions.mICEServers.end(); ++iter) {
        auto server = (*iter);
        if (isServerType(server, "turn:")) {
          ZS_LOG_TRACE(log("found turn server") + server.toDebug())
          mHasTURNServers = true;
          return mHasTURNServers;
        }
      }
      return mHasTURNServers;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::isServerType(
                                   const Server &server,
                                   const char *urlPrefix
                                   ) const
    {
      size_t length = strlen(urlPrefix);

      if (server.mURLs.size() < 1) {
        ZS_LOG_INSANE(log("server is not of url search type (no URLs found)") + server.toDebug() + ZS_PARAMIZE(urlPrefix))
        return false;
      }

      for (auto iterURLs = server.mURLs.begin(); iterURLs != server.mURLs.end(); ++iterURLs) {
        const String &url = (*iterURLs);
        if (0 != url.compare(0, length, urlPrefix)) {
          ZS_LOG_INSANE(log("server is not of url search type") + server.toDebug() + ZS_PARAMIZE(url) + ZS_PARAMIZE(urlPrefix))
          return false;
        }

        ZS_LOG_INSANE(log("found server of url search type") + server.toDebug() + ZS_PARAMIZE(url) + ZS_PARAMIZE(urlPrefix))
      }

      return true;
    }

    //-------------------------------------------------------------------------
    IICEGathererTypes::StringList ICEGatherer::createDNSLookupString(
                                                                     const Server &server,
                                                                     const char *urlPrefix
                                                                     ) const
    {
      size_t length = strlen(urlPrefix);

      StringList result;

      if (server.mURLs.size() < 1) {
        ZS_LOG_INSANE(log("server is not of url search type (no URLs found)") + server.toDebug() + ZS_PARAMIZE(urlPrefix));
        return result;
      }

      for (auto iterURLs = server.mURLs.begin(); iterURLs != server.mURLs.end(); ++iterURLs) {
        const String &url = (*iterURLs);
        if (0 != url.compare(0, length, urlPrefix)) {
          ZS_LOG_INSANE(log("server is not of url search type") + server.toDebug() + ZS_PARAMIZE(url) + ZS_PARAMIZE(urlPrefix));
          continue;
        }

        ZS_LOG_INSANE(log("found server of url search type") + server.toDebug() + ZS_PARAMIZE(url) + ZS_PARAMIZE(urlPrefix));

        result.push_back(url);
      }

      return result;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::needsHostPort(HostIPSorter::DataPtr hostData)
    {
      auto policy = hostData->mFilterPolicy;
      if (hostData->mIP.isIPv4()) {
        if (0 == (FilterPolicy_NoIPv4Host & policy)) {
          ZS_LOG_TRACE(log("host port needed because peer host ports are not filtered"))
          return true;
        }
        if (0 == (FilterPolicy_NoIPv4Prflx & policy)) {
          ZS_LOG_TRACE(log("host port needed because peer reflexive ports are not filtered"))
          return true;
        }
        if (0 == (FilterPolicy_NoIPv4Srflx & policy)) {
          if (hasSTUNServers()) {
            ZS_LOG_TRACE(log("host port needed because server reflexive ports are not filtered"))
            return true;
          }
        }
        if (0 == (FilterPolicy_NoIPv4Srflx & policy)) {
          if (hasTURNServers()) {
            ZS_LOG_TRACE(log("host port needed because relay ports are not filtered"))
            return true;
          }
        }
        ZS_LOG_TRACE(log("IPv4 host is being filtered") + hostData->toDebug())
        return false;
      }
      if (hostData->mIP.isIPv6())  {
        if ((hostData->mIP.isIPv4Mapped()) ||
            (hostData->mIP.isIPv4Compatible()) ||
            (hostData->mIP.isIPv46to4()) ||
            (hostData->mIP.isTeredoTunnel()) ||
            (hostData->mInterfaceType == InterfaceType_Tunnel)) {
          if (0 != (FilterPolicy_NoIPv6Tunnel & policy)) {
            ZS_LOG_TRACE(log("host port is not needed because peer host does not allow IPv6 tunnel address"))
            return false;
          }
        }
        if (!hostData->mIsTemporaryIP) {
          if (0 != (FilterPolicy_NoIPv6Permanent & policy)) {
            ZS_LOG_TRACE(log("host port is not needed because permanent IP addresses are not allowed"))
            return false;
          }
        }
        if (0 == (FilterPolicy_NoIPv6Host & policy)) {
          ZS_LOG_TRACE(log("host port needed because peer host ports are not filtered"))
          return true;
        }
        if (0 == (FilterPolicy_NoIPv6Prflx & policy)) {
          ZS_LOG_TRACE(log("host port needed because peer reflexive ports are not filtered"))
          return true;
        }
        if (0 == (FilterPolicy_NoIPv6Srflx & policy)) {
          if (hasSTUNServers()) {
            ZS_LOG_TRACE(log("host port needed because server reflexive ports are not filtered"))
            return true;
          }
        }
        if (0 == (FilterPolicy_NoIPv6Srflx & policy)) {
          if (hasTURNServers()) {
            ZS_LOG_TRACE(log("host port needed because relay ports are not filtered"))
            return true;
          }
        }
        ZS_LOG_TRACE(log("IPv4 host is being filtered") + hostData->toDebug())
        return false;
      }
      
      return false;
    }
    
    //-------------------------------------------------------------------------
    void ICEGatherer::shutdown(HostPortPtr hostPort)
    {
      if (!hostPort) return;

      ZS_LOG_TRACE(log("shutting down host port") + hostPort->toDebug())

      // scope: shutdown reflexive ports
      {
        for (auto iter_doNotUse = hostPort->mReflexivePorts.begin(); iter_doNotUse != hostPort->mReflexivePorts.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto reflexivePort = (*current);
          shutdown(reflexivePort, hostPort);
        }
        hostPort->mReflexivePorts.clear();
      }

      // scope: shutdown relay ports
      {
        for (auto iter_doNotUse = hostPort->mRelayPorts.begin(); iter_doNotUse != hostPort->mRelayPorts.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto relayPort = (*current);
          shutdown(relayPort, hostPort);
        }
        hostPort->mRelayPorts.clear();
      }

      // scope: shutdown TCP ports
      {
        for (auto iter_doNotuse = hostPort->mTCPPorts.begin(); iter_doNotuse != hostPort->mTCPPorts.end(); )
        {
          auto current = iter_doNotuse;
          ++iter_doNotuse;

          auto tcpPort = (*current).second.second;

          shutdown(tcpPort, hostPort);
        }

        hostPort->mTCPPorts.clear();
      }

      removeCandidate(hostPort->mCandidateUDP);
      removeCandidate(hostPort->mCandidateTCPPassive);
      removeCandidate(hostPort->mCandidateTCPActive);

      hostPort->mCandidateUDP.reset();
      hostPort->mCandidateTCPPassive.reset();
      hostPort->mCandidateTCPActive.reset();

      if (hostPort->mBoundUDPSocket) {
        auto found = mHostPortSockets.find(hostPort->mBoundUDPSocket);
        if (found != mHostPortSockets.end()) {
          mHostPortSockets.erase(found);
        }
        try {
          hostPort->mBoundUDPSocket->close();
        } catch(Socket::Exceptions::Unspecified &error) {
          ZS_LOG_ERROR(Detail, log("failed to close udp socket") + ZS_PARAM("error", error.errorCode()))
        }
        hostPort->mBoundUDPSocket.reset();
      }

      if (hostPort->mBoundTCPSocket) {
        auto found = mHostPortSockets.find(hostPort->mBoundTCPSocket);
        if (found != mHostPortSockets.end()) {
          mHostPortSockets.erase(found);
        }
        try {
          hostPort->mBoundTCPSocket->close();
        } catch(Socket::Exceptions::Unspecified &error) {
          ZS_LOG_ERROR(Detail, log("failed to close tcp socket") + ZS_PARAM("error", error.errorCode()))
        }
        hostPort->mBoundTCPSocket.reset();
      }

      // scope: remove from host ports mapping
      {
        for (auto iter_doNotUse = mHostPorts.begin(); iter_doNotUse != mHostPorts.end();)
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto compareHostPort = (*current).second;
          if (compareHostPort != hostPort) continue;

          ZS_EVENTING_3(
                        x, i, Debug, IceGathererHostPortDestroy, ol, IceGatherer, Stop,
                        puid, id, mID,
                        puid, hostPortId, hostPort->mID,
                        string, hostPortIp, hostPort->mHostData->mIP.string()
                        );

          ZS_LOG_TRACE(log("found host port to remote") + compareHostPort->toDebug())
          mHostPorts.erase(current);
          break;
        }
      }

      for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (route->mHostPort == hostPort) {
          ZS_LOG_TRACE(log("route is mapped to closed host port (thus route must be closed)") + route->toDebug())
          removeRoute(route->mRouterRoute);
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::shutdown(
                               ReflexivePortPtr reflexivePort,
                               HostPortPtr ownerHostPort
                               )
    {
      if (!reflexivePort) return;

      ZS_EVENTING_3(
                    x, i, Debug, IceGathererReflexivePortDestroy, ol, IceGatherer, Stop,
                    puid, id, mID,
                    puid, reflexivePortId, reflexivePort->mID,
                    string, firstServerUrl, reflexivePort->mServer.mURLs.size() > 0 ? reflexivePort->mServer.mURLs.front() : String()
                    );

      ZS_LOG_TRACE(log("shutting down reflexive port") + reflexivePort->toDebug())

      // scope: remote from host port
      {
        for (auto iter_doNotUse = ownerHostPort->mReflexivePorts.begin(); iter_doNotUse != ownerHostPort->mReflexivePorts.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto compareReflexivePort = (*current);
          if (compareReflexivePort != reflexivePort) continue;

          ZS_LOG_TRACE(log("found relay port to remove from host port") + compareReflexivePort->toDebug())
          ownerHostPort->mReflexivePorts.erase(current);
          break;
        }
      }

      if (reflexivePort->mInactivityTimer) {
        auto found = mReflexiveInactivityTimers.find(reflexivePort->mInactivityTimer);
        if (found != mReflexiveInactivityTimers.end()) {
          mReflexiveInactivityTimers.erase(found);
        }
        reflexivePort->mInactivityTimer->cancel();
        reflexivePort->mInactivityTimer.reset();
      }

      if (reflexivePort->mSTUNDiscovery) {
        auto found = mSTUNDiscoveries.find(reflexivePort->mSTUNDiscovery);
        if (found != mSTUNDiscoveries.end()) {
          mSTUNDiscoveries.erase(found);
        }
        reflexivePort->mSTUNDiscovery->cancel();
        reflexivePort->mSTUNDiscovery.reset();
      }

      removeCandidate(reflexivePort->mCandidate);
      reflexivePort->mCandidate.reset();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::shutdown(
                               RelayPortPtr relayPort,
                               HostPortPtr ownerHostPort
                               )
    {
      if (!relayPort) return;

      ZS_THROW_INVALID_ARGUMENT_IF(!ownerHostPort);

      ZS_EVENTING_3(
                    x, i, Debug, IceGathererRelayPortDestroy, ol, IceGatherer, Stop,
                    puid, id, mID,
                    puid, relayPort, relayPort->mID,
                    string, firstServerUrl, relayPort->mServer.mURLs.size() > 0 ? relayPort->mServer.mURLs.front() : String()
                    );

      ZS_LOG_TRACE(log("shutting down relay port") + relayPort->toDebug())

      // scope: remote from host port
      {
        for (auto iter_doNotUse = ownerHostPort->mRelayPorts.begin(); iter_doNotUse != ownerHostPort->mRelayPorts.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto compareRelayPort = (*current);
          if (compareRelayPort != relayPort) continue;

          ZS_LOG_TRACE(log("found relay port to remove from host port") + compareRelayPort->toDebug())
          ownerHostPort->mRelayPorts.erase(current);
          break;
        }
      }

      if (relayPort->mTURNSocket) {
        auto found = mTURNSockets.find(relayPort->mTURNSocket);
        if (found != mTURNSockets.end()) {
          mTURNSockets.erase(found);
        }
        relayPort->mTURNSocket->shutdown();

        mShutdownTURNSockets[relayPort->mTURNSocket] = HostAndRelayPortPair(ownerHostPort, relayPort);
        relayPort->mTURNSocket.reset();
      }
      if (relayPort->mInactivityTimer) {
        auto found = mRelayInactivityTimers.find(relayPort->mInactivityTimer);
        if (found != mRelayInactivityTimers.end()) {
          mRelayInactivityTimers.erase(found);
        }
        relayPort->mInactivityTimer->cancel();
        relayPort->mInactivityTimer.reset();
      }

      removeCandidate(relayPort->mReflexiveCandidate);
      removeCandidate(relayPort->mRelayCandidate);

      relayPort->mRelayCandidate.reset();
      relayPort->mReflexiveCandidate.reset();

      if (!relayPort->mServerResponseIP.isAddressEmpty()) {
        auto found = ownerHostPort->mIPToRelayPortMapping.find(relayPort->mServerResponseIP);
        if (found != ownerHostPort->mIPToRelayPortMapping.end()) {
          ownerHostPort->mIPToRelayPortMapping.erase(found);
        }
      }

      for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (route->mRelayPort == relayPort) {
          ZS_LOG_TRACE(log("route is mapped to closed relay port (thus route must be closed)") + route->toDebug())
          removeRoute(route->mRouterRoute);
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::shutdown(
                               TCPPortPtr tcpPort,
                               HostPortPtr ownerHostPort
                               )
    {
      if (!tcpPort) return;

      ZS_LOG_TRACE(log("shutting down tcp port") + tcpPort->toDebug());

      ZS_EVENTING_3(
                    x, i, Debug, IceGathererTcpPortDestroy, ol, IceGatherer, Stop,
                    puid, id, mID,
                    puid, tcpPortId, tcpPort->mID,
                    string, remoteIp, tcpPort->mRemoteIP.string()
                    );

      if (tcpPort->mCandidate) {
        auto found = mTCPCandidateToTCPPorts.find(tcpPort->mCandidate);
        if (found != mTCPCandidateToTCPPorts.end()) {
          mTCPCandidateToTCPPorts.erase(found);
        }
      }

      if (tcpPort->mSocket) {
        // remove from host port mapping
        {
          auto found = ownerHostPort->mTCPPorts.find(tcpPort->mSocket);
          if (found != ownerHostPort->mTCPPorts.end()) {
            ownerHostPort->mTCPPorts.erase(found);
          }
        }

        // remove from tcp port mapping
        {
          auto found = mTCPPorts.find(tcpPort->mSocket);
          if (found != mTCPPorts.end()) {
            mTCPPorts.erase(found);
          }
        }

        try {
          tcpPort->mSocket->close();
        } catch (Socket::Exceptions::Unspecified &error) {
          ZS_LOG_ERROR(Detail, log("failed to close tcp port socket") + ZS_PARAM("error", error.errorCode()))
        }

        tcpPort->mSocket.reset();
      }

      tcpPort->mIncomingBuffer.Clear();
      tcpPort->mOutgoingBuffer.Clear();

      for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (route->mTCPPort == tcpPort) {
          ZS_LOG_TRACE(log("route is mapped to closed tcp socket (thus route must be closed)") + route->toDebug())
          removeRoute(route->mRouterRoute);
        }
      }
    }

    //-------------------------------------------------------------------------
    SocketPtr ICEGatherer::bind(
                                bool firstAttempt,
                                IPAddress &ioBindIP,
                                IICETypes::Protocols protocol
                                )
    {
      ZS_LOG_DEBUG(log("attempting to bind to IP") + ZS_PARAM("ip", ioBindIP.string()));

      auto createFamily = (ioBindIP.isIPv6() ? Socket::Create::IPv6 : Socket::Create::IPv4);

      SocketPtr socket;

      try {
        switch (protocol) {
          case IICETypes::Protocol_UDP: socket = Socket::createUDP(createFamily); break;
          case IICETypes::Protocol_TCP: socket = Socket::createTCP(createFamily); break;
        }

        if (0 != mDefaultPort) {
          if (firstAttempt) {
            // only bind using the default port in the first attempt
            ioBindIP.setPort(mDefaultPort);
            ZS_LOG_DEBUG(log("will attempt to bind to default port") + ZS_PARAM("ip address", ioBindIP.string()))
          } else {
            ZS_LOG_WARNING(Debug, log("will not attempt to rebind to default port") + ZS_PARAM("ip address", ioBindIP.string()))
          }
        }

        socket->bind(ioBindIP);
        socket->setBlocking(false);

        try {
#ifndef __QNX__
          socket->setOptionFlag(Socket::SetOptionFlag::IgnoreSigPipe, true);
#endif //ndef __QNX__
        } catch(Socket::Exceptions::UnsupportedSocketOption &) {
        }

        IPAddress local = socket->getLocalAddress();

        socket->setDelegate(mThisWeak.lock());

        WORD bindPort = local.getPort();
        ioBindIP.setPort(bindPort);
        if (0 == mDefaultPort) {
          mDefaultPort = bindPort;
          ZS_LOG_TRACE(log("selected default bind port") + ZS_PARAMIZE(mDefaultPort))
        }
        ZS_THROW_CUSTOM_PROPERTIES_1_IF(Socket::Exceptions::Unspecified, 0 == bindPort, 0)
      } catch(Socket::Exceptions::Unspecified &error) {
        ZS_LOG_ERROR(Detail, log("bind error") + ZS_PARAM("error", error.errorCode()))
        socket.reset();
        goto bind_failure;
      }

      switch (protocol) {
        case IICETypes::Protocol_UDP: break;
        case IICETypes::Protocol_TCP: {
          if (mGatherPassiveTCP) {
            try {
              ZS_LOG_DEBUG(log("attemping to listen for incoming socket connections"))
              socket->listen();
            } catch(Socket::Exceptions::Unspecified &error) {
              ZS_LOG_ERROR(Detail, log("listen error") + ZS_PARAM("error", error.errorCode()))
              socket.reset();
              goto bind_failure;
            }
          }
          break;
        }
      }

      goto bind_success;

    bind_success:
      {
        ZS_LOG_DEBUG(log("bind successful") + ZS_PARAM("bind ip", ioBindIP.string()))
        return socket;
      }

    bind_failure:
      {
        ZS_LOG_WARNING(Debug, log("bind failure") + ZS_PARAM("bind ip", ioBindIP.string()))
      }
      return SocketPtr();
    }
    
    //-------------------------------------------------------------------------
    IICETypes::CandidatePtr ICEGatherer::createCandidate(
                                                         HostIPSorter::DataPtr hostData,
                                                         IICETypes::CandidateTypes candidateType,
                                                         const IPAddress &boundIP,
                                                         IICETypes::Protocols protocol,
                                                         IICETypes::TCPCandidateTypes tcpType
                                                         )
    {
      CandidatePtr candidate(make_shared<Candidate>());

      AddressFamilies family = (boundIP.isIPv4() ? AddressFamily_IPv4 : AddressFamily_IPv6);

      candidate->mInterfaceType = toString(hostData->mInterfaceType);

      candidate->mProtocol = protocol;
      candidate->mTCPType = tcpType;
      candidate->mIP = boundIP.string(false);
      candidate->mPort = boundIP.getPort();
      candidate->mCandidateType = candidateType;
      candidate->mFoundation = candidate->foundation();
      candidate->mComponent = mComponent;

      WORD localPreference = 0;

      auto found = mLastLocalPreference.find(candidate->mFoundation);
      if (found != mLastLocalPreference.end()) {
        WORD &preference = (*found).second;
        ++preference;
        localPreference = preference;
      } else {
        mLastLocalPreference[candidate->mFoundation] = localPreference;
      }

      candidate->mPriority = (mPreferences[PreferenceType_Priority].mCandidateTypePreferences[candidateType]) |
                             (mPreferences[PreferenceType_Priority].mProtocolTypePreferences[protocol]) |
                             (mPreferences[PreferenceType_Priority].mInterfaceTypePreferences[hostData->mInterfaceType]) |
                             (mPreferences[PreferenceType_Priority].mAddressFamilyPreferences[family]) |
                             ((localPreference % 0xFF) << 8) |
                             (256 - mComponent);

      candidate->mUnfreezePriority = (mPreferences[PreferenceType_Unfreeze].mCandidateTypePreferences[candidateType]) |
                                     (mPreferences[PreferenceType_Unfreeze].mProtocolTypePreferences[protocol]) |
                                     (mPreferences[PreferenceType_Unfreeze].mInterfaceTypePreferences[hostData->mInterfaceType]) |
                                     (mPreferences[PreferenceType_Unfreeze].mAddressFamilyPreferences[family]) |
                                     ((localPreference % 0xFF) << 8) |
                                     (256 - mComponent);

      addCandidate(*hostData, boundIP, candidate);

      return candidate;
    }

    //-------------------------------------------------------------------------
    IICETypes::CandidatePtr ICEGatherer::createCandidate(
                                                         HostIPSorter::DataPtr hostData,
                                                         IICETypes::CandidateTypes candidateType,
                                                         const IPAddress &baseIP,
                                                         const IPAddress &relatedIP,
                                                         const IPAddress &boundIP,
                                                         const Server &server
                                                         )
    {
      IICETypes::Protocols protocol = IICETypes::Protocol_UDP;

      CandidatePtr candidate(make_shared<Candidate>());

      String serverURL;
      if (server.mURLs.size() > 0) {
        serverURL = server.mURLs.front();
      }

      AddressFamilies family = (boundIP.isIPv4() ? AddressFamily_IPv4 : AddressFamily_IPv6);

      candidate->mInterfaceType = toString(hostData->mInterfaceType);
      candidate->mProtocol = protocol;
      candidate->mIP = boundIP.string(false);
      candidate->mPort = boundIP.getPort();
      candidate->mCandidateType = candidateType;
      candidate->mRelatedAddress = relatedIP.string(false);
      candidate->mRelatedPort = relatedIP.getPort();
      candidate->mFoundation = candidate->foundation(serverURL, baseIP.string(false));
      candidate->mComponent = mComponent;

      WORD localPreference = 0;

      auto found = mLastLocalPreference.find(candidate->mFoundation);
      if (found != mLastLocalPreference.end()) {
        WORD &preference = (*found).second;
        ++preference;
        localPreference = preference;
      } else {
        mLastLocalPreference[candidate->mFoundation] = localPreference;
      }

      candidate->mPriority = (mPreferences[PreferenceType_Priority].mCandidateTypePreferences[candidateType]) |
                             (mPreferences[PreferenceType_Priority].mProtocolTypePreferences[protocol]) |
                             (mPreferences[PreferenceType_Priority].mInterfaceTypePreferences[hostData->mInterfaceType]) |
                             (mPreferences[PreferenceType_Priority].mAddressFamilyPreferences[family]) |
                             ((localPreference % 0xFF) << 8) |
                             (256 - mComponent);

      candidate->mUnfreezePriority = (mPreferences[PreferenceType_Unfreeze].mCandidateTypePreferences[candidateType]) |
                                     (mPreferences[PreferenceType_Unfreeze].mProtocolTypePreferences[protocol]) |
                                     (mPreferences[PreferenceType_Unfreeze].mInterfaceTypePreferences[hostData->mInterfaceType]) |
                                     (mPreferences[PreferenceType_Unfreeze].mAddressFamilyPreferences[family]) |
                                     ((localPreference % 0xFF) << 8) |
                                     (256 - mComponent);

      addCandidate(*hostData, boundIP, candidate);

      return candidate;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::addCandidate(
                                   const HostIPSorter::Data &hostData,
                                   const IPAddress &ip,
                                   CandidatePtr candidate
                                   )
    {
      if (!candidate) return;

      if (candidate->mCandidateType == IICETypes::CandidateType_Prflx) {
        ZS_LOG_TRACE(log("must filter out peer reflexive candidates as they do not get notified"))
        return;
      }

      if (isFiltered(hostData.mFilterPolicy, ip, *candidate)) {
        ZS_EVENTING_16(
                       x, i, Debug, IceGathererFilterCandidateEvent, ol, IceGatherer, InternalEvent,
                       puid, id, mID,
                       pointer, candidate, candidate.get(),
                       string, filterPolicy, IICEGathererTypes::toString(hostData.mFilterPolicy),
                       string, boundIp, ip.string(),
                       string, interfaceType, candidate->mInterfaceType,
                       string, foundation, candidate->mFoundation,
                       enum, component, candidate->mComponent,
                       dword, priority, candidate->mPriority,
                       dword, unfreezePriority, candidate->mUnfreezePriority,
                       string, protocol, IICETypes::toString(candidate->mProtocol),
                       string, ip, candidate->mIP,
                       word, port, candidate->mPort,
                       string, candidateType, IICETypes::toString(candidate->mCandidateType),
                       string, tcpType, IICETypes::toString(candidate->mTCPType),
                       string, relatedAddress, candidate->mRelatedAddress,
                       word, relatedPort, candidate->mRelatedPort
                       );

        ZS_LOG_TRACE(log("this candidate is filtered") + candidate->toDebug())
        return;
      }

      String localHash = candidate->hash();

      // NOTE: The uniqueness of a candidate is based on all properites minus
      // the priorities. Thus a candidate is not truly unique if the candidate
      // has all the same values but with different priority values.
      String notifyHash = candidate->hash(false);

      if (mLocalCandidates.find(localHash) != mLocalCandidates.end()) {
        ZS_LOG_TRACE(log("canadidate already added") + candidate->toDebug())
        return;
      }

      bool isUnique = false;
      if (mNotifiedCandidates.find(notifyHash) == mNotifiedCandidates.end()) {
        isUnique = true;
      }

      ZS_EVENTING_16(
                     x, i, Debug, IceGathererAddCandidateEvent, ol, IceGatherer, Event,
                     puid, id, mID,
                     pointer, candidate, candidate.get(),
                     string, localHash, localHash,
                     string, notifyHash, notifyHash,
                     string, interfaceType, candidate->mInterfaceType,
                     string, foundation, candidate->mFoundation,
                     enum, component, candidate->mComponent,
                     dword, priority, candidate->mPriority,
                     dword, unfreezePriority, candidate->mUnfreezePriority,
                     string, protocol, IICETypes::toString(candidate->mProtocol),
                     string, ip, candidate->mIP,
                     word, port, candidate->mPort,
                     string, candidateType, IICETypes::toString(candidate->mCandidateType),
                     string, tcpType, IICETypes::toString(candidate->mTCPType),
                     string, relatedAddress, candidate->mRelatedAddress,
                     word, relatedPort, candidate->mRelatedPort
                     );

      mLocalCandidates[localHash] = CandidatePair(candidate, notifyHash);
      if (!isUnique) {
        ZS_LOG_TRACE(log("similar candidate already notified (thus do not notify of this candidate)") + candidate->toDebug())
        return;
      }

      mNotifiedCandidates[notifyHash] = CandidatePair(candidate, localHash);

      if (InternalState_Ready == mCurrentState) {
        setState(InternalState_Gathering);
      }

      CandidatePtr sendCandidate(make_shared<Candidate>(*candidate));

      ZS_LOG_DEBUG(log("notify local candidate") + candidate->toDebug())
      mSubscriptions.delegate()->onICEGathererLocalCandidate(mThisWeak.lock(), sendCandidate);
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::removeCandidate(CandidatePtr candidate)
    {
      if (!candidate) return;

      String localHash = candidate->hash();
      String notifyHash = candidate->hash(false);

      auto foundLocal = mLocalCandidates.find(localHash);
      if (foundLocal == mLocalCandidates.end()) {
        ZS_LOG_WARNING(Debug, log("local candidate is not found (might have been filtered or was peer reflexive)") + candidate->toDebug())
        return;
      }

      ZS_EVENTING_16(
                     x, i, Debug, IceGathererRemoveCandidateEvent, ol, IceGatherer, Event,
                     puid, id, mID,
                     pointer, candidate, candidate.get(),
                     string, localHash, localHash,
                     string, notifyHash, notifyHash,
                     string, interfaceType, candidate->mInterfaceType,
                     string, foundation, candidate->mFoundation,
                     enum, component, candidate->mComponent,
                     dword, priority, candidate->mPriority,
                     dword, unfreezePriority, candidate->mUnfreezePriority,
                     string, protocol, IICETypes::toString(candidate->mProtocol),
                     string, ip, candidate->mIP,
                     word, port, candidate->mPort,
                     string, candidateType, IICETypes::toString(candidate->mCandidateType),
                     string, tcpType, IICETypes::toString(candidate->mTCPType),
                     string, relatedAddress, candidate->mRelatedAddress,
                     word, relatedPort, candidate->mRelatedPort
                     );

      mLocalCandidates.erase(foundLocal);

      auto foundUnique = mNotifiedCandidates.find(notifyHash);
      if (foundUnique == mNotifiedCandidates.end()) {
        ZS_LOG_WARNING(Debug, log("unique candidate is not found") + candidate->toDebug())
        return;
      }

      const String &previousLocalHash = (*foundUnique).second.second;

      if (previousLocalHash != localHash) {
        ZS_LOG_TRACE(log("candidate being removed was not notified candidate") + candidate->toDebug())
        return;
      }

      CandidatePtr previousCandidate = (*foundUnique).second.first;

      ZS_LOG_DEBUG(log("notify candidate gone") + previousCandidate->toDebug())

      CandidatePtr sendCandidate(make_shared<Candidate>(*previousCandidate));
      auto pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onICEGathererLocalCandidateGone(mThisWeak.lock(), sendCandidate);
      }

      mNotifiedCandidates.erase(foundUnique);

      for (auto iter = mLocalCandidates.begin(); iter != mLocalCandidates.end(); ++iter) {
        const String &otherNotifyHash = (*iter).second.second;
        if (otherNotifyHash == notifyHash) {
          const String &otherLocalHash = (*iter).first;
          auto otherCandidate = (*iter).second.first;

          mNotifiedCandidates[notifyHash] = CandidatePair(otherCandidate, otherLocalHash);

          if (InternalState_Ready == mCurrentState) {
            setState(InternalState_Gathering);
          }

          CandidatePtr sendCandidate(make_shared<Candidate>(*otherCandidate));

          ZS_LOG_DEBUG(log("notify replacement local candidate") + otherCandidate->toDebug())
          if (pThis) {
            mSubscriptions.delegate()->onICEGathererLocalCandidate(mThisWeak.lock(), sendCandidate);
          }
          return;
        }
      }

      ZS_LOG_TRACE(log("no replacement local candidate found"))
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::isFiltered(
                                 FilterPolicies policy,
                                 const IPAddress &ip,
                                 const Candidate &candidate
                                 ) const
    {
      switch (candidate.mCandidateType) {
        case IICETypes::CandidateType_Host:   {
          if (ip.isIPv4()) {
            if (0 != (FilterPolicy_NoIPv4Host & policy)) return true;

            if (0 != (FilterPolicy_NoIPv4Private & policy)) {
              if (ip.isPrivate()) return true;
            }
          } else {
            if (0 != (FilterPolicy_NoIPv6Host & policy)) return true;

            if (0 != (FilterPolicy_NoIPv6Private & policy)) {
              if (ip.isPrivate()) return true;
            }
          }
          break;
        }
        case IICETypes::CandidateType_Srflex: {
          if (ip.isIPv4()) {
            if (0 != (FilterPolicy_NoIPv4Srflx & policy)) return true;
          } else {
            if (0 != (FilterPolicy_NoIPv6Srflx & policy)) return true;
          }
          break;
        }
        case IICETypes::CandidateType_Prflx: {
          if (ip.isIPv4()) {
            if (0 != (FilterPolicy_NoIPv4Prflx & policy)) return true;
          } else {
            if (0 != (FilterPolicy_NoIPv6Prflx & policy)) return true;
          }
          break;
        }
        case IICETypes::CandidateType_Relay: {
          if (ip.isIPv4()) {
            if (0 != (FilterPolicy_NoIPv4Relay & policy)) return true;
          } else {
            if (0 != (FilterPolicy_NoIPv6Relay & policy)) return true;
          }
          break;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::read(
                           HostPortPtr hostPort,
                           SocketPtr socket
                           )
    {
      UseTURNSocketPtr turnSocket;
      STUNPacketPtr stunPacket;

      size_t totalRead = 0;
      IPAddress fromIP;
      BYTE readBuffer[0xFFFF] {};

      CandidatePtr localCandidate;

      {
        AutoRecursiveLock lock(*this);

        if (hostPort->mBoundUDPSocket == socket) {
          bool wouldBlock = false;
          try {
            totalRead = socket->receiveFrom(fromIP, readBuffer, sizeof(readBuffer), &wouldBlock);
          } catch(Socket::Exceptions::Unspecified &error) {
            ZS_LOG_WARNING(Debug, log("socket read error") + ZS_PARAM("socket", string(socket)) + ZS_PARAM("error", error.errorCode()))
            return false;
          }

          if (0 == totalRead) {
            if (wouldBlock) {
              ZS_LOG_INSANE(log("socket read would block") + ZS_PARAM("socket", string(socket)))
            } else {
              ZS_LOG_WARNING(Debug, log("failed to read any data from socket") + ZS_PARAM("socket", string(socket)))
            }
            return false;
          }

          ZS_EVENTING_4(
                        x, i, Trace, IceGathererUdpSocketPacketReceivedFrom, ol, IceGatherer, Receive,
                        puid, id, mID,
                        string, fromIp, fromIP.string(),
                        buffer, packet, &(readBuffer[0]),
                        size, size, totalRead
                        );

          ZS_LOG_INSANE(log("receiving incoming packet") + ZS_PARAM("from ip", fromIP.string()) + ZS_PARAM("read", totalRead) + hostPort->toDebug());

          stunPacket = STUNPacket::parseIfSTUN(&(readBuffer[0]), totalRead, mSTUNPacketParseOptions);
          fixSTUNParserOptions(stunPacket);

          // scope: check if for relay socket
          {
            auto found = hostPort->mIPToRelayPortMapping.find(fromIP);
            if (found != hostPort->mIPToRelayPortMapping.end()) {
              auto relayPort = (*found).second;
              turnSocket = relayPort->mTURNSocket;
              if (!turnSocket) {
                ZS_LOG_WARNING(Detail, log("TURN socket was not found despite mapping being found") + relayPort->toDebug());
                goto unknown_handler;
              }
              goto found_relay_port;
            }
          }

          // this is not a relay socket, see if there is a route
          localCandidate = hostPort->mCandidateUDP;
          if (!localCandidate) {
            ZS_LOG_WARNING(Trace, log("did not find local candidate"))
            goto unknown_handler;
          }
          goto handle_incoming;
        }

        if (hostPort->mBoundTCPSocket == socket) {
          if (mGatherPassiveTCP) {
            ZS_THROW_INVALID_ASSUMPTION_IF(!hostPort->mCandidateTCPPassive)
            ZS_LOG_DEBUG(log("notified of incoming TCP connection") + hostPort->toDebug())

            TCPPortPtr tcpPort(make_shared<TCPPort>());
            tcpPort->mConnected = true;

            IPAddress localAddress;

            try {
              bool wouldBlock = false;
              tcpPort->mSocket = socket->accept(tcpPort->mRemoteIP, &wouldBlock);
              if (wouldBlock) {
                ZS_LOG_INSANE(log("no more sockets to accept") + tcpPort->toDebug())
                return false;
              }
              localAddress = socket->getLocalAddress();
            } catch(Socket::Exceptions::Unspecified &error) {
              ZS_LOG_WARNING(Detail, log("failed to accept incoming TCP connection") + ZS_PARAM("error", error.errorCode()))
              return false;
            }

            ZS_EVENTING_3(
                          x, i, Trace, IceGathererTcpPortCreate, ol, IceGatherer, Start,
                          puid, id, mID,
                          puid, tcpPortId, tcpPort->mID,
                          string, remoteIp, tcpPort->mRemoteIP.string()
                          );

            // create mappings for this socket
            mTCPPorts[tcpPort->mSocket] = HostAndTCPPortPair(hostPort, tcpPort);
            hostPort->mTCPPorts[tcpPort->mSocket] = HostAndTCPPortPair(hostPort, tcpPort);

            tcpPort->mSocket->setDelegate(mThisWeak.lock());

            tcpPort->mCandidate = hostPort->mCandidateTCPPassive;

            mTCPCandidateToTCPPorts[tcpPort->mCandidate] = tcpPort;

            ZS_LOG_DEBUG(log("incoming connection ready") + hostPort->toDebug() + tcpPort->toDebug())
            return true;
          }
        }
      }

      return false;

    unknown_handler:
      {
        if (stunPacket) {
          if (ISTUNRequester::handleSTUNPacket(fromIP, stunPacket)) {
            ZS_LOG_TRACE(log("handled by stun requester") + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
            return true;
          }
        }
        return true;
      }

    found_relay_port:
      {
        ZS_EVENTING_5(
                      x, i, Trace, IceGathererUdpSocketPacketForwardingToTurnSocket, ol, IceGatherer, Deliver,
                      puid, id, mID,
                      string, fromIp, fromIP.string(),
                      bool, isStunPacket, ((bool)stunPacket),
                      buffer, packet, &(readBuffer[0]),
                      size, size, totalRead
                      );

        if (stunPacket) {
          if (ISTUNRequester::handleSTUNPacket(fromIP, stunPacket)) {
            ZS_LOG_TRACE(log("handled by stun requester") + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
            return true;
          }

          ZS_LOG_INSANE(log("forwarding stun packet to turn socket") + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
          turnSocket->handleSTUNPacket(fromIP, stunPacket);
          return true;
        }

        ZS_LOG_INSANE(log("forwarding turn channel data to turn socket") + ZS_PARAM("from ip", fromIP.string()) + ZS_PARAM("total", totalRead))
        turnSocket->handleChannelData(fromIP, &(readBuffer[0]), totalRead);
        return true;
      }

    handle_incoming:
      {
        if (stunPacket) {
          if (ISTUNRequester::handleSTUNPacket(fromIP, stunPacket)) {
            ZS_LOG_TRACE(log("handled by stun requester") + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
            return true;
          }

          ZS_LOG_INSANE(log("handling incoming stun packet") + localCandidate->toDebug() + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
          auto response = handleIncomingPacket(localCandidate, fromIP, stunPacket);
          if (response) {
            AutoRecursiveLock lock(*this);

            if (hostPort->mBoundUDPSocket) {
              auto result = sendUDPPacket(socket, hostPort->mBoundUDPIP, fromIP, *response, response->SizeInBytes());
              if (!result) {
                ZS_LOG_WARNING(Debug, log("failed to send response packet to stun request") + localCandidate->toDebug() + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
              }
            } else {
              ZS_LOG_WARNING(Debug, log("cannot send response as socket is gone") + localCandidate->toDebug() + ZS_PARAM("from ip", fromIP.string()) + stunPacket->toDebug())
            }
          }
          return true;
        }
        ZS_LOG_INSANE(log("handling incoming packet") + localCandidate->toDebug() + ZS_PARAM("from ip", fromIP.string()) + ZS_PARAM("total", totalRead))
        handleIncomingPacket(localCandidate, fromIP, &(readBuffer[0]), totalRead);
        return true;
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::read(
                           HostPort &hostPort,
                           TCPPort &tcpPort
                           )
    {
      BufferedPacketList packets;

      CandidatePtr localCandidate;
      IPAddress fromIP;

      {
        AutoRecursiveLock lock(*this);

        while (true)
        {
          if (!tcpPort.mSocket) {
            ZS_LOG_WARNING(Detail, log("cannot read closed socket") + tcpPort.toDebug());
            return;
          }

          BYTE buffer[4096] {};

          try {
            bool wouldBlock = false;
            size_t read = tcpPort.mSocket->receive(&(buffer[0]), sizeof(buffer), &wouldBlock);
            if (0 == read) goto process_packets;

            tcpPort.mIncomingBuffer.Put(&(buffer[0]), read);
          } catch(Socket::Exceptions::Unspecified &error) {
            ZS_LOG_ERROR(Detail, log("unable to receive from socket") + ZS_PARAM("error", error.errorCode()) + tcpPort.toDebug());
            goto process_packets;
          }

          while (true) {
            auto currentSize = tcpPort.mIncomingBuffer.CurrentSize();

            CryptoPP::word16 packetSize {};

            if (currentSize < sizeof(packetSize)) goto nothing_more_to_parse;

            // peak ahead to see if the entire packet has arrived
            tcpPort.mIncomingBuffer.PeekWord16(packetSize);

            if (currentSize < sizeof(packetSize) + packetSize) goto nothing_more_to_parse;

            tcpPort.mIncomingBuffer.Skip(sizeof(packetSize));

            // now have enough to extract out of buffer

            BufferedPacketPtr packet(make_shared<BufferedPacket>());
            packet->mBuffer = make_shared<SecureByteBlock>(packetSize);
            if (!localCandidate) {
              localCandidate = tcpPort.mCandidate;
              fromIP = tcpPort.mRemoteIP;
            }

            // fill packet with incoming data
            tcpPort.mIncomingBuffer.Get(*(packet->mBuffer), packetSize);

            ZS_EVENTING_4(
                          x, i, Trace, IceGathererTcpSocketPacketReceivedFrom, ol, IceGatherer, Receive,
                          puid, id, mID,
                          string, remoteIp, tcpPort.mRemoteIP.string(),
                          buffer, packet, packet->mBuffer->BytePtr(),
                          size, size, packetSize
                          );

            packet->mSTUNPacket = STUNPacket::parseIfSTUN(*(packet->mBuffer), packet->mBuffer->SizeInBytes(), mSTUNPacketParseOptions);
            fixSTUNParserOptions(packet->mSTUNPacket);

            packets.push_back(packet);
          }

        nothing_more_to_parse:
          {
            ZS_LOG_INSANE(log("nothing more to parse at this time") + ZS_PARAM("packets found", packets.size()) + tcpPort.toDebug())
          }
        }
      }

    process_packets:
      {
        for (auto iter = packets.begin(); iter != packets.end(); ++iter) {
          auto packet = (*iter);

          if (packet->mSTUNPacket) {
            if (ISTUNRequester::handleSTUNPacket(fromIP, packet->mSTUNPacket)) {
              ZS_LOG_TRACE(log("handled by stun requester") + ZS_PARAM("from ip", fromIP.string()) + packet->mSTUNPacket->toDebug())
              continue;
            }

            ZS_LOG_TRACE(log("handling incoming TCP stun packet") + packet->toDebug() + packet->mSTUNPacket->toDebug())

            auto response = handleIncomingPacket(localCandidate, fromIP, packet->mSTUNPacket);
            if (response) {
              AutoRecursiveLock lock(*this);
              if (tcpPort.mSocket) {
                ZS_LOG_TRACE(log("sending packet response by putting into TCP send queue") + tcpPort.toDebug())
                // put the buffer at the end of the queue

                CryptoPP::word16 packeSize = ((CryptoPP::word16)(response->SizeInBytes()));

                tcpPort.mOutgoingBuffer.PutWord16(packeSize);
                tcpPort.mOutgoingBuffer.Put(*response, response->SizeInBytes());
                if ((tcpPort.mConnected) &&
                    (tcpPort.mWriteReady)) {
                  ZS_LOG_INSANE(log("simulate TCP write ready to force the packet to send immediately"))
                  ISocketDelegateProxy::create(mThisWeak.lock())->onWriteReady(tcpPort.mSocket);
                }
              } else {
                ZS_LOG_WARNING(Debug, log("socket is now gone thus response cannot be sent") + tcpPort.toDebug() + ZS_PARAM("from ip", fromIP.string()) + packet->mSTUNPacket->toDebug())
              }
            }
            continue;
          }

          ZS_LOG_INSANE(log("handling incoming TCP packet") + packet->toDebug())
          handleIncomingPacket(localCandidate, fromIP, *(packet->mBuffer), packet->mBuffer->SizeInBytes());
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::write(
                            HostPort &hostPort,
                            SocketPtr socket
                            )
    {
      AutoRecursiveLock lock(*this);

      if (hostPort.mBoundUDPSocket == socket) {
        for (auto iter = hostPort.mRelayPorts.begin(); iter != hostPort.mRelayPorts.end(); ++iter) {
          auto relayPort = (*iter);
          if (!relayPort->mTURNSocket) continue;

          ZS_LOG_INSANE(log("notifying relay port of host port write ready") + relayPort->toDebug());
          relayPort->mTURNSocket->notifyWriteReady();
        }
      }

      if (hostPort.mBoundTCPSocket == socket) {
        ZS_LOG_TRACE(log("ignoring wrote ready on listen socket") + hostPort.toDebug());
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::writeIfTCPPort(SocketPtr socket)
    {
      auto found = mTCPPorts.find(socket);
      if (found == mTCPPorts.end()) return false;

      TCPPort &tcpPort = *((*found).second.second);

      ZS_EVENTING_2(
                    x, i, Trace, IceGathererInternalTcpSocketWriteReadyEvent, ol, IceGatherer, InternalEvent,
                    puid, id, mID,
                    puid, tcpPortId, tcpPort.mID
                    );

      if (!tcpPort.mSocket) {
        ZS_LOG_WARNING(Detail, log("tcp port was closed during write notification") + tcpPort.toDebug())
        return true;
      }

      tcpPort.mWriteReady = true;

      if (!tcpPort.mConnected) {
        ZS_LOG_TRACE(log("outgoing tcp port is now connected") + tcpPort.toDebug());
        tcpPort.mConnected = true;
      }

      while (true) {
        size_t currentSize = static_cast<size_t>(tcpPort.mOutgoingBuffer.CurrentSize());
        if (0 == currentSize) {
          ZS_LOG_INSANE(log("nothing more to send") + tcpPort.toDebug());
          goto finished_write;
        }

        try {
          if (currentSize > 0xFFFF + sizeof(CryptoPP::word16)) currentSize = 0xFFFF + sizeof(CryptoPP::word16);

          SecureByteBlock buffer(currentSize);
          tcpPort.mOutgoingBuffer.Peek(buffer, currentSize);

          bool wouldBlock = false;
          auto sent = tcpPort.mSocket->send(buffer, currentSize, &wouldBlock);

          if (0 == sent) {
            ZS_LOG_INSANE(log("no more room to send data") + tcpPort.toDebug());
            tcpPort.mWriteReady = false;
            goto finished_write;
          }

          ZS_EVENTING_4(
                        x, i, Trace, IceGathererTcpSocketSentOutgoing, ol, IceGatherer, Send,
                        puid, id, mID,
                        string, remoteIp, tcpPort.mRemoteIP.string(),
                        buffer, packet, buffer.BytePtr(),
                        size, size, sent
                        );

          ZS_LOG_INSANE(log("sent TCP data to remote party") + tcpPort.toDebug() + ZS_PARAM("sent", sent))

          // consume the amount sent from the pending queue (and try sending more)
          tcpPort.mOutgoingBuffer.Skip(sent);
        } catch(Socket::Exceptions::Unspecified &error) {
          ZS_LOG_ERROR(Detail, log("unable to send to socket") + ZS_PARAM("error", error.errorCode()) + tcpPort.toDebug())
          goto finished_write;
        }
      }

    finished_write: {}
      return true;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::close(
                            HostPortPtr hostPort,
                            SocketPtr socket
                            )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!hostPort)
      AutoRecursiveLock lock(*this);

      if ((hostPort->mBoundUDPSocket != socket) &&
          (hostPort->mBoundTCPSocket != socket)) {
        ZS_LOG_WARNING(Detail, log("socket was not found on host port") + hostPort->toDebug() + ZS_PARAM("socket", string(socket)))
        return;
      }

      ZS_EVENTING_4(
                    x, i, Trace, IceGathererHostPortClose, ol, IceGatherer, HostSocketClose,
                    puid, id, mID,
                    puid, hostPortId, hostPort->mID,
                    string, hostIp, hostPort->mHostData->mIP.string(),
                    string, protocolType, IICETypes::toString(hostPort->mBoundUDPSocket == socket ? IICETypes::Protocol_UDP : IICETypes::Protocol_TCP)
                    );

      CandidatePtr ingored;

      SocketPtr &hostSocket = (hostPort->mBoundUDPSocket == socket ? hostPort->mBoundUDPSocket : hostPort->mBoundTCPSocket);
      CandidatePtr &candidate1 = (hostPort->mBoundUDPSocket == socket ? hostPort->mCandidateUDP : hostPort->mCandidateTCPPassive);
      CandidatePtr &candidate2 = (hostPort->mBoundUDPSocket == socket ? ingored : hostPort->mCandidateTCPActive);

      ZS_LOG_WARNING(Detail, log("bound UDP or TCP socket unexpectedly closed") + hostPort->toDebug() + ZS_PARAM("socket", string(socket)))

      auto found = mHostPortSockets.find(socket);
      if (found != mHostPortSockets.end()) {
        mHostPortSockets.erase(found);
      }

      try {
        socket->close();
      } catch(Socket::Exceptions::Unspecified &error) {
        ZS_LOG_ERROR(Detail, log("unable to close socket") + ZS_PARAM("error", error.errorCode()) + hostPort->toDebug())
      }

      socket.reset();

      removeCandidate(candidate1);
      removeCandidate(candidate2);

      hostSocket.reset();
      candidate1.reset();
      candidate2.reset();

      hostPort->mBoundOptionsHash.clear();

      // cause rebinding to occur on port
      mLastBoundHostPortsHostHash.clear();
      mLastReflexiveHostsHash.clear();
      mLastRelayHostsHash.clear();

      // kick start to cause another binding attempt
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::close(
                            TCPPortPtr tcpPort,
                            HostPortPtr ownerHostPort
                            )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!tcpPort)
      ZS_THROW_INVALID_ARGUMENT_IF(!ownerHostPort)

      AutoRecursiveLock lock(*this);
      ZS_LOG_DEBUG(log("tcp port is closing") + tcpPort->toDebug())
      shutdown(tcpPort, ownerHostPort);
    }

    //-------------------------------------------------------------------------
    SecureByteBlockPtr ICEGatherer::handleIncomingPacket(
                                                         CandidatePtr localCandidate,
                                                         const IPAddress &remoteIP,
                                                         STUNPacketPtr stunPacket
                                                         )
    {
      RoutePtr route;
      RouterRoutePtr routerRoute;
      UseICETransportPtr transport;
      String rFrag;

      {
        AutoRecursiveLock lock(*this);

        // scope: check binding request
        {
          // try and setup a quick route
          if ((STUNPacket::Class_Request != stunPacket->mClass) ||
              (STUNPacket::Method_Binding != stunPacket->mMethod)) {

            // scope: search for existing route
            {
              auto route = installRoute(localCandidate, remoteIP, UseICETransportPtr());
              if (route) {
                routerRoute = route->mRouterRoute;
                transport = route->mTransport.lock();

                if (transport) goto found_transport;
              }
            }
            
            goto buffer_data_now;
          }

          String username = stunPacket->mUsername;

          auto pos = username.find(':');
          if (pos == String::npos) {
            ZS_LOG_WARNING(Debug, log("stun packet does not contain a proper username") + stunPacket->toDebug())
            goto stun_failed_validation;
          }

          String lFrag = username.substr(0, pos);
          rFrag = username.substr(pos+1);

          if (lFrag != mUsernameFrag) {
            ZS_LOG_WARNING(Debug, log("stun packet does not contain expected username") + ZS_PARAM("expecting", mUsernameFrag) + stunPacket->toDebug())
            goto stun_failed_validation;
          }

          if (!stunPacket->isValidMessageIntegrity(mPassword)) {
            if (!stunPacket->hasAttribute(STUNPacket::Attribute_MSICE2_ImplementationVersion)) {
              ZS_LOG_WARNING(Debug, log("stun packet does pass message integrity") + ZS_PARAM("password", mPassword) + stunPacket->toDebug());
              goto stun_failed_validation;
            }
            stunPacket->mOptions.mCalculateMessageIntegrityUsingFinalMessageSize = true;
            stunPacket->mOptions.mZeroPadMessageIntegrityInputToBlockSize = 64;
            if (!stunPacket->isValidMessageIntegrity(mPassword)) {
              ZS_LOG_WARNING(Debug, log("stun packet does pass MSICE message integrity") + ZS_PARAM("password", mPassword) + stunPacket->toDebug());
              goto stun_failed_validation;
            }
          }

          auto found = mInstalledTransports.find(rFrag);
          if (found == mInstalledTransports.end()) goto buffer_data_now;

          auto installedTransport = (*found).second;
          transport = installedTransport->mTransport.lock();
          if (!transport) {
            ZS_LOG_WARNING(Debug, log("transport is now gone") + ZS_PARAM("transport id", installedTransport->mTransportID) + stunPacket->toDebug())
            mInstalledTransports.erase(found);
            goto buffer_data_now;
          }

          route = installRoute(localCandidate, remoteIP, transport);
          if (!route) {
            ZS_LOG_WARNING(Debug, log("failed to install route thus must buffer data") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + ZS_PARAM("transport", transport->getID()))
            goto buffer_data_now;
          }

          ZS_LOG_DEBUG(log("installed new route") + route->toDebug())
          routerRoute = route->mRouterRoute;
          goto found_transport;
        }
      }

      goto buffer_data_now;

    found_transport:
      {
        ZS_LOG_DEBUG(log("forwarding stun packet to ice transport") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("from ip", remoteIP.string()) + stunPacket->toDebug());
        ZS_EVENTING_5(
                      x, i, Trace, IceGathererDeliverIceTransportIncomingStunPacket, ol, IceGatherer, Deliver,
                      puid, id, mID,
                      puid, iceTransportId, transport->getID(),
                      puid, routeId, ((bool)route) ? route->mID : static_cast<PUID>(0),
                      puid, routerRouteId, routerRoute->mID,
                      bool, wasBuffered, false
                      );
        stunPacket->trace(__func__);
        transport->notifyPacket(routerRoute, stunPacket);
        return SecureByteBlockPtr();
      }

    stun_failed_validation:
      {
        stunPacket->mErrorCode = STUNPacket::ErrorCode_Unauthorized;

        STUNPacketPtr response = stunPacket->createErrorResponse(stunPacket);
        response->mPassword = mPassword;
        response->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
        fix(response);

        ZS_EVENTING_4(
                      x, e, Debug, IceGathererErrorIceTransportIncomingStunPacket, ol, IceGatherer, Receive,
                      puid, id, mID,
                      puid, iceTransportId, transport ? transport->getID() : static_cast<PUID>(0),
                      puid, routeId, ((bool)route) ? route->mID : static_cast<PUID>(0),
                      puid, routerRouteId, routerRoute ? routerRoute->mID : static_cast<PUID>(0)
                      );

        ZS_LOG_ERROR(Debug, log("candidate password integrity failed") + ZS_PARAM("request", stunPacket->toDebug()) + ZS_PARAM("reply", response->toDebug()))
        response->trace(__func__);
        return response->packetize(STUNPacket::RFC_5245_ICE);
      }

    buffer_data_now:
      {
        if (!routerRoute) {
          routerRoute = mGathererRouter->findRoute(localCandidate, remoteIP, true);
        }
        if (!routerRoute) {
          ZS_LOG_WARNING(Detail, log("unable to create router route (thus must ignore incoming packet)") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + stunPacket->toDebug())
          return SecureByteBlockPtr();
        }

        AutoRecursiveLock lock(*this);

        BufferedPacketPtr packet(make_shared<BufferedPacket>());
        packet->mTimestamp = zsLib::now();
        packet->mRouterRoute = routerRoute;
        packet->mSTUNPacket = stunPacket;
        packet->mRFrag = rFrag;

        ZS_EVENTING_2(
                      x, i, Trace, IceGathererBufferIceTransportIncomingStunPacket, ol, IceGatherer, Buffer,
                      puid, id, mID,
                      puid, routerRouteId, routerRoute->mID
                      );
        stunPacket->trace(__func__);

        ZS_LOG_TRACE(log("buffering stun packet until ice transport installed to handle packet") + packet->toDebug())
        mBufferedPackets.push_back(packet);

        if (!mCleanUpBufferingTimer) {
          mCleanUpBufferingTimer = ITimer::create(mThisWeak.lock(), Seconds(1));
        }
      }
      return SecureByteBlockPtr();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::handleIncomingPacket(
                                           CandidatePtr localCandidate,
                                           const IPAddress &remoteIP,
                                           const BYTE *buffer,
                                           size_t bufferSizeInBytes
                                           )
    {
      RoutePtr route;
      RouterRoutePtr routerRoute;
      UseICETransportPtr transport;

      {
        AutoRecursiveLock lock(*this);

        // scope: search for existing route
        {
          route = installRoute(localCandidate, remoteIP, UseICETransportPtr());
          if (route) {
            routerRoute = route->mRouterRoute;
            transport = route->mTransport.lock();

            if (transport) goto found_transport;
          }
        }
      }

      goto buffer_data_now;

    found_transport:
      {
        ZS_LOG_DEBUG(log("forwarding data packet to ice transport") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("from ip", remoteIP.string()) + ZS_PARAM("size", bufferSizeInBytes));
        ZS_EVENTING_7(
                      x, i, Trace, IceGathererDeliverIceTransportIncomingPacket, ol, IceGatherer, Deliver,
                      puid, id, mID,
                      puid, iceTransportId, transport->getID(),
                      puid, routeId, routerRoute->mID,
                      puid, routerRouteId, routerRoute->mID,
                      bool, wasBuffered, false,
                      buffer, packet, buffer,
                      size, size, bufferSizeInBytes
                      );
        transport->notifyPacket(routerRoute, buffer, bufferSizeInBytes);
      }

    buffer_data_now:
      {
        if (!routerRoute) {
          routerRoute = mGathererRouter->findRoute(localCandidate, remoteIP, true);
        }
        if (!routerRoute) {
          ZS_LOG_WARNING(Detail, log("unable to create router route (thus must ignore incoming packet)") + localCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + ZS_PARAM("size", bufferSizeInBytes))
          return;
        }

        AutoRecursiveLock lock(*this);

        BufferedPacketPtr packet(make_shared<BufferedPacket>());
        packet->mTimestamp = zsLib::now();
        packet->mRouterRoute = routerRoute;
        packet->mBuffer = IHelper::convertToBuffer(buffer, bufferSizeInBytes);

        ZS_EVENTING_6(
                      x, i, Trace, IceGathererBufferIceTransportIncomingPacket, ol, IceGatherer, Buffer,
                      puid, id, mID,
                      puid, iceTransportId, transport->getID(),
                      puid, routerRouteId, routerRoute->mID,
                      bool, wasBuffered, false,
                      buffer, packet, buffer,
                      size, size, bufferSizeInBytes
                      );

        ZS_LOG_TRACE(log("buffering packet until ice transport installed to handle packet") + packet->toDebug())
        mBufferedPackets.push_back(packet);
        
        if (!mCleanUpBufferingTimer) {
          mCleanUpBufferingTimer = ITimer::create(mThisWeak.lock(), Seconds(1));
        }
      }
    }

    //-------------------------------------------------------------------------
    IICETypes::CandidatePtr ICEGatherer::findSentFromLocalCandidate(RouterRoutePtr routerRoute)
    {
      CandidatePtr result;

      auto routerCandidate = routerRoute->mLocalCandidate;
      String routerCandidateHash = routerRoute->mLocalCandidate->hash();

      AutoRecursiveLock lock(*this);

      // NOTE: The candidates pointers from the routerer are not the same as
      //       candidate pointer used by the ice gatherer. As such, we need
      //       to compare by hash and not by candidate pointer match.

      {
        for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
          auto hostPort = (*iter).second;
          if (IICETypes::Protocol_UDP == routerCandidate->mProtocol) {
            if (hostPort->mCandidateUDP) {
              if (hostPort->mCandidateUDP->hash() == routerCandidateHash) {
                result = hostPort->mCandidateUDP;
                goto done;
              }
            }

            for (auto iterRelay = hostPort->mRelayPorts.begin(); iterRelay != hostPort->mRelayPorts.end(); ++iterRelay) {
              auto relayPort = (*iterRelay);
              if (relayPort->mReflexiveCandidate) {
                if (relayPort->mReflexiveCandidate->hash() == routerCandidateHash) {
                  result = hostPort->mCandidateUDP;
                  goto done;
                }
              }
              if (relayPort->mRelayCandidate) {
                if (relayPort->mRelayCandidate->hash() == routerCandidateHash) {
                  result = relayPort->mRelayCandidate;
                  goto done;
                }
              }
            }

            for (auto iterRelay = hostPort->mReflexivePorts.begin(); iterRelay != hostPort->mReflexivePorts.end(); ++iterRelay) {
              auto reflexivePort = (*iterRelay);
              if (reflexivePort->mCandidate) {
                if (reflexivePort->mCandidate->hash() == routerCandidateHash) {
                  result = hostPort->mCandidateUDP;
                  goto done;
                }
              }
            }
          }

          if (IICETypes::Protocol_TCP == routerCandidate->mProtocol) {
            if (hostPort->mCandidateTCPPassive) {
              if (hostPort->mCandidateTCPPassive->hash() == routerCandidateHash) {
                result = hostPort->mCandidateTCPPassive;
                goto done;
              }
            }
            if (hostPort->mCandidateTCPActive) {
              if (hostPort->mCandidateTCPActive->hash() == routerCandidateHash) {
                result = hostPort->mCandidateTCPActive;
                goto done;
              }
            }
          }

        }

        goto done;
      }

    done:
      {
        if (result) {
          ZS_LOG_TRACE(log("sent from local candidate found") + result->toDebug() + routerRoute->toDebug())
        } else {
          ZS_LOG_WARNING(Detail, log("no sent from local candidate found for router route") + routerRoute->toDebug())
        }
      }

      return result;
    }
    
    //-----------------------------------------------------------------------
    ICEGatherer::RoutePtr ICEGatherer::installRoute(
                                                    RouterRoutePtr routerRoute,
                                                    UseICETransportPtr transport
                                                    )
    {
      RoutePtr route;

      // scope: see if route already exists
      {
        auto found = mRoutes.find(routerRoute->mID);
        if (found == mRoutes.end()) goto install_new_route;

        route = (*found).second;
        auto foundTransport = route->mTransport.lock();

        if (!foundTransport) goto remove_existing_route;
        if (transport) {
          if (foundTransport->getID() != transport->getID()) goto remove_existing_route;
        }

        route->mLastUsed = zsLib::now();
        return route;
      }

    remove_existing_route:
      {
        ZS_LOG_WARNING(Debug, log("previous route must be removed") + route->toDebug() + routerRoute->toDebug() + ZS_PARAM("transport id", transport ? transport->getID() : 0))
        removeRoute(route->mRouterRoute);
        goto install_new_route;
      }

   install_new_route:
      {
        CandidatePtr sentFromLocalCandidate = findSentFromLocalCandidate(routerRoute);
        if (!sentFromLocalCandidate) {
          ZS_LOG_WARNING(Detail, log("unable to find local candidate for router route") + routerRoute->toDebug())
          return RoutePtr();
        }
        return installRoute(sentFromLocalCandidate, routerRoute->mRemoteIP, transport);
      }
    }

    //-----------------------------------------------------------------------
    ICEGatherer::RoutePtr ICEGatherer::installRoute(
                                                    CandidatePtr sentFromLocalCandidate,
                                                    const IPAddress &remoteIP,
                                                    UseICETransportPtr transport
                                                    )
    {
      LocalCandidateRemoteIPPair search(sentFromLocalCandidate, remoteIP);

      RoutePtr route;

      // see if route already exists
      {
        auto found = mQuickSearchRoutes.find(search);
        ZS_EVENTING_4(
                      x, i, Trace, IceGathererSearchQuickRoute, ol, IceGatherer, Info,
                      puid, id, mID,
                      pointer, localCandidate, sentFromLocalCandidate.get(),
                      string, remoteIp, remoteIP.string(),
                      bool, found, found != mQuickSearchRoutes.end()
                      );
        if (found == mQuickSearchRoutes.end()) goto create_new_route;

        // found a route mapping
        route = (*found).second;
        auto foundTransport = route->mTransport.lock();

        if (!foundTransport) goto remove_existing_route;
        if (transport) {
          if (foundTransport->getID() != transport->getID()) goto remove_existing_route;
        }

        route->mLastUsed = zsLib::now();
        return route;
      }

    remove_existing_route:
      {
        ZS_LOG_WARNING(Debug, log("previous route must be removed") + route->toDebug() + ZS_PARAM("transport id", transport ? transport->getID() : 0))
        removeRoute(route->mRouterRoute);
        goto create_new_route;
      }

    create_new_route:
      {
        if (!transport) {
          ZS_LOG_WARNING(Trace, log("transport to install was not found"))
          return RoutePtr();
        }

        // install a route
        route = make_shared<Route>();
        route->mOuterObjectID = mID;
        route->mLastUsed = zsLib::now();
        route->mLocalCandidate = sentFromLocalCandidate;
        route->mTransportID = transport->getID();
        route->mTransport = transport;

        ZS_LOG_DEBUG(log("installing new route") + route->toDebug())

        // scope: figure out how to route outgoing packets
        {
          if (IICETypes::Protocol_TCP == sentFromLocalCandidate->mProtocol) {
            auto found = mTCPCandidateToTCPPorts.find(sentFromLocalCandidate);
            if (found != mTCPCandidateToTCPPorts.end()) {
              auto tcpPort = (*found).second;

              if (tcpPort->mRemoteIP == remoteIP) {
                route->mTCPPort = tcpPort;
                if (0 == route->mTCPPort->mTransportID) {
                  auto foundTransport = route->mTransport.lock();
                  route->mTransport = foundTransport;
                  if (foundTransport) transport = foundTransport;

                  route->mTransportID = transport->getID();
                }
                goto resolved_local_candidate;
              }
            }

            for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
              auto hostPort = (*iter).second;
              if ((hostPort->mCandidateTCPPassive == sentFromLocalCandidate) ||
                  (hostPort->mCandidateTCPActive == sentFromLocalCandidate)) {
                // search for an incoming or outgoing TCP connection that satisfies the requirement
                for (auto iter = hostPort->mTCPPorts.begin(); iter != hostPort->mTCPPorts.end(); ++iter) {
                  auto tcpPort = (*iter).second.second;
                  auto tcpPortTransport = tcpPort->mTransport.lock();

                  if (tcpPort->mRemoteIP != remoteIP) continue; // must be connecting to/from same remote location
                  if (tcpPortTransport != transport) continue;  // do not pick unless confirmed that it belongs to this transport
                  if (tcpPort->mCandidate->mTCPType != sentFromLocalCandidate->mTCPType) continue; // passive must match passive and active must match active

                  // this is an exact match
                  route->mTCPPort = tcpPort;
                  if (tcpPort->mConnected) {
                    break; // prefer a connected TCP port (otherwise keep searching)
                  }
                }

                if (route->mTCPPort) {
                  route->mLocalCandidate = route->mTCPPort->mCandidate;
                  goto resolved_local_candidate;
                }

                ZS_LOG_DEBUG(log("no existing TCP ports found that can satisfy this route") + sentFromLocalCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + ZS_PARAM("transport", transport->getID()))

                TCPPortPtr tcpPort(make_shared<TCPPort>());

                tcpPort->mConnected = false;
                tcpPort->mRemoteIP = remoteIP;
                tcpPort->mTransportID = transport->getID();
                tcpPort->mTransport = transport;

                ZS_EVENTING_3(
                              x, i, Trace, IceGathererTcpPortCreate, ol, IceGatherer, Start,
                              puid, id, mID,
                              puid, tcpPortId, tcpPort->mID,
                              string, remoteIp, tcpPort->mRemoteIP.string()
                              );

                IPAddress localIP;

                try {
                  auto createFamily = (hostPort->mHostData->mIP.isIPv6() ? Socket::Create::IPv6 : Socket::Create::IPv4);
                  tcpPort->mSocket = Socket::createTCP(createFamily);
                  tcpPort->mSocket->bind(hostPort->mHostData->mIP);
                  localIP = tcpPort->mSocket->getLocalAddress();

                  tcpPort->mCandidate = hostPort->mCandidateTCPActive;

                  bool woudlBlock = false;

                  tcpPort->mSocket->setBlocking(false);
                  tcpPort->mSocket->setDelegate(mThisWeak.lock());
                  tcpPort->mSocket->connect(remoteIP, &woudlBlock);
                } catch(Socket::Exceptions::Unspecified &error) {
                  ZS_LOG_WARNING(Detail, log("failed to create an outgoing TCP connection") + ZS_PARAM("error", error.errorCode()) + ZS_PARAM("local ip", localIP.string()) + ZS_PARAM("remote ip", remoteIP.string()))
                  tcpPort->mSocket.reset();
                  goto failed_resolve_local_candidate;
                }

                route->mTCPPort = tcpPort;

                hostPort->mTCPPorts[tcpPort->mSocket] = HostAndTCPPortPair(hostPort, tcpPort);
                mTCPPorts[tcpPort->mSocket] = HostAndTCPPortPair(hostPort, tcpPort);
                mTCPCandidateToTCPPorts[tcpPort->mCandidate] = tcpPort;

                ZS_LOG_DEBUG(log("created outgoing TCP connection for use in route") + tcpPort->toDebug() + route->toDebug())
                
                goto resolved_local_candidate;
              }
            }
            goto failed_resolve_local_candidate;
          }

          if (IICETypes::Protocol_UDP == sentFromLocalCandidate->mProtocol) {
            for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
              auto hostPort = (*iter).second;
              if (hostPort->mCandidateUDP == sentFromLocalCandidate) {
                route->mHostPort = hostPort;
                goto resolved_local_candidate;
              }
              for (auto iterRelay = hostPort->mRelayPorts.begin(); iterRelay != hostPort->mRelayPorts.end(); ++iterRelay) {
                auto relayPort = (*iterRelay);
                if (relayPort->mReflexiveCandidate == sentFromLocalCandidate) {
                  route->mHostPort = hostPort;
                  goto resolved_local_candidate;
                }
                if (relayPort->mRelayCandidate == sentFromLocalCandidate) {
                  route->mRelayPort = relayPort;
                  goto resolved_local_candidate;
                }
              }

              for (auto iterReflexive = hostPort->mReflexivePorts.begin(); iterReflexive != hostPort->mReflexivePorts.end(); ++iterReflexive) {
                auto reflexivePort = (*iterReflexive);

                if (reflexivePort->mCandidate == sentFromLocalCandidate) {
                  route->mHostPort = hostPort;
                  goto resolved_local_candidate;
                }
              }
            }
          }

          goto failed_resolve_local_candidate;
        }

      resolved_local_candidate:
        {
          route->mRouterRoute = mGathererRouter->findRoute(route->mLocalCandidate, remoteIP, true);

          if (!route->mRouterRoute) {
            ZS_LOG_WARNING(Detail, log("failed to create router route") + route->toDebug())
            return RoutePtr();
          }
          
          removeRoute(route->mRouterRoute);

          if (!transport) {
            ZS_LOG_WARNING(Detail, log("transport to install route was not found"))
            goto failed_resolve_local_candidate;
          }

          route->trace(__func__, "install route");

          ZS_LOG_TRACE(log("installing route") + route->toDebug());

          ZS_EVENTING_4(
                        x, i, Debug, IceGathererInstallQuickRoute, ol, IceGatherer, Receive,
                        puid, id, mID,
                        pointer, candidate, search.first.get(),
                        string, ip, search.second.string(),
                        puid, routeId, route->mID
                        );

          mRoutes[route->mRouterRoute->mID] = route;
          mQuickSearchRoutes[search] = route;

          IGathererAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyDeliverRouteBufferedPackets(transport, route->mRouterRoute->mID);
          return route;
        }

      failed_resolve_local_candidate:
        {
          ZS_LOG_WARNING(Detail, log("failed to find any local candidate to route packets") + sentFromLocalCandidate->toDebug() + ZS_PARAM("remote ip", remoteIP.string()) + ZS_PARAM("transport", transport->getID()))
          return RoutePtr();
        }
      }

      return route;
    }

    //-----------------------------------------------------------------------
    void ICEGatherer::fix(STUNPacketPtr stun) const
    {
      stun->mLogObject = "ortc::ICEGatherer";
      stun->mLogObjectID = mID;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::removeAllRelatedRoutes(
                                             TransportID transportID,
                                             UseICETransportPtr transportIfAvailable
                                             )
    {
      for (auto iter_doNotUse = mQuickSearchRoutes.begin(); iter_doNotUse != mQuickSearchRoutes.end();)
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (route->mTransportID != transportID) continue;

        ZS_EVENTING_4(
                      x, i, Debug, IceGathererRemoveQuickRoute, ol, IceGatherer, Info,
                      puid, id, mID,
                      pointer, candidate, (*current).first.first.get(),
                      string, ipAddress, (*current).first.second.string(),
                      puid, routeId, route->mID
                      );

        ZS_LOG_WARNING(Detail, log("need to remove route because of unbinding previous transport") + route->toDebug())

        mQuickSearchRoutes.erase(current);
      }

      for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end();)
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second;
        if (route->mTransportID != transportID) continue;

        route->trace(__func__, "remove all related");

        ZS_LOG_WARNING(Detail, log("need to remove route because of unbinding previous transport") + route->toDebug())

        mRoutes.erase(current);
      }
    }
    
    //-------------------------------------------------------------------------
    bool ICEGatherer::sendUDPPacket(
                                    SocketPtr socket,
                                    const IPAddress &boundIP,
                                    const IPAddress &remoteIP,
                                    const BYTE *buffer,
                                    size_t bufferSizeInBytes
                                    )
    {
      if (!socket) return false;
      if (!buffer) return true;
      if (0 == bufferSizeInBytes) return true;

      try {
        bool wouldBlock = false;

        ZS_EVENTING_5(
                      x, i, Trace, IceGathererUdpSocketPacketSentTo, ol, IceGatherer, Send,
                      puid, id, mID,
                      string, boundIp, boundIP.string(),
                      string, remoteIp, remoteIP.string(),
                      buffer, packet, buffer,
                      size, size, bufferSizeInBytes
                      );

        auto sent = socket->sendTo(remoteIP, buffer, bufferSizeInBytes, &wouldBlock);
        ZS_LOG_INSANE(log("packet sent") + ZS_PARAM("socket", string(socket)) + ZS_PARAM("to", remoteIP.string()) + ZS_PARAM("from", boundIP.string()) + ZS_PARAM("size", bufferSizeInBytes))

        if (sent == bufferSizeInBytes) return true;
      } catch(Socket::Exceptions::Unspecified &error) {
        ZS_LOG_ERROR(Debug, log("unable to send packet") + ZS_PARAM("error", error.errorCode()) + ZS_PARAM("to", remoteIP.string()) + ZS_PARAM("from", boundIP.string()))
        return false;
      }

      ZS_LOG_WARNING(Trace, log("could not send packet at this time") + ZS_PARAM("socket", string(socket)) + ZS_PARAM("to", remoteIP.string()) + ZS_PARAM("from", boundIP.string()) + ZS_PARAM("size", bufferSizeInBytes))
      return false;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::shouldKeepWarm() const
    {
      if (mKeepWarmSinceJustCreated) return true;
      if (mTransportsStillNeedsCandidates) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::shouldWarmUpAfterInterfaceBinding() const
    {
      if (Time() != mWarmUpAfterNewInterfaceBindingUntil) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::fixSTUNParserOptions(const STUNPacketPtr &packet)
    {
      if (mSTUNPacketParseOptions.mBindResponseRequiresUsernameAttribute) return; // already "fixed" broken ICE

      if (!packet) return;
      if (STUNPacket::Method_Binding != packet->mMethod) return;
      if (STUNPacket::Class_Response != packet->mClass) return;
      if (packet->mUsername.isEmpty()) return;

      mSTUNPacketParseOptions.mBindResponseAllowedUsernameAttribute = true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::HostIPSorter
    #pragma mark

    //-------------------------------------------------------------------------
    bool ICEGatherer::HostIPSorter::compareLocalIPs(const DataPtr &pData1, const DataPtr &pData2)
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!pData1)
      ZS_THROW_INVALID_ARGUMENT_IF(!pData2)

      const Data &data1 = (*pData1);
      const Data &data2 = (*pData2);

      if ((InterfaceType_Unknown != data1.mInterfaceType) &&
          (InterfaceType_Unknown != data2.mInterfaceType)) {
        if (data1.mInterfaceType < data2.mInterfaceType) return true;
        if (data1.mInterfaceType > data2.mInterfaceType) return false;
      }

      if (data1.mOrderIndex < data2.mOrderIndex) return true;
      if (data1.mOrderIndex > data2.mOrderIndex) return false;
      if (data1.mAdapterMetric < data2.mAdapterMetric) return true;
      if (data1.mAdapterMetric > data2.mAdapterMetric) return false;
      if (data1.mIndex < data2.mIndex) return true;
      if (data1.mIndex > data2.mIndex) return false;

      if (data1.mIP.isIPv4()) {
        if (data2.mIP.isIPv4()) {
          return data1.mIP < data2.mIP;
        }
        return true;
      }

      if (data2.mIP.isIPv4())
        return false;

      return data1.mIP < data2.mIP;
    }

    //-------------------------------------------------------------------------
    ICEGatherer::HostIPSorter::DataPtr ICEGatherer::HostIPSorter::prepare(
                                                                          const IPAddress &ip,
                                                                          const Options &options
                                                                          )
    {
      DataPtr data(make_shared<Data>());
      data->mIP = ip;
      InterfaceMappingList empty;
      fixMapping(*data, empty);
      fixPolicy(*data, options);
      return data;
    }

    //-------------------------------------------------------------------------
    ICEGatherer::HostIPSorter::DataPtr ICEGatherer::HostIPSorter::prepare(
                                                                          const char *hostName,
                                                                          const IPAddress &ip,
                                                                          const Options &options
                                                                          )
    {
      DataPtr data(make_shared<Data>());
      data->mHostName = String(hostName);
      data->mIP = ip;
      InterfaceMappingList empty;
      fixMapping(*data, empty);
      fixPolicy(*data, options);
      return data;
    }

    //-------------------------------------------------------------------------
    ICEGatherer::HostIPSorter::DataPtr ICEGatherer::HostIPSorter::prepare(
                                                                          const char *interfaceName,
                                                                          const char *hostName,
                                                                          const IPAddress &ip,
                                                                          const InterfaceMappingList &prefs,
                                                                          const Options &options
                                                                          )
    {
      DataPtr data(make_shared<Data>());
      data->mInterfaceName = String(interfaceName);
      data->mHostName = String(hostName);
      data->mIP = ip;
      fixMapping(*data, prefs);
      fixPolicy(*data, options);
      return data;
    }

    //-------------------------------------------------------------------------
    ICEGatherer::HostIPSorter::DataPtr ICEGatherer::HostIPSorter::prepare(
                                                                          const char *interfaceName,
                                                                          const char *interfaceDescription,
                                                                          const IPAddress &ip,
                                                                          ULONG metric,
                                                                          const InterfaceMappingList &prefs,
                                                                          const Options &options
                                                                          )
    {
      DataPtr data(make_shared<Data>());
      data->mInterfaceName = String(interfaceName);
      data->mInterfaceDescription = String(interfaceDescription);
      data->mIP = ip;
      data->mAdapterMetric = metric;
      fixMapping(*data, prefs);
      fixPolicy(*data, options);
      return data;
    }

    //-------------------------------------------------------------------------
    ICEGatherer::HostIPSorter::DataPtr ICEGatherer::HostIPSorter::prepare(
                                                                          const char *interfaceName,
                                                                          const IPAddress &ip,
                                                                          const InterfaceMappingList &prefs,
                                                                          const Options &options
                                                                          )
    {
      DataPtr data(make_shared<Data>());
      data->mInterfaceName = String(interfaceName);
      data->mIP = ip;
      fixMapping(*data, prefs);
      fixPolicy(*data, options);
      return data;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::HostIPSorter::fixMapping(
                                               Data &ioData,
                                               const InterfaceMappingList &prefs
                                               )
    {
      for (int loop = 0; loop < 2; ++loop) {
        const String &useValue = (0 == loop ? ioData.mInterfaceName : ioData.mInterfaceDescription);
        if (useValue.isEmpty()) continue;

        const char *start = useValue.c_str();
        const char *num = start + useValue.length();

        for (; num != start; --num) {
          if ('\0' == *num) continue;
          if (isdigit(*num)) continue;

          // this is not a digit
          ++num;
          break;
        }

        String numStr(num);
        if (numStr.hasData()) {
          try {
            ioData.mIndex = Numeric<decltype(ioData.mIndex)>(numStr);
            goto number_found;
          } catch(const Numeric<decltype(ioData.mIndex)>::ValueOutOfRange &) {
            ZS_LOG_WARNING(Detail, slog("number failed to convert") + ZS_PARAM("number", numStr) + ioData.toDebug())
          }
        }
      }

    number_found:
      {
        for (auto iter = prefs.begin(); iter != prefs.end(); ++iter) {
          const InterfaceNameMappingInfo &info = (*iter);

          if (info.mInterfaceNameRegularExpression.isEmpty()) {
            ZS_LOG_WARNING(Detail, slog("invalid regular expression") + info.toDebug())
            continue;
          }

          if ((ioData.mInterfaceName.isEmpty()) &&
              (ioData.mInterfaceDescription.isEmpty())) {
            ZS_LOG_TRACE(slog("no interface information found") + ioData.toDebug())
          }

          String exp = info.mInterfaceNameRegularExpression;

          std::regex_constants::syntax_option_type syntaxOptions {};
          if (0 == exp.compare(0, strlen("(?i)"), "(?i)")) {
            exp = exp.substr(strlen("(?i)"));
            syntaxOptions = std::regex_constants::icase;
          }

          std::regex e(exp, syntaxOptions);

          if (ioData.mInterfaceName.hasData()) {
            if (std::regex_match(ioData.mInterfaceName, e)) goto found_match;
          }
          if (ioData.mInterfaceDescription.hasData()) {
            if (std::regex_match(ioData.mInterfaceName, e)) goto found_match;
          }

          continue;

        found_match:
          {
            ZS_LOG_TRACE(slog("fixed mapping interface type to ip") + ioData.toDebug() + info.toDebug())

            ioData.mOrderIndex = info.mOrderIndex;
            if (InterfaceType_Unknown == ioData.mInterfaceType) {
              ioData.mInterfaceType = info.mInterfaceType;
            }
            return;
          }
        }
      }

      // did not find match, guess the network type based purely on the IP address

      if (!ioData.mIP.isIPv6()) return;

      if ((ioData.mIP.isIPv46to4()) ||
          (ioData.mIP.isIPv4Compatible()) ||
          (ioData.mIP.isIPv4Mapped()) ||
          (ioData.mIP.isTeredoTunnel())) {
        ioData.mInterfaceType = InterfaceType_Tunnel; // treat it like a tunnel interface
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::HostIPSorter::fixPolicy(
                                              Data &ioData,
                                              const Options &options
                                              )
    {
      FilterPolicies defaultPolicy = FilterPolicy_None;
      for (auto iter = options.mInterfacePolicies.begin(); iter != options.mInterfacePolicies.end(); ++iter) {
        auto interfacePolicy = (*iter);
        InterfaceTypes interfaceType = toInterfaceType(interfacePolicy.mInterfaceType);
        if (InterfaceType_Unknown == interfaceType) {
          ZS_LOG_WARNING(Debug, slog("interface type is not understood") + interfacePolicy.toDebug())
          continue;
        }
        if (InterfaceType_Default == interfaceType) {
          ZS_LOG_TRACE(slog("found default filter policy") + interfacePolicy.toDebug())
          defaultPolicy = interfacePolicy.mGatherPolicy;
          continue;
        }
        if (interfaceType == ioData.mInterfaceType) {
          ZS_LOG_TRACE(slog("found exact interface policy") + interfacePolicy.toDebug())
          ioData.mFilterPolicy = interfacePolicy.mGatherPolicy;
          return;
        }
      }
      ioData.mFilterPolicy = defaultPolicy;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::HostIPSorter::sort(DataList &ioDataList)
    {
      ioDataList.sort(HostIPSorter::compareLocalIPs);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::HostIPSorter::Data
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::HostIPSorter::Data::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::HostIPSorter::Data");

      IHelper::debugAppend(resultEl, "interface name", mInterfaceName);
      IHelper::debugAppend(resultEl, "interface description", mInterfaceDescription);
      IHelper::debugAppend(resultEl, "interface type", toString(mInterfaceType));
      IHelper::debugAppend(resultEl, "filter policy", IICEGathererTypes::toString(mFilterPolicy));

      IHelper::debugAppend(resultEl, "host name", mHostName);
      IHelper::debugAppend(resultEl, "ip", mIP.string());
      IHelper::debugAppend(resultEl, "order id", mOrderIndex);
      IHelper::debugAppend(resultEl, "adapter metric", mAdapterMetric);
      IHelper::debugAppend(resultEl, "index", mIndex);
      IHelper::debugAppend(resultEl, "temporary ip", mIsTemporaryIP);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::HostIPSorter::InterfaceNameMappingInfo
    #pragma mark

    //-------------------------------------------------------------------------
    ICEGatherer::HostIPSorter::InterfaceNameMappingInfo ICEGatherer::HostIPSorter::InterfaceNameMappingInfo::create(ElementPtr el)
    {
      InterfaceNameMappingInfo info;

      if (!el) return info;

      info.mInterfaceNameRegularExpression = IHelper::getElementTextAndDecode(el->findFirstChildElement("nameRegEx"));
      info.mInterfaceType =  toInterfaceType(IHelper::getElementTextAndDecode(el->findFirstChildElement("type")));
      String order = IHelper::getElementText(el->findFirstChildElement("order"));
      try {
        info.mOrderIndex = Numeric<decltype(info.mOrderIndex)>(order);
      } catch(const Numeric<decltype(info.mOrderIndex)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Detail, slog("failed to convert expression") + ZS_PARAMIZE(order))
      }

      return info;
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::HostIPSorter::InterfaceNameMappingInfo::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      if (mInterfaceNameRegularExpression.hasData()) {
        rootEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("nameRegEx", mInterfaceNameRegularExpression));
      }
      if (InterfaceType_Unknown != mInterfaceType) {
        rootEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("type", toString(mInterfaceType)));
      }
      if (0 != mOrderIndex) {
        rootEl->adoptAsLastChild(IHelper::createElementWithNumber("order", string(mOrderIndex)));
      }
      return rootEl;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::HostIPSorter::InterfaceNameMappingInfo::hasData() const
    {
      return ((mInterfaceNameRegularExpression.hasData()) ||
              (InterfaceType_Unknown != mInterfaceType) ||
              (0 != mOrderIndex));
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::HostIPSorter::InterfaceNameMappingInfo::toDebug() const
    {
      return createElement("ortc::ICEGatherer::HostIPSorter::InterfaceNameMappingInfo");
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::ReflexivePort
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::ReflexivePort::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::ReflexivePort");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, mServer.toDebug());
      IHelper::debugAppend(resultEl, "stun discovery", UseSTUNDiscovery::toDebug(mSTUNDiscovery));

      IHelper::debugAppend(resultEl, "options hash", mOptionsHash);
      IHelper::debugAppend(resultEl, mCandidate ? mCandidate->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "last activity", mLastActivity);
      IHelper::debugAppend(resultEl, "inactivity timer", mInactivityTimer ? mInactivityTimer->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::RelayPort
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::RelayPort::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::RelayPort");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, mServer.toDebug());
      IHelper::debugAppend(resultEl, "turn socket", UseTURNSocket::toDebug(mTURNSocket));

      IHelper::debugAppend(resultEl, "options hash", mOptionsHash);
      IHelper::debugAppend(resultEl, "relay candidate", mRelayCandidate ? mRelayCandidate->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "reflexive candidate", mRelayCandidate ? mRelayCandidate->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "last activity", mLastActivity);
      IHelper::debugAppend(resultEl, "inactivity timer", mInactivityTimer ? mInactivityTimer->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::HostPort
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::HostPort::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::HostPort");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, mHostData->toDebug());

      IHelper::debugAppend(resultEl, "options hash", mBoundOptionsHash);

      IHelper::debugAppend(resultEl, "candidate udp", mCandidateUDP ? mCandidateUDP->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "bound udp ip", mBoundUDPIP.string());
      IHelper::debugAppend(resultEl, "bound udp socket", string(mBoundUDPSocket));
      IHelper::debugAppend(resultEl, "udp back off timer", UseBackOffTimer::toDebug(mBindUDPBackOffTimer));

      IHelper::debugAppend(resultEl, "passive candidate tcp", mCandidateTCPPassive ? mCandidateTCPPassive->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "active candidate tcp", mCandidateTCPActive ? mCandidateTCPActive->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "bound udp ip", mBoundTCPIP.string());
      IHelper::debugAppend(resultEl, "bound tcp socket", string(mBoundTCPSocket));
      IHelper::debugAppend(resultEl, "tcp back off timer", UseBackOffTimer::toDebug(mBindTCPBackOffTimer));

      IHelper::debugAppend(resultEl, "warm up after binding", mWarmUpAfterBinding);

      IHelper::debugAppend(resultEl, "reflexive options hash", mReflexiveOptionsHash);
      IHelper::debugAppend(resultEl, "reflexive ports", mReflexivePorts.size());

      IHelper::debugAppend(resultEl, "relay options hash", mRelayOptionsHash);
      IHelper::debugAppend(resultEl, "relay ports", mRelayPorts.size());
      IHelper::debugAppend(resultEl, "ip to relay port mapping", mIPToRelayPortMapping.size());

      IHelper::debugAppend(resultEl, "tcp ports", mTCPPorts.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::TCPPort
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::TCPPort::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::TCPPort");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "connected", mConnected);

      IHelper::debugAppend(resultEl, "candidate", mCandidate ? mCandidate->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "remote ip", mRemoteIP.string());
      IHelper::debugAppend(resultEl, "socket", string(mSocket));
      IHelper::debugAppend(resultEl, "incoming buffer", mIncomingBuffer.CurrentSize());
      IHelper::debugAppend(resultEl, "outgoing buffer", mOutgoingBuffer.CurrentSize());

      IHelper::debugAppend(resultEl, "transport id", mTransportID);
      UseICETransportPtr transport = mTransport.lock();
      IHelper::debugAppend(resultEl, "ice transport", transport ? transport->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::BufferedPacket
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::BufferedPacket::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::BufferedPacket");

      IHelper::debugAppend(resultEl, "timestamp", mTimestamp);

      IHelper::debugAppend(resultEl, mRouterRoute ? mRouterRoute->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "stun packet", (bool)mSTUNPacket);
      IHelper::debugAppend(resultEl, "rfrag", mRFrag);

      IHelper::debugAppend(resultEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::Route
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGatherer::Route::trace(const char *function, const char *message) const
    {
      if (mLocalCandidate) {
        ZS_EVENTING_20(
                       x, i, Trace, IceGathererRouteTrace, ol, IceGatherer, Info,
                       puid, routeId, mID,
                       string, callingMethod, function,
                       string, message, message,
                       puid, outerObjectId, mOuterObjectID,
                       duration, lastUsed, zsLib::timeSinceEpoch<Milliseconds>(mLastUsed).count(),
                       puid, transportId, mTransportID,
                       word, hostPort, mHostPort ? mHostPort->mID : static_cast<WORD>(0),
                       word, relayPort, mRelayPort ? mRelayPort->mID : static_cast<WORD>(0),
                       puid, tcpPortId, mTCPPort ? mTCPPort->mID : static_cast<PUID>(0),
                       string, interfaceType, mLocalCandidate->mInterfaceType,
                       string, foundation, mLocalCandidate->mFoundation,
                       dword, priority, mLocalCandidate->mPriority,
                       dword, unfreezePriority, mLocalCandidate->mUnfreezePriority,
                       string, protocol, IICETypes::toString(mLocalCandidate->mProtocol),
                       string, ip, mLocalCandidate->mIP,
                       word, port, mLocalCandidate->mPort,
                       string, candidateType, IICETypes::toString(mLocalCandidate->mCandidateType),
                       string, tcpType, IICETypes::toString(mLocalCandidate->mTCPType),
                       string, relatedAddress, mLocalCandidate->mRelatedAddress,
                       word, relatedPort, mLocalCandidate->mRelatedPort
                       );

      } else {
        ZS_EVENTING_20(
                       x, i, Trace, IceGathererRouteTrace, ol, IceGatherer, Info,
                       puid, routeId, mID,
                       string, callingMethod, function,
                       string, message, message,
                       puid, outerObjectId, mOuterObjectID,
                       duration, lastUsed, zsLib::timeSinceEpoch<Milliseconds>(mLastUsed).count(),
                       puid, transportId, mTransportID,
                       word, hostPort, mHostPort ? mHostPort->mID : static_cast<WORD>(0),
                       word, relayPort, mRelayPort ? mRelayPort->mID : static_cast<WORD>(0),
                       puid, tcpPortId, mTCPPort ? mTCPPort->mID : static_cast<PUID>(0),
                       string, interfaceType, (const char *)NULL,
                       string, foundation, (const char *)NULL,
                       dword, priority, static_cast<DWORD>(0),
                       dword, unfreezePriority, static_cast<DWORD>(0),
                       string, protocol, (const char *)NULL,
                       string, ip, (const char *)NULL,
                       word, port, static_cast<WORD>(0),
                       string, candidateType, (const char *)NULL,
                       string, tcpType, (const char *)NULL,
                       string, relatedAddress, (const char *)NULL,
                       word, relatedPort, static_cast<WORD>(0)
                       );

      }
      if (mRouterRoute) mRouterRoute->trace(function, message);
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::Route::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::Route");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, mRouterRoute ? mRouterRoute->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "last used", mLastUsed);
      IHelper::debugAppend(resultEl, "local candidate", mLocalCandidate ? mLocalCandidate->toDebug() : ElementPtr());

      UseICETransportPtr transport = mTransport.lock();
      IHelper::debugAppend(resultEl, "transport id", mTransportID);
      IHelper::debugAppend(resultEl, "ice transport", transport ? transport->getID() : 0);

      IHelper::debugAppend(resultEl, "host port", mHostPort ? mHostPort->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "relay port", mRelayPort ? mRelayPort->toDebug() : ElementPtr());
      IHelper::debugAppend(resultEl, "tcp port", mTCPPort ? mTCPPort->toDebug() : ElementPtr());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::InstalledTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::InstalledTransport::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::InstalledTransport");

      UseICETransportPtr transport = mTransport.lock();
      IHelper::debugAppend(resultEl, "transport id", mTransportID);
      IHelper::debugAppend(resultEl, "ice transport", transport ? transport->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer::Preference
    #pragma mark

    //-------------------------------------------------------------------------
    ICEGatherer::Preference::Preference() :
      Preference(PreferenceType_Priority)
    {
    }
    
    //-------------------------------------------------------------------------
    ICEGatherer::Preference::Preference(PreferenceTypes type) :
      mType(type)
    {
      mCandidateTypePreferences = new DWORD[IICETypes::CandidateType_Last+1] {};
      mProtocolTypePreferences = new DWORD[IICETypes::Protocol_Last+1] {};
      mInterfaceTypePreferences = new DWORD[ICEGatherer::InterfaceType_Last+1] {};
      mAddressFamilyPreferences = new DWORD[ICEGatherer::AddressFamily_Last+1] {};

      switch (type) {
        case PreferenceType_Priority:
        {
          mCandidateTypePreferences[IICETypes::CandidateType_Host]   = (90 << (24-0));
          mCandidateTypePreferences[IICETypes::CandidateType_Srflex] = (80 << (24-0));
          mCandidateTypePreferences[IICETypes::CandidateType_Prflx]  = (70 << (24-0));
          mCandidateTypePreferences[IICETypes::CandidateType_Relay]  = (60 << (24-0));

          mProtocolTypePreferences[IICETypes::Protocol_UDP] = (1 << (24-2));
          mProtocolTypePreferences[IICETypes::Protocol_TCP] = (0 << (24-2));

          mInterfaceTypePreferences[ICEGatherer::InterfaceType_Unknown] = (1 << (24-6));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_Default] = (0 << (24-6));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_LAN]     = (9 << (24-6));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_Tunnel]  = (5 << (24-6));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_WLAN]    = (8 << (24-6));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_WWAN]    = (7 << (24-6));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_VPN]     = (6 << (24-6));

          mAddressFamilyPreferences[ICEGatherer::AddressFamily_IPv4]    = (1 << (24-8));
          mAddressFamilyPreferences[ICEGatherer::AddressFamily_IPv6]    = (0 << (24-8));
          break;
        }
        case PreferenceType_Unfreeze:
        {
          // happy eyeballs unfreeze order (prefer IPv4)
          mAddressFamilyPreferences[ICEGatherer::AddressFamily_IPv4]    = (1 << (24+6));
          mAddressFamilyPreferences[ICEGatherer::AddressFamily_IPv6]    = (0 << (24+6));

          // most likely to succeed next = udp
          mProtocolTypePreferences[IICETypes::Protocol_UDP] = (1 << (24+4));
          mProtocolTypePreferences[IICETypes::Protocol_TCP] = (0 << (24+4));

          // most likely to succeed = relay
          mCandidateTypePreferences[IICETypes::CandidateType_Host]   = (10 << (24-4));
          mCandidateTypePreferences[IICETypes::CandidateType_Srflex] = (20 << (24-4));
          mCandidateTypePreferences[IICETypes::CandidateType_Prflx]  = (30 << (24-4));
          mCandidateTypePreferences[IICETypes::CandidateType_Relay]  = (40 << (24-4));

          // most likely to succeed = ethernet
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_Unknown] = (1 << (24-8));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_Default] = (0 << (24-8));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_LAN]     = (9 << (24-8));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_Tunnel]  = (5 << (24-8));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_WLAN]    = (8 << (24-8));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_WWAN]    = (7 << (24-8));
          mInterfaceTypePreferences[ICEGatherer::InterfaceType_VPN]     = (6 << (24-8));
          break;
        }
      }
    }

    //-------------------------------------------------------------------------
    ICEGatherer::Preference::~Preference()
    {
      delete [] mCandidateTypePreferences;
      delete [] mProtocolTypePreferences;
      delete [] mInterfaceTypePreferences;
      delete [] mAddressFamilyPreferences;
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::Preference::getSettingsPrefixes(
                                                      const char * &outCandidateType,
                                                      const char * &outProtocolType,
                                                      const char * &outInterfaceType,
                                                      const char * &outAddressFamily
                                                      ) const
    {
      switch (mType) {
        case PreferenceType_Priority: {
          outCandidateType = ORTC_SETTING_GATHERER_CANDIDATE_TYPE_PREFERENCE_PRIORITY_PREFIX;
          outProtocolType = ORTC_SETTING_GATHERER_PROTOCOL_TYPE_PREFERENCE_PRIORITY_PREFIX;
          outInterfaceType = ORTC_SETTING_GATHERER_INTERFACE_TYPE_PREFERENCE_PRIORITY_PREFIX;
          outAddressFamily = ORTC_SETTING_GATHERER_ADDRESS_FAMILY_PREFERENCE_PRIORITY_PREFIX;
          break;
        }
        case PreferenceType_Unfreeze: {
          outCandidateType = ORTC_SETTING_GATHERER_CANDIDATE_TYPE_PREFERENCE_UNFREEZE_PREFIX;
          outProtocolType = ORTC_SETTING_GATHERER_PROTOCOL_TYPE_PREFERENCE_UNFREEZE_PREFIX;
          outInterfaceType = ORTC_SETTING_GATHERER_INTERFACE_TYPE_PREFERENCE_UNFREEZE_PREFIX;
          outAddressFamily = ORTC_SETTING_GATHERER_ADDRESS_FAMILY_PREFERENCE_UNFREEZE_PREFIX;
          break;
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::Preference::load()
    {
      const char *candidateTypeStr = NULL;
      const char *protocolTypeStr = NULL;
      const char *interfaceTypeStr = NULL;
      const char *addressFamilyStr = NULL;
      getSettingsPrefixes(candidateTypeStr, protocolTypeStr, interfaceTypeStr, addressFamilyStr);

      for (auto loop = zsLib::to_underlying(IICETypes::CandidateType_First); loop <= IICETypes::CandidateType_Last; ++loop) {
        mCandidateTypePreferences[loop] = static_cast<DWORD>(ISettings::getUInt((String(candidateTypeStr) + IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop))).c_str()));
        ZS_EVENTING_5(
                      x, i, Detail, IceGathererInitializeInstallPreference, ol, IceGatherer, Initialization,
                      puid, outerObjectId, mOuterObjectID,
                      string, preferenceType, ICEGatherer::toString(mType),
                      string, preference, "candidate type",
                      string, subPreferenceType, IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop)),
                      dword, preferenceValue, mCandidateTypePreferences[loop]
                      );

        ZS_LOG_DEBUG(log("candidate type preference") + ZS_PARAM("type", ICEGatherer::toString(mType)) + ZS_PARAM("candidate type", IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop))) + ZS_PARAM("preference", mCandidateTypePreferences[loop]))
      }
      for (auto loop = zsLib::to_underlying(IICETypes::Protocol_First); loop <= IICETypes::Protocol_Last; ++loop) {
        mProtocolTypePreferences[loop] = static_cast<DWORD>(ISettings::getUInt((String(protocolTypeStr) + IICETypes::toString(static_cast<IICETypes::Protocols>(loop))).c_str()));
        ZS_EVENTING_5(
                      x, i, Detail, IceGathererInitializeInstallPreference, ol, IceGatherer, Initialization,
                      puid, outerObjectId, mOuterObjectID,
                      string, preferenceType, ICEGatherer::toString(mType),
                      string, preference, "protocol type",
                      string, subPreferenceType, IICETypes::toString(static_cast<IICETypes::Protocols>(loop)),
                      dword, preferenceValue, mProtocolTypePreferences[loop]
                      );
        ZS_LOG_DEBUG(log("protocol type preference") + ZS_PARAM("type", ICEGatherer::toString(mType)) + ZS_PARAM("protocol", IICETypes::toString(static_cast<IICETypes::Protocols>(loop))) + ZS_PARAM("preference", mProtocolTypePreferences[loop]))
      }
      for (auto loop = zsLib::to_underlying(ICEGatherer::InterfaceType_First); loop <= ICEGatherer::InterfaceType_Last; ++loop) {
        mInterfaceTypePreferences[loop] = static_cast<DWORD>(ISettings::getUInt((String(interfaceTypeStr) + ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop))).c_str()));
        ZS_EVENTING_5(
                      x, i, Detail, IceGathererInitializeInstallPreference, ol, IceGatherer, Initialization,
                      puid, outerObjectId, mOuterObjectID,
                      string, preferenceType, ICEGatherer::toString(mType),
                      string, preference, "interface type",
                      string, subPreferenceType, ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop)),
                      dword, preferenceValue, mInterfaceTypePreferences[loop]
                      );
        ZS_LOG_DEBUG(log("interface type preference") + ZS_PARAM("type", ICEGatherer::toString(mType)) + ZS_PARAM("interface type", ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop))) + ZS_PARAM("preference", mInterfaceTypePreferences[loop]))
      }
      for (auto loop = zsLib::to_underlying(ICEGatherer::AddressFamily_First); loop <= ICEGatherer::AddressFamily_Last; ++loop) {
        mAddressFamilyPreferences[loop] = static_cast<DWORD>(ISettings::getUInt((String(addressFamilyStr) + ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop))).c_str()));
        ZS_EVENTING_5(
                      x, i, Detail, IceGathererInitializeInstallPreference, ol, IceGatherer, Initialization,
                      puid, outerObjectId, mOuterObjectID,
                      string, preferenceType, ICEGatherer::toString(mType),
                      string, preference, "address family",
                      string, subPreferenceType, ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop)),
                      dword, preferenceValue, mAddressFamilyPreferences[loop]
                      );
        ZS_LOG_DEBUG(log("address family type preference") + ZS_PARAM("type", ICEGatherer::toString(mType)) + ZS_PARAM("address family", ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop))) + ZS_PARAM("preference", mAddressFamilyPreferences[loop]))
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::Preference::save() const
    {
      const char *candidateTypeStr = NULL;
      const char *protocolTypeStr = NULL;
      const char *interfaceTypeStr = NULL;
      const char *addressFamilyStr = NULL;
      getSettingsPrefixes(candidateTypeStr, protocolTypeStr, interfaceTypeStr, addressFamilyStr);

      for (size_t loop = IICETypes::CandidateType_First; loop <= IICETypes::CandidateType_Last; ++loop) {
        ISettings::setUInt((String(candidateTypeStr) + IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop))).c_str(), mCandidateTypePreferences[loop]);
      }
      for (size_t loop = IICETypes::Protocol_First; loop <= IICETypes::Protocol_Last; ++loop) {
        ISettings::setUInt((String(protocolTypeStr) + IICETypes::toString(static_cast<IICETypes::Protocols>(loop))).c_str(), mProtocolTypePreferences[loop]);
      }
      for (size_t loop = ICEGatherer::InterfaceType_First; loop <= ICEGatherer::InterfaceType_Last; ++loop) {
        ISettings::setUInt((String(interfaceTypeStr) + ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop))).c_str(), mInterfaceTypePreferences[loop]);
      }
      for (size_t loop = ICEGatherer::AddressFamily_First; loop <= ICEGatherer::AddressFamily_Last; ++loop) {
        ISettings::setUInt((String(addressFamilyStr) + ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop))).c_str(), mAddressFamilyPreferences[loop]);
      }
    }

    //-------------------------------------------------------------------------
    Log::Params ICEGatherer::Preference::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICEGatherer::Preference");
      IHelper::debugAppend(objectEl, "id", mOuterObjectID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::Preference::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::Preference");

      const char *candidateTypeStr = NULL;
      const char *protocolTypeStr = NULL;
      const char *interfaceTypeStr = NULL;
      const char *addressFamilyStr = NULL;
      getSettingsPrefixes(candidateTypeStr, protocolTypeStr, interfaceTypeStr, addressFamilyStr);

      ElementPtr candidatesEl = Element::create("candidates");
      ElementPtr protocolsEl = Element::create("protocols");
      ElementPtr interfacesEl = Element::create("interfaces");
      ElementPtr addressFamiliesEl = Element::create("addressFamilies");

      for (size_t loop = IICETypes::CandidateType_First; loop <= IICETypes::CandidateType_Last; ++loop) {
        IHelper::debugAppend(candidatesEl, (String(candidateTypeStr) + IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop))).c_str(), mCandidateTypePreferences[loop]);
      }
      for (size_t loop = IICETypes::Protocol_First; loop <= IICETypes::Protocol_Last; ++loop) {
        IHelper::debugAppend(protocolsEl, (String(protocolTypeStr) + IICETypes::toString(static_cast<IICETypes::Protocols>(loop))).c_str(), mProtocolTypePreferences[loop]);
      }
      for (size_t loop = ICEGatherer::InterfaceType_First; loop <= ICEGatherer::InterfaceType_Last; ++loop) {
        IHelper::debugAppend(interfacesEl, (String(interfaceTypeStr) + ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop))).c_str(), mInterfaceTypePreferences[loop]);
      }
      for (size_t loop = ICEGatherer::AddressFamily_First; loop <= ICEGatherer::AddressFamily_Last; ++loop) {
        IHelper::debugAppend(addressFamiliesEl, (String(addressFamilyStr) + ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop))).c_str(), mAddressFamilyPreferences[loop]);
      }

      resultEl->adoptAsLastChild(candidatesEl);
      resultEl->adoptAsLastChild(protocolsEl);
      resultEl->adoptAsLastChild(interfacesEl);
      resultEl->adoptAsLastChild(addressFamiliesEl);

      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IICEGathererFactory &IICEGathererFactory::singleton()
    {
      return ICEGathererFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr IICEGathererFactory::create(
                                               IICEGathererDelegatePtr delegate,
                                               const IICEGathererTypes::Options &options
                                               )
    {
      if (this) {}
      return internal::ICEGatherer::create(delegate, options);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (helpers)
  #pragma mark

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IICEGathererTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::States
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICEGathererTypes::toString(States state)
  {
    switch (state) {
      case State_New:         return "new";
      case State_Gathering:   return "gathering";
      case State_Complete:    return "complete";
      case State_Closed:      return "closed";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICEGathererTypes::States IICEGathererTypes::toState(const char *state) throw (InvalidParameters)
  {
    String stateStr(state);
    for (IICEGathererTypes::States index = IICEGathererTypes::State_First; index <= IICEGathererTypes::State_Last; index = static_cast<IICEGathererTypes::States>(static_cast<std::underlying_type<IICEGathererTypes::States>::type>(index) + 1)) {
      if (0 == stateStr.compareNoCase(IICEGathererTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + stateStr)
    return IICEGathererTypes::State_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::FilterPolicies
  #pragma mark

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark PolicyBitInfo
    #pragma mark

    struct PolicyBitInfo
    {
      IICEGathererTypes::FilterPolicies mPolicyBits;
      const char *mPolicyStr;

      //-----------------------------------------------------------------------
      static PolicyBitInfo *getInfos() {
        static PolicyBitInfo filterInfos[] {
          {IICEGathererTypes::FilterPolicy_NoIPv4Host,       "NoIPv4Host"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Srflx,      "NoIPv4Srflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Prflx,      "NoIPv4Prflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Relay,      "NoIPv4Relay"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Private,    "NoIPv4Private"},
          {IICEGathererTypes::FilterPolicy_NoIPv4,           "NoIPv4"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Host,       "NoIPv6Host"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Srflx,      "NoIPv6Srflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Prflx,      "NoIPv6Prflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Relay,      "NoIPv6Relay"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Private,    "NoIPv6Private"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Tunnel,     "NoIPv6Tunnel"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Permanent,  "NoIPv6Permanent"},
          {IICEGathererTypes::FilterPolicy_NoIPv6,           "NoIPv6"},
          {IICEGathererTypes::FilterPolicy_NoHost,           "NoHost"},
          {IICEGathererTypes::FilterPolicy_NoSrflx,          "NoSrflx"},
          {IICEGathererTypes::FilterPolicy_NoPrflx,          "NoPrflx"},
          {IICEGathererTypes::FilterPolicy_NoRelay,          "NoRelay"},
          {IICEGathererTypes::FilterPolicy_NoPrivate,        "NoPrivate"},
          {IICEGathererTypes::FilterPolicy_RelayOnly,        "RelayOnly"},
          {IICEGathererTypes::FilterPolicy_NoCandidates,     "NoCandidates"},
          {IICEGathererTypes::FilterPolicy_None,             NULL}
        };
        return filterInfos;
      }
    };

  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::toString(FilterPolicies policies)
  {
    typedef std::list<const char *> PolicyList;

    PolicyList policyList;

    auto filterInfos = internal::PolicyBitInfo::getInfos();

    while (0 != policies) {
      IICEGathererTypes::FilterPolicies mostSpecific = FilterPolicy_None;
      const char *found = NULL;

      for (size_t loop = 0; NULL != filterInfos[loop].mPolicyStr; ++loop) {
        if (filterInfos[loop].mPolicyBits == (policies & filterInfos[loop].mPolicyBits)) {
          if (mostSpecific < filterInfos[loop].mPolicyBits) {
            mostSpecific = filterInfos[loop].mPolicyBits;
            found = filterInfos[loop].mPolicyStr;
          }
        }
      }

      if (FilterPolicy_None == mostSpecific) break;

      // screen out processed filter
      policies = static_cast<FilterPolicies>(policies & (FilterPolicy_NoCandidates ^ mostSpecific));

      policyList.push_front(found);
    }

    String policyStr;
    for (auto iter = policyList.begin(); iter != policyList.end(); ++iter) {
      if (policyStr.hasData()) {
        policyStr += ",";
      }
      policyStr += (*iter);
    }

    return policyStr;
  }

  //---------------------------------------------------------------------------
  IICEGathererTypes::FilterPolicies IICEGathererTypes::toPolicy(const char *filters)
  {
    String policyStr(filters);

    IHelper::SplitMap splits;
    IHelper::split(policyStr, splits, ',');

    IICEGathererTypes::FilterPolicies result = FilterPolicy_None;

    auto filterInfos = internal::PolicyBitInfo::getInfos();

    for (auto iter = splits.begin(); iter != splits.end(); ++iter) {
      for (size_t loop = 0; NULL != filterInfos[loop].mPolicyStr; ++loop) {
        auto str = (*iter).second;
        if (0 == str.compareNoCase(filterInfos[loop].mPolicyStr)) {
          result = static_cast<FilterPolicies>(result | filterInfos[loop].mPolicyBits);
          break;
        }
      }
    }

    return result;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::CredentialType
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICEGathererTypes::toString(CredentialTypes type)
  {
    switch (type) {
      case CredentialType_Password:     return "password";
      case CredentialType_Token:        return "token";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICEGathererTypes::CredentialTypes IICEGathererTypes::toCredentialType(const char *type) throw (InvalidParameters)
  {
    String typeStr(type);
    for (IICEGathererTypes::CredentialTypes index = IICEGathererTypes::CredentialType_First; index <= IICEGathererTypes::CredentialType_Last; index = static_cast<IICEGathererTypes::CredentialTypes>(static_cast<std::underlying_type<IICEGathererTypes::CredentialTypes>::type>(index) + 1)) {
      if (0 == typeStr.compareNoCase(IICEGathererTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + typeStr)
    return IICEGathererTypes::CredentialType_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::Options
  #pragma mark

  //---------------------------------------------------------------------------
  IICEGathererTypes::Options::Options(ElementPtr elem)
  {
    if (!elem) return;

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("continuousGathering"));
      if (str.hasData()) {
        try {
          mContinuousGathering = Numeric<decltype(mContinuousGathering)>(str);
        } catch(const Numeric<decltype(mContinuousGathering)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("continuous gathering value out of range") + ZS_PARAM("value", str))
        }
      }
    }

    ElementPtr interfacePoliciesEl = elem->findFirstChildElement("interfacePolicies");

    if (interfacePoliciesEl) {
      ElementPtr interfacePolicyEl = interfacePoliciesEl->findFirstChildElement("interfacePolicy");
      while (interfacePolicyEl) {
        InterfacePolicy policy(interfacePolicyEl);
        mInterfacePolicies.push_back(policy);
        interfacePolicyEl = interfacePolicyEl->findNextSiblingElement("interfacePolicy");
      }
    } else {
      String gatherPolicy = IHelper::getElementText(elem->findFirstChildElement("gatherPolicy"));
      if (gatherPolicy.hasData()) {
        InterfacePolicy policy;
        if ("all" == gatherPolicy) {
          policy.mGatherPolicy = FilterPolicy_None;
        } else if ("nohost" == gatherPolicy) {
          policy.mGatherPolicy = FilterPolicy_NoHost;
        } else if ("relay" == gatherPolicy) {
          policy.mGatherPolicy = FilterPolicy_RelayOnly;
        } else if ("public" == gatherPolicy) {
          policy.mGatherPolicy = FilterPolicy_NoPrivate;
        }
        mInterfacePolicies.push_back(policy);
      }
    }

    ElementPtr serversEl = elem->findFirstChildElement("iceServers");

    if (serversEl) {
      ElementPtr serverEl = interfacePoliciesEl->findFirstChildElement("iceServer");
      while (serverEl) {
        Server server(serverEl);
        mICEServers.push_back(server);
        serverEl = serverEl->findNextSiblingElement("iceServer");
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::Options::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    elem->adoptAsLastChild(IHelper::createElementWithNumber("continuousGathering",  string(mContinuousGathering)));

    if (mInterfacePolicies.size() > 0) {
      ElementPtr interfacePoliciesEl = Element::create("interfacePolicies");

      for (auto iter = mInterfacePolicies.begin(); iter != mInterfacePolicies.end(); ++iter)
      {
        auto &policy = (*iter);

        interfacePoliciesEl->adoptAsLastChild(policy.createElement());
      }
      elem->adoptAsLastChild(interfacePoliciesEl);
    }
    if (mICEServers.size() > 0) {
      ElementPtr iceServersEl = Element::create("iceServers");

      for (auto iter = mICEServers.begin(); iter != mICEServers.end(); ++iter)
      {
        auto &server = (*iter);

        iceServersEl->adoptAsLastChild(server.createElement());
      }
      elem->adoptAsLastChild(iceServersEl);
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::Options::toDebug() const
  {
    return createElement("ortc::IICEGathererTypes::Options");
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::Options::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update(mContinuousGathering ? "Options:true:policy:" : "Options:false:policy:");
    for (auto iter = mInterfacePolicies.begin(); iter != mInterfacePolicies.end(); ++iter) {
      auto policy = (*iter);
      hasher->update(policy.hash());
      hasher->update(":");
    }
    hasher->update("servers:");
    for (auto iter = mICEServers.begin(); iter != mICEServers.end(); ++iter) {
      auto server = (*iter);
      hasher->update(server.hash());
      hasher->update(":");
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::Server
  #pragma mark

  //---------------------------------------------------------------------------
  IICEGathererTypes::Server::Server(ElementPtr elem)
  {
    if (!elem) return;

    ElementPtr urlsEl = elem->findFirstChildElement("urls");

    if (urlsEl) {
      ElementPtr urlEl = urlsEl->findFirstChildElement("url");
      if (urlEl) {
        while (urlEl) {
          mURLs.push_back(IHelper::getElementTextAndDecode(urlEl));
          urlEl = urlEl->findNextSiblingElement("iceServer");
        }
      } else {
        mURLs.push_back(IHelper::getElementTextAndDecode(urlsEl));
      }
    }

    mUserName = IHelper::getElementTextAndDecode(elem->findFirstChildElement("username"));
    mCredential = IHelper::getElementTextAndDecode(elem->findFirstChildElement("credential"));

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("credentialType"));
      if (str.hasData()) {
        try {
          mCredentialType = IICEGathererTypes::toCredentialType(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("credential type is invalid") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::Server::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mURLs.size() > 0) {
      if (mURLs.size() > 1) {
        ElementPtr urlsEl = Element::create("urls");

        for (auto iter = mURLs.begin(); iter != mURLs.end(); ++iter) {
          auto url = (*iter);
          urlsEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("url", url));
        }

        elem->adoptAsLastChild(urlsEl);
      } else {
        auto url = mURLs.front();
        elem->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("urls",  url));
      }
    }

    if (mUserName.hasData()) {
      elem->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("username",  mUserName));
    }
    if (mCredential.hasData()) {
      elem->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("credential",  mCredential));
    }

    elem->adoptAsLastChild(IHelper::createElementWithText("credentialType", toString(mCredentialType)));

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::Server::toDebug() const
  {
    return createElement("ortc::IICEGathererTypes::Server");
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::Server::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("Server:");
    for (auto iter = mURLs.begin(); iter != mURLs.end(); ++iter) {
      auto url = (*iter);
      hasher->update(url);
      hasher->update(":");
    }
    hasher->update("credentials:");
    hasher->update(mUserName);
    hasher->update(":");
    hasher->update(mCredential);
    hasher->update(":");
    hasher->update(toString(mCredentialType));

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::InterfacePolicy
  #pragma mark

  //---------------------------------------------------------------------------
  IICEGathererTypes::InterfacePolicy::InterfacePolicy(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IICEGathererTypes::InterfacePolicy", "interfaceType", mInterfaceType);

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("gatherPolicy"));
      if (str.hasData()) {
        try {
          mGatherPolicy = toPolicy(str);
        }
        catch (const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("gather policy not valid") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::InterfacePolicy::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "interfaceType", mInterfaceType, false);
    UseHelper::adoptElementValue(elem, "gatherPolicy", IICEGathererTypes::toString(mGatherPolicy), false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::InterfacePolicy::toDebug() const
  {
    return createElement("ortc::IICEGathererTypes::InterfacePolicy");
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::InterfacePolicy::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("InterfacePolicy:");
    hasher->update(mInterfaceType);
    hasher->update(":");
    hasher->update(toString(mGatherPolicy));
    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes::ErrorEvent
  #pragma mark

  //---------------------------------------------------------------------------
  IICEGathererTypes::ErrorEvent::ErrorEvent(ElementPtr elem)
  {
    if (!elem) return;

    {
      ElementPtr candidateEl = elem->findFirstChildElement("hostCandidate");
      if (candidateEl) {
        CandidatePtr candidate = make_shared<Candidate>(candidateEl);
        if (!candidate->ip().isEmpty()) {
          mHostCandidate = candidate;
        }
      }
    }

    UseHelper::getElementValue(elem, "ortc::ErrorEvent", "url", mURL);
    UseHelper::getElementValue(elem, "ortc::ErrorEvent", "errorCode", mErrorCode);
    UseHelper::getElementValue(elem, "ortc::ErrorEvent", "errorText", mErrorText);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::ErrorEvent::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mHostCandidate) {
      elem->adoptAsLastChild(mHostCandidate->createElement("hostCandidate"));
    }
    UseHelper::adoptElementValue(elem, "url", mURL, false);
    if (0 != mErrorCode) {
      UseHelper::adoptElementValue(elem, "errorCode", mErrorCode);
    }
    UseHelper::adoptElementValue(elem, "errorText", mErrorText, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::ErrorEvent::toDebug() const
  {
    return createElement("ortc::IICEGathererTypes::ErrorEvent");
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::ErrorEvent::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ErrorEvent:");
    if (mHostCandidate) {
      hasher->update(mHostCandidate->hash());
    }
    hasher->update(":");
    hasher->update(mURL);
    hasher->update(":");
    hasher->update(mErrorCode);
    hasher->update(":");
    hasher->update(mErrorText);
    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGatherer
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IICEGatherer::toDebug(IICEGathererPtr gatherer)
  {
    return internal::ICEGatherer::toDebug(internal::ICEGatherer::convert(gatherer));
  }

  //---------------------------------------------------------------------------
  IICEGathererPtr IICEGatherer::create(
                                       IICEGathererDelegatePtr delegate,
                                       const Options &options
                                       )
  {
    return internal::IICEGathererFactory::singleton().create(delegate, options);
  }
}
