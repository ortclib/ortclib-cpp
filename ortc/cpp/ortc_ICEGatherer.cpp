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
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>
#include <openpeer/services/ISettings.h>
#include <openpeer/services/ISTUNDiscovery.h>
#include <openpeer/services/ITURNSocket.h>

#include <zsLib/Numeric.h>
#include <zsLib/Timer.h>
#include <zsLib/XML.h>

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
#endif //HAVE_IPHLPAPI_H

#ifdef WINRT
using namespace Windows::Networking::Connectivity;
#endif //WINRT

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif //__APPLE__

#define ORTC_ICEGATHERER_TO_ORDER(xInterfaceType, xOrder) ((((ULONG)xInterfaceType)*100)+xOrder)

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)

  using zsLib::Numeric;
  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
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
    #pragma mark IICEGathererForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IICEGathererForSettings::applyDefaults()
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

      String mapping = UseServicesHelper::toString(rootEl);

      UseSettings::setString(ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING, mapping);
      UseSettings::setUInt(ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH, (24*8)/5); // must be at least 24 bits
      UseSettings::setUInt(ORTC_SETTING_GATHERER_PASSWORD_LENGTH, (128*8)/5);     // must be at least 128 bits

      UseSettings::setBool(ORTC_SETTING_GATHERER_CREATE_TCP_CANDIDATES, true);
      UseSettings::setString(ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER, "/1,*2:120///");

      {
        ICEGatherer::Preference pref(ICEGatherer::PreferenceType_Priority);
        pref.save();
      }
      {
        ICEGatherer::Preference pref(ICEGatherer::PreferenceType_Unfreeze);
        pref.save();
      }

      UseSettings::setUInt(ORTC_SETTING_GATHERER_DEFAULT_CANDIDATES_WARM_UNTIL_IN_SECONDS, 60);
      UseSettings::setUInt(ORTC_SETTING_GATHERER_DEFAULT_STUN_KEEP_ALIVE_IN_SECONDS, 30);
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
        case InternalState_Shutdown:      return IICEGathererTypes::State_Complete;
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
        case InterfaceType_WWAN:      return "wlan";
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
                             IMessageQueuePtr queue,
                             IICEGathererDelegatePtr delegate,
                             const Options &options
                             ) :
      SharedRecursiveLock(SharedRecursiveLock::create()),
      MessageQueueAssociator(queue),
      mUsernameFrag(UseServicesHelper::randomString(UseSettings::getUInt(ORTC_SETTING_GATHERER_USERNAME_FRAG_LENGTH))),
      mPassword(UseServicesHelper::randomString(UseSettings::getUInt(ORTC_SETTING_GATHERER_PASSWORD_LENGTH))),
      mCreateTCPCandidates(UseSettings::getBool(ORTC_SETTING_GATHERER_CREATE_TCP_CANDIDATES)),
      mOptions(options)
    {
      mPreferences[PreferenceType_Priority].load();
      mPreferences[PreferenceType_Unfreeze].load();

      String networkOrder = UseSettings::getString(ORTC_SETTING_GATHERER_INTERFACE_NAME_MAPPING);
      if (networkOrder.hasData()) {

        ElementPtr rootEl = UseServicesHelper::toJSON(networkOrder);
        if (rootEl) {
          ElementPtr interfaceEl = rootEl->getFirstChildElement();
          while (interfaceEl) {
            HostIPSorter::InterfaceNameMappingInfo info = HostIPSorter::InterfaceNameMappingInfo::create(interfaceEl);
            if (info.hasData()) {
              mInterfaceMappings.push_back(info);
            }
            interfaceEl = interfaceEl->getNextSiblingElement();
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::init()
    {
      ZS_LOG_DETAIL(log("created"))
    }

    //-------------------------------------------------------------------------
    ICEGatherer::~ICEGatherer()
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"))
      cancel();
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr ICEGatherer::convert(IICEGathererPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICEGatherer, object);
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr ICEGatherer::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICEGatherer, object);
    }

    //-------------------------------------------------------------------------
    ICEGathererPtr ICEGatherer::convert(ForICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICEGatherer, object);
    }

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
      return ICEGathererPtr();
    }

    //-------------------------------------------------------------------------
    PUID ICEGatherer::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    IICEGathererSubscriptionPtr ICEGatherer::subscribe(IICEGathererDelegatePtr delegate)
    {
      return IICEGathererSubscriptionPtr();
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
      ParametersPtr result(new Parameters);
      result->mUseCandidateFreezePolicy = true;
      result->mUsernameFragment = mUsernameFrag;
      result->mPassword = mPassword;
      return result;
    }

    //-------------------------------------------------------------------------
    IICEGathererTypes::CandidateListPtr ICEGatherer::getLocalCandidates() const
    {
      return CandidateListPtr();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::gather(const Options &options)
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGatherer => IICEGathererForICETransport
    #pragma mark

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
      ZS_LOG_TRACE(log("on wake"))
      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    Log::Params ICEGatherer::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICEGatherer");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
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
      ElementPtr resultEl = Element::create("ortc::ICEGatherer");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "internal state", toString(mCurrentState));
      UseServicesHelper::debugAppend(resultEl, "state", IICEGathererTypes::toString(toState(mCurrentState)));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      return resultEl;
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

      ZS_LOG_DEBUG(debug("step"))

      if (InternalState_Pending == mCurrentState) setState(InternalState_Gathering);

      if (!stepCalculateOptionsHash()) goto done;
      if (!stepResolveHostIPs()) goto done;
      if (!stepGetHostIPs()) goto done;
      if (!stepCalculateHostsHash()) goto done;
      if (!stepFixHostPorts()) goto not_complete;
      if (!stepBindHostPorts()) goto not_complete;
      if (!stepWarmth()) goto not_complete;
      if (!stepSetupReflexive()) goto not_complete;

    not_complete:
      setState(InternalState_Gathering);

    done:
      ZS_LOG_TRACE(debug("step complete"))
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCalculateOptionsHash()
    {
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
          ZS_LOG_TRACE(log("waiting for query to resolve") + ZS_PARAM("query", query.mQuery->getID()))
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
            ZS_LOG_TRACE(log("found host ip") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", query.mOriginalData->mInterfaceName) + ZS_PARAM("host", query.mOriginalData->mHostName))

            HostIPSorter::DataPtr data(new HostIPSorter::Data);
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

      if (!mGetLocalIPsNow) {
        ZS_LOG_TRACE(log("no need to check host IPs at this moment"))
        return true;
      }

      mGetLocalIPsNow = false;
      mHostsHash.clear();

#if defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
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

            if (profileName.hasData()) {
              mPendingHostIPs.push_back(HostIPSorter::prepare(profileName, useName, ip, mInterfaceMappings, mOptions));
            } else {
              mPendingHostIPs.push_back(HostIPSorter::prepare(useName, ip, mOptions));
            }
          }
        }
      }

#endif //defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)

#ifdef HAVE_GETADAPTERADDRESSES
      // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx

#undef MALLOC
#undef FREE

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

      // scope: use GetAdaptersAddresses
      {
        ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_FRIENDLY_NAME;

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

                  ip.setPort(mBindPort);

                  String friendlyName(pCurrAddresses->FriendlyName);
                  String description(pCurrAddresses->Description);

                  ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", friendlyName) + ZS_PARAM("description", description))

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

              ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()))

              mResolvedHostIPs.push_back(HostIPSorter::prepare(ip, mOptions));
            }
          }
        }
      }
#endif //0
#endif //HAVE_GETADAPTERADDRESSES

#ifdef HAVE_GETIFADDRS
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

          ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", ifa->ifa_name))

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

      if (mPendingHostIPs.size() > 0) {
        ZS_LOG_TRACE(log("not all host IPs resolved") + ZS_PARAM("pending size", mPendingHostIPs.size()))
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
        return false;
      }

      ZS_LOG_TRACE(log("all host IPs resolved") + ZS_PARAM("size", mResolvedHostIPs.size()))
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepCalculateHostsHash()
    {
      if (mHostsHash.hasData()) {
        ZS_LOG_TRACE(log("host hash already calculated"))
        return true;
      }

      HostIPSorter::sort(mResolvedHostIPs);

      ZS_LOG_DEBUG(log("--- GATHERING HOST IPs: START ---"))

      SHA1Hasher hasher;

      for (auto iter = mResolvedHostIPs.begin(); iter != mResolvedHostIPs.end(); ++iter) {
        auto data = (*iter);
        ZS_LOG_DEBUG(log("found host IP") + ZS_PARAM("ip", data->mIP.string()))

        hasher.update(data->mIP.string());
        hasher.update(":");
      }

      mHostsHash = hasher.final();

      ZS_LOG_DEBUG(log("--- GATHERING HOST IPs: END ---") + ZS_PARAM("hash", mHostsHash))
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepFixHostPorts()
    {
      typedef std::map<IPAddress, HostIPSorter::DataPtr> IPDataMap;

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
        shutdown(*hostPort);
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

        HostPortPtr hostPort(new HostPort);

        hostPort->mHostData = hostData;

        mHostPorts[hostData->mIP] = hostPort;
      }

      ZS_LOG_DEBUG(log("total hosts now set up") + ZS_PARAM("total", mHostPorts.size()))

      mLastFixedHostPortsHostsHash = mHostsHash;
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepBindHostPorts()
    {
      if (mHostsHash == mLastBoundHostPortsHostHash) {
        ZS_LOG_TRACE(log("host ports have not changed (thus no need to bind ports)"))
        return true;
      }

      bool allBound = true;

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        if (hostPort->mBoundOptionsHash == mOptionsHash) {
          ZS_LOG_TRACE(log("already bound udp / tcp socket") + hostPort->toDebug())
          continue;
        }

        if (!hostPort->mBoundUDPSocket) {
          IPAddress bindIP(hostPort->mHostData->mIP);
          hostPort->mBoundUDPSocket = bind(bindIP, IICETypes::Protocol_UDP);
          if (hostPort->mBoundUDPSocket) {
            ZS_LOG_DEBUG(log("successfully bound UDP socket") + ZS_PARAM("bind ip", bindIP.string()))
            hostPort->mBoundUDPIP = bindIP;
            mHostPortSockets[hostPort->mBoundUDPSocket] = hostPort;
            hostPort->mCandidateUDP = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Host, bindIP);
          } else {
            ZS_LOG_WARNING(Debug, log("failed to bind UDP socket") + ZS_PARAM("bind ip", bindIP.string()))
          }
        } else {
          addCandidate(*(hostPort->mHostData), hostPort->mBoundUDPIP, hostPort->mCandidateUDP);
        }

        // scope: create TCP socket
        if (mCreateTCPCandidates) {
          if (!hostPort->mBoundTCPSocket) {
            IPAddress bindIP(hostPort->mHostData->mIP);
            hostPort->mBoundTCPSocket = bind(bindIP, IICETypes::Protocol_TCP);
            if (hostPort->mBoundTCPSocket) {
              ZS_LOG_DEBUG(log("successfully bound TCP socket") + ZS_PARAM("bind ip", bindIP.string()))
              hostPort->mBoundTCPIP = bindIP;
              mHostPortSockets[hostPort->mBoundTCPSocket] = hostPort;
              hostPort->mCandidateTCP = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Host, bindIP, IICETypes::Protocol_TCP);
            } else {
              ZS_LOG_WARNING(Debug, log("failed to bind TCP socket") + ZS_PARAM("bind ip", bindIP.string()))
            }
          } else {
            addCandidate(*(hostPort->mHostData), hostPort->mBoundTCPIP, hostPort->mCandidateTCP);
          }
        }

        if (!hostPort->mBoundUDPSocket) goto host_not_bound;
        if (!mCreateTCPCandidates) goto host_bound;
        if (!hostPort->mBoundTCPSocket) goto host_not_bound;

        goto host_bound;

      host_bound:
        {
          ZS_LOG_TRACE(log("bound udp / tcp socket") + hostPort->toDebug())
          hostPort->mBoundOptionsHash = mOptionsHash;
          continue;
        }

      host_not_bound:
        {
          ZS_LOG_WARNING(Trace, log("failed to bind udp / tcp socket") + hostPort->toDebug())
          allBound = false;
          continue;
        }
      }

      if (allBound) {
        mLastBoundHostPortsHostHash = mHostsHash;

        ZS_LOG_DEBUG(log("all sockets have been bound"))
        if (mBindBackOffTimer) {
          mBindBackOffTimer->cancel();
          mBindBackOffTimer.reset();
        }
        return true;
      }

      if (!mBindBackOffTimer) {
        mBindBackOffTimer = IBackOffTimer::create<Seconds>(UseSettings::getString(ORTC_SETTING_GATHERER_BIND_BACK_OFF_TIMER), mThisWeak.lock());
      }

      ZS_LOG_WARNING(Debug, log("not all sockets bound"))

      // allow creation of reflexive and relay candidates (even though not everything was bound)
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepWarmth()
    {
      Time now = zsLib::now();
      if (Time() != mWarmUntil) {
        if (mWarmUntil <= now) {
          ZS_LOG_DEBUG(log("no longer need to keep candidates warm"))
          mWarmUntil = Time();

          if (mWarmUntilTimer) {
            mWarmUntilTimer->cancel();
            mWarmUntilTimer.reset();
          }
        }
      }

      if (mLastCandidatesWarmUntilOptionsHash == mOptionsHash) {
        ZS_LOG_TRACE(log("candidates warmth has not change"))
        return true;
      }

      Time previousWarmUntil = mWarmUntil;

      mWarmUntil = mOptions.mKeepCandidatesWarmUntil;

      if (Time() == mWarmUntil) {
        mWarmUntil = now + Seconds(UseSettings::getUInt(ORTC_SETTING_GATHERER_DEFAULT_CANDIDATES_WARM_UNTIL_IN_SECONDS));
      }

      if (previousWarmUntil == mWarmUntil) {
        ZS_LOG_DEBUG(log("no change in candidate warmth") + ZS_PARAMIZE(mWarmUntil))
        return true;
      }

      if (Time() != mWarmUntil) {
        if (mWarmUntil <= now) {
          mWarmUntil = Time();
        }
      }

      ZS_LOG_DEBUG(log("candidates are to be kept warm until") + ZS_PARAMIZE(mWarmUntil))

      if (mWarmUntilTimer) {
        mWarmUntilTimer->cancel();
        mWarmUntilTimer.reset();
      }

      if (Time() != mWarmUntil) {
        mWarmUntilTimer = Timer::create(mThisWeak.lock(), mWarmUntil);
        ZS_THROW_UNEXPECTED_ERROR_IF(!mWarmUntilTimer)
        ZS_LOG_DEBUG(log("created timer to keep candidates warm") + ZS_PARAM("timer", mWarmUntilTimer->getID()))
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::stepSetupReflexive()
    {
      typedef std::map<String, bool> HashMap;

      if (Time() == mWarmUntil) {
        ZS_LOG_TRACE(log("no reflexive candidates should be setup at this time (not being kept warm)"))
        return true;
      }

      if (mHostsHash == mLastReflexiveHostsHash) {
        ZS_LOG_TRACE(log("reflexive server options have not changed (thus no need to setup reflexive ports)"))
        return true;
      }

      bool allSetup = true;

      for (auto iter = mHostPorts.begin(); iter != mHostPorts.end(); ++iter) {
        auto hostPort = (*iter).second;

        if (!hostPort->mBoundUDPSocket) {
          ZS_LOG_WARNING(Trace, log("cannot setup reflexive until local socket is bound"))
          allSetup = false;
          continue;
        }

        bool hostPortSetup = true;

        if (hostPort->mReflexiveOptionsHash == mOptionsHash) {
          ZS_LOG_TRACE(log("already setup reflexive candidates"))
          continue;
        }

        HashMap foundServers;

        for (auto iterOptions = mOptions.mICEServers.begin(); iterOptions != mOptions.mICEServers.end(); ++iterOptions) {
          auto server = (*iterOptions);
          if (!isServerType(server, "stun:")) continue;

          String serverHash = server.hash();

          foundServers[serverHash] = true;

          ReflexivePortPtr reflexivePort;

          for (auto iterReflex = hostPort->mReflexivePorts.begin(); iterReflex != hostPort->mReflexivePorts.end(); ++iterReflex) {
            reflexivePort = (*iterReflex);

            if (reflexivePort->mServer.hash() != serverHash) continue;

            if (mOptionsHash != reflexivePort->mOptionsHash) {
              shutdown(*reflexivePort);
              reflexivePort->mOptionsHash.clear();
            }

            goto found_server;
          }

          goto not_found_server;

        not_found_server:
          {
            reflexivePort = ReflexivePortPtr(new ReflexivePort);
            reflexivePort->mServer = server;

            hostPort->mReflexivePorts.push_back(reflexivePort);
          }

        found_server:
          {
            reflexivePort->mOptionsHash = mOptionsHash;

            if (!reflexivePort->mSTUNDiscovery) {
              ZS_LOG_DEBUG(log("setting up stun discovery") + server.toDebug())

              Seconds keepAliveTime;

              auto keepAliveInSeconds = UseSettings::getUInt(ORTC_SETTING_GATHERER_DEFAULT_STUN_KEEP_ALIVE_IN_SECONDS);
              if (0 != keepAliveInSeconds) {
                keepAliveTime = Seconds(keepAliveInSeconds);
              }

              reflexivePort->mSTUNDiscovery = UseSTUNDiscovery::create(UseServicesHelper::getServiceQueue(), mThisWeak.lock(), createDNSLookupString(server, "stun:"), keepAliveTime);
              ZS_THROW_UNEXPECTED_ERROR_IF(!reflexivePort->mSTUNDiscovery)

              mSTUNDiscoveries[reflexivePort->mSTUNDiscovery] = HostAndReflexivePortPair(hostPort, reflexivePort);
            }

            if (!reflexivePort->mSTUNDiscovery->isComplete()) {
              ZS_LOG_TRACE(log("still waiting for stun discovery to complete"))
              hostPortSetup = allSetup = false;
              continue;
            }

            if (reflexivePort->mCandidate) {
              ZS_LOG_TRACE(log("already have candidate"))
              continue;
            }

            IPAddress ip = reflexivePort->mSTUNDiscovery->getMappedAddress();
            if (ip.isAddressEmpty()) {
              ZS_LOG_WARNING(Debug, log("failed to obtain server reflexive address") + hostPort->toDebug() + server.toDebug())
              continue;
            }

            reflexivePort->mCandidate = createCandidate(hostPort->mHostData, IICETypes::CandidateType_Srflex, hostPort->mBoundUDPIP, ip, server);

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

          shutdown(*reflexivePort);
          hostPort->mReflexivePorts.erase(currentReflex);
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
    IICETypes::CandidatePtr ICEGatherer::createCandidate(
                                                         HostIPSorter::DataPtr hostData,
                                                         IICETypes::CandidateTypes candidateType,
                                                         const IPAddress &boundIP,
                                                         IICETypes::Protocols protocol
                                                         )
    {
      CandidatePtr candidate(new Candidate);

      SHA1Hasher hasher;

      hasher.update("foundation:");
      hasher.update(IICETypes::toString(candidateType));
      hasher.update(":");
      hasher.update(boundIP.string(false));
      hasher.update(":");
      hasher.update(IICETypes::toString(protocol));

      String foundation = hasher.final();

      WORD localPreference = 0;

      auto found = mLastLocalPreference.find(foundation);
      if (found != mLastLocalPreference.end()) {
        WORD &preference = (*found).second;
        ++preference;
        localPreference = preference;
      } else {
        mLastLocalPreference[foundation] = localPreference;
      }

      AddressFamilies family = (boundIP.isIPv4() ? AddressFamily_IPv4 : AddressFamily_IPv6);

      candidate->mInterfaceType = toString(hostData->mInterfaceType);
      candidate->mFoundation = foundation;
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

      candidate->mProtocol = protocol;
      candidate->mIP = boundIP.string(false);
      candidate->mPort = boundIP.getPort();
      candidate->mCandidateType = candidateType;

      addCandidate(*hostData, boundIP, candidate);

      return candidate;
    }

    //-------------------------------------------------------------------------
    IICETypes::CandidatePtr ICEGatherer::createCandidate(
                                                         HostIPSorter::DataPtr hostData,
                                                         IICETypes::CandidateTypes candidateType,
                                                         const IPAddress &baseIP,
                                                         const IPAddress &boundIP,
                                                         const Server &server
                                                         )
    {
      IICETypes::Protocols protocol = IICETypes::Protocol_UDP;

      CandidatePtr candidate(new Candidate);

      String serverURL;
      if (server.mURLs.size() > 0) {
        serverURL = server.mURLs.front();
      }

      SHA1Hasher hasher;

      hasher.update("foundation:");
      hasher.update(IICETypes::toString(candidateType));
      hasher.update(":");
      hasher.update(baseIP.string(false));
      hasher.update(":");
      hasher.update(IICETypes::toString(protocol));
      hasher.update(":");
      hasher.update(serverURL);

      String foundation = hasher.final();

      WORD localPreference = 0;

      auto found = mLastLocalPreference.find(foundation);
      if (found != mLastLocalPreference.end()) {
        WORD &preference = (*found).second;
        ++preference;
        localPreference = preference;
      } else {
        mLastLocalPreference[foundation] = localPreference;
      }

      AddressFamilies family = (boundIP.isIPv4() ? AddressFamily_IPv4 : AddressFamily_IPv6);

      candidate->mInterfaceType = toString(hostData->mInterfaceType);
      candidate->mFoundation = foundation;
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

      candidate->mProtocol = protocol;
      candidate->mIP = boundIP.string(false);
      candidate->mPort = boundIP.getPort();
      candidate->mCandidateType = candidateType;
      candidate->mRelatedAddress = baseIP.string(false);
      candidate->mRelatedPort = baseIP.getPort();

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

      if (isFiltered(hostData.mFilterPolicy, ip, *candidate)) {
        ZS_LOG_TRACE(log("this candidate is filtered") + candidate->toDebug())
        return;
      }

      String hash = candidate->hash();

      auto found = mLocalCandidates.find(hash);
      if (found != mLocalCandidates.end()) {
        ZS_LOG_TRACE(log("canadidate already added") + candidate->toDebug())
        return;
      }

      mLocalCandidates[hash] = candidate;

      CandidatePtr sentCandidate(new Candidate);
      *sentCandidate = *candidate;

      ZS_LOG_DEBUG(log("notify local candidate") + candidate->toDebug())
      mSubscriptions.delegate()->onICEGathererLocalCandidate(mThisWeak.lock(), sentCandidate);
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::removeCandidate(CandidatePtr candidate)
    {
      if (!candidate) return;

      String hash = candidate->hash();
      auto found = mLocalCandidates.find(hash);
      if (found == mLocalCandidates.end()) {
        ZS_LOG_TRACE(log("candidate was never notified as local") + candidate->toDebug())
        return;
      }

      CandidatePtr sentCandidate(new Candidate);
      *sentCandidate = *candidate;

      ZS_LOG_DEBUG(log("notify candidate gone") + candidate->toDebug())
      mSubscriptions.delegate()->onICEGathererLocalCandidateGone(mThisWeak.lock(), sentCandidate);

      mLocalCandidates.erase(found);
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
          } else {
            if (0 != (FilterPolicy_NoIPv6Host & policy)) return true;
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
    SocketPtr ICEGatherer::bind(
                                IPAddress &ioBindIP,
                                IICETypes::Protocols protocol
                                )
    {
      ZS_LOG_DEBUG(log("attempting to bind to IP") + ZS_PARAM("ip", ioBindIP.string()))

      SocketPtr socket;

      try {
        switch (protocol) {
          case IICETypes::Protocol_UDP: socket = Socket::createUDP();
          case IICETypes::Protocol_TCP: socket = Socket::createTCP();
        }

        if (0 != mDefaultPort) {
          if (!mBindBackOffTimer) {
            // only bind using the default port while there is no backoff timer
            ioBindIP.setPort(mDefaultPort);
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
          try {
            ZS_LOG_DEBUG(log("attemping to listen for incoming socket connections"))
            socket->listen();
          } catch(Socket::Exceptions::Unspecified &error) {
            ZS_LOG_ERROR(Detail, log("listen error") + ZS_PARAM("error", error.errorCode()))
            socket.reset();
            goto bind_failure;
          }
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
    void ICEGatherer::shutdown(HostPort &hostPort)
    {
      // delete from maps too
    }

    //-------------------------------------------------------------------------
    bool ICEGatherer::hasSTUNServers()
    {
      if (mOptionsHash == mHasSTUNServersHash) {
        return mHasSTUNServers;
      }

      mHasSTUNServersHash = mOptionsHash;
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
      if (mOptionsHash == mHasTURNServersHash) {
        return mHasTURNServers;
      }

      mHasTURNServersHash = mOptionsHash;
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
    String ICEGatherer::createDNSLookupString(
                                              const Server &server,
                                              const char *urlPrefix
                                              ) const
    {
      size_t length = strlen(urlPrefix);

      String result;

      if (server.mURLs.size() < 1) {
        ZS_LOG_INSANE(log("server is not of url search type (no URLs found)") + server.toDebug() + ZS_PARAMIZE(urlPrefix))
        return result;
      }

      for (auto iterURLs = server.mURLs.begin(); iterURLs != server.mURLs.end(); ++iterURLs) {
        const String &url = (*iterURLs);
        if (0 != url.compare(0, length, urlPrefix)) {
          ZS_LOG_INSANE(log("server is not of url search type") + server.toDebug() + ZS_PARAMIZE(url) + ZS_PARAMIZE(urlPrefix))
          continue;
        }

        ZS_LOG_INSANE(log("found server of url search type") + server.toDebug() + ZS_PARAMIZE(url) + ZS_PARAMIZE(urlPrefix))

        String dnsStr = url.substr(length);

        if (result.hasData()) {
          result += "," + dnsStr;
        } else {
          result = dnsStr;
        }
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
    void ICEGatherer::cancel()
    {
      if (isShutdown()) {
        ZS_LOG_TRACE(log("already shutdown"))
        return;
      }

      ZS_LOG_DEBUG(log("cancel"))

      setState(InternalState_ShuttingDown);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();



      ZS_LOG_TRACE(log("cancel complete"))

      setState(InternalState_Shutdown);

      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::setState(InternalStates state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DEBUG(log("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      auto oldState = toState(mCurrentState);
      auto newState = toState(state);

      mCurrentState = state;

      auto pThis = mThisWeak.lock();
      if ((pThis) &&
          (oldState != newState)) {
        ZS_LOG_TRACE(log("reporting state change to delegates") + ZS_PARAM("new state", IICEGathererTypes::toString(newState)) + ZS_PARAM("old state", IICEGathererTypes::toString(oldState)))
        mSubscriptions.delegate()->onICEGathererStateChanged(pThis, newState);
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

      ZS_LOG_WARNING(Detail, log("error set") + ZS_PARAMIZE(mLastError) + ZS_PARAMIZE(mLastErrorReason))
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
      DataPtr data(new Data);
      data->mIP = ip;
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
      DataPtr data(new Data);
      data->mHostName = String(hostName);
      data->mIP = ip;
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
      DataPtr data(new Data);
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
      DataPtr data(new Data);
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
      DataPtr data(new Data);
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
      if (prefs.size() < 1) return;

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
          } catch(Numeric<decltype(ioData.mIndex)>::ValueOutOfRange &) {
            ZS_LOG_WARNING(Detail, slog("number failed to convert") + ZS_PARAM("number", numStr) + ioData.toDebug())
          }
        }
      }

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

        std::regex e(info.mInterfaceNameRegularExpression);

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
          break;
        }
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::HostIPSorter::fixPolicy(
                                              Data &ioData,
                                              const Options &options
                                              )
    {
      FilterPolicies defaultPolicy = FilterPolicy_None;
      for (auto iter = options.mInterfacePolicy.begin(); iter != options.mInterfacePolicy.end(); ++iter) {
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

      UseServicesHelper::debugAppend(resultEl, "interface name", mInterfaceName);
      UseServicesHelper::debugAppend(resultEl, "interface description", mInterfaceDescription);
      UseServicesHelper::debugAppend(resultEl, "interface type", toString(mInterfaceType));
      UseServicesHelper::debugAppend(resultEl, "filter policy", IICEGathererTypes::toString(mFilterPolicy));

      UseServicesHelper::debugAppend(resultEl, "host name", mHostName);
      UseServicesHelper::debugAppend(resultEl, "ip", mIP.string());
      UseServicesHelper::debugAppend(resultEl, "order id", mOrderIndex);
      UseServicesHelper::debugAppend(resultEl, "adapter metric", mAdapterMetric);
      UseServicesHelper::debugAppend(resultEl, "index", mIndex);
      UseServicesHelper::debugAppend(resultEl, "temporary ip", mIsTemporaryIP);

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

      info.mInterfaceNameRegularExpression = UseServicesHelper::getElementTextAndDecode(el->findFirstChildElement("nameRegEx"));
      info.mInterfaceType =  toInterfaceType(UseServicesHelper::getElementTextAndDecode(el->findFirstChildElement("type")));
      String order = UseServicesHelper::getElementText(el->findFirstChildElement("order"));
      try {
        info.mOrderIndex = Numeric<decltype(info.mOrderIndex)>(order);
      } catch(Numeric<decltype(info.mOrderIndex)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Detail, slog("failed to convert expression") + ZS_PARAMIZE(order))
      }

      return info;
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGatherer::HostIPSorter::InterfaceNameMappingInfo::createElement() const
    {
      ElementPtr rootEl = Element::create("interface");

      if (mInterfaceNameRegularExpression.hasData()) {
        rootEl->adoptAsLastChild(UseServicesHelper::createElementWithTextAndJSONEncode("nameRegEx", mInterfaceNameRegularExpression));
      }
      if (InterfaceType_Unknown != mInterfaceType) {
        rootEl->adoptAsLastChild(UseServicesHelper::createElementWithTextAndJSONEncode("type", toString(mInterfaceType)));
      }
      if (0 != mOrderIndex) {
        rootEl->adoptAsLastChild(UseServicesHelper::createElementWithNumber("order", string(mOrderIndex)));
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
      ElementPtr resultEl = Element::create("ortc::ICEGatherer::HostIPSorter::InterfaceNameMappingInfo");

      UseServicesHelper::debugAppend(resultEl, "interface name regex", mInterfaceNameRegularExpression);
      UseServicesHelper::debugAppend(resultEl, "type", toString(mInterfaceType));
      UseServicesHelper::debugAppend(resultEl, "order", mOrderIndex);

      return resultEl;
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

      UseServicesHelper::debugAppend(resultEl, mServer.toDebug());
      UseServicesHelper::debugAppend(resultEl, "stun discovery", UseSTUNDiscovery::toDebug(mSTUNDiscovery));

      UseServicesHelper::debugAppend(resultEl, "options hash", mOptionsHash);
      UseServicesHelper::debugAppend(resultEl, mCandidate ? mCandidate->toDebug() : ElementPtr());

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

      UseServicesHelper::debugAppend(resultEl, mServer.toDebug());
      UseServicesHelper::debugAppend(resultEl, "turn socket", UseTURNSocket::toDebug(mTURNSocket));

      UseServicesHelper::debugAppend(resultEl, "options hash", mOptionsHash);
      UseServicesHelper::debugAppend(resultEl, mCandidate ? mCandidate->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "last send data", mLastSentData);
      UseServicesHelper::debugAppend(resultEl, "inactivity timer", mInactivityTimer ? mInactivityTimer->getID() : 0);

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

      UseServicesHelper::debugAppend(resultEl, mHostData->toDebug());

      UseServicesHelper::debugAppend(resultEl, "options hash", mBoundOptionsHash);

      UseServicesHelper::debugAppend(resultEl, "candidate udp", mCandidateUDP ? mCandidateUDP->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "bound udp ip", mBoundUDPIP.string());
      UseServicesHelper::debugAppend(resultEl, "bound udp socket", (bool)mBoundUDPSocket);

      UseServicesHelper::debugAppend(resultEl, "candidate tcp", mCandidateTCP ? mCandidateTCP->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "bound udp ip", mBoundTCPIP.string());
      UseServicesHelper::debugAppend(resultEl, "bound tcp socket", (bool)mBoundTCPSocket);

      UseServicesHelper::debugAppend(resultEl, "reflexive options hash", mReflexiveOptionsHash);
      UseServicesHelper::debugAppend(resultEl, "reflexive ports", mReflexivePorts.size());

      UseServicesHelper::debugAppend(resultEl, "relay options hash", mRelayOptionsHash);
      UseServicesHelper::debugAppend(resultEl, "relay ports", mRelayPorts.size());

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
    void ICEGatherer::Preference::getSettingsPrefixes(
                                                      const char * &outCandidateType,
                                                      const char * &outProtocolType,
                                                      const char * &outInterfaceType,
                                                      const char * &outAddressFamily
                                                      )
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

      for (size_t loop = IICETypes::CandidateType_First; loop <= IICETypes::CandidateType_Last; ++loop) {
        mCandidateTypePreferences[loop] = static_cast<DWORD>(UseSettings::getUInt((String(candidateTypeStr) + IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop))).c_str()));
      }
      for (size_t loop = IICETypes::Protocol_First; loop <= IICETypes::Protocol_Last; ++loop) {
        mProtocolTypePreferences[loop] = static_cast<DWORD>(UseSettings::getUInt((String(protocolTypeStr) + IICETypes::toString(static_cast<IICETypes::Protocols>(loop))).c_str()));
      }
      for (size_t loop = ICEGatherer::InterfaceType_First; loop <= ICEGatherer::InterfaceType_Last; ++loop) {
        mInterfaceTypePreferences[loop] = static_cast<DWORD>(UseSettings::getUInt((String(interfaceTypeStr) + ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop))).c_str()));
      }
      for (size_t loop = ICEGatherer::AddressFamily_First; loop <= ICEGatherer::AddressFamily_Last; ++loop) {
        mAddressFamilyPreferences[loop] = static_cast<DWORD>(UseSettings::getUInt((String(addressFamilyStr) + ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop))).c_str()));
      }
    }

    //-------------------------------------------------------------------------
    void ICEGatherer::Preference::save()
    {
      const char *candidateTypeStr = NULL;
      const char *protocolTypeStr = NULL;
      const char *interfaceTypeStr = NULL;
      const char *addressFamilyStr = NULL;
      getSettingsPrefixes(candidateTypeStr, protocolTypeStr, interfaceTypeStr, addressFamilyStr);

      for (size_t loop = IICETypes::CandidateType_First; loop <= IICETypes::CandidateType_Last; ++loop) {
        UseSettings::setUInt((String(candidateTypeStr) + IICETypes::toString(static_cast<IICETypes::CandidateTypes>(loop))).c_str(), mCandidateTypePreferences[loop]);
      }
      for (size_t loop = IICETypes::Protocol_First; loop <= IICETypes::Protocol_Last; ++loop) {
        UseSettings::setUInt((String(protocolTypeStr) + IICETypes::toString(static_cast<IICETypes::Protocols>(loop))).c_str(), mProtocolTypePreferences[loop]);
      }
      for (size_t loop = ICEGatherer::InterfaceType_First; loop <= ICEGatherer::InterfaceType_Last; ++loop) {
        UseSettings::setUInt((String(interfaceTypeStr) + ICEGatherer::toString(static_cast<ICEGatherer::InterfaceTypes>(loop))).c_str(), mInterfaceTypePreferences[loop]);
      }
      for (size_t loop = ICEGatherer::AddressFamily_First; loop <= ICEGatherer::AddressFamily_Last; ++loop) {
        UseSettings::setUInt((String(addressFamilyStr) + ICEGatherer::toString(static_cast<ICEGatherer::AddressFamilies>(loop))).c_str(), mAddressFamilyPreferences[loop]);
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

    ElementPtr toDebug();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICEGathererTypes::toString(States state)
  {
    switch (state) {
      case State_New:         return "new";
      case State_Gathering:   return "gathering";
      case State_Complete:    return "complete";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICEGathererTypes::States IICEGathererTypes::toState(const char *state) throw (InvalidParameters)
  {
    static IICEGathererTypes::States states[] =
    {
      State_New,
      State_Gathering,
      State_Complete,
    };

    String stateStr(state);

    for (size_t loop = 0; loop < (sizeof(states) / sizeof(states[0])); ++loop) {
      if (stateStr == toString(states[loop])) {
        return states[loop];
      }
    }

    ZS_THROW_INVALID_ARGUMENT("Invalid parameter value: " + stateStr)
    return State_New;
  }

  namespace internal
  {
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark PolicyBitInfo
    #pragma mark

    struct PolicyBitInfo
    {
      IICEGathererTypes::FilterPolicies mPolicyBits;
      const char *mPolicyStr;

      static PolicyBitInfo *getInfos() {
        static PolicyBitInfo filterInfos[] {
          {IICEGathererTypes::FilterPolicy_NoIPv4Host,       "NoIPv4Host"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Srflx,      "NoIPv4Srflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Prflx,      "NoIPv4Prflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv4Relay,      "NoIPv4Relay"},
          {IICEGathererTypes::FilterPolicy_NoIPv4,           "NoIPv4"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Host,       "NoIPv6Host"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Srflx,      "NoIPv6Srflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Prflx,      "NoIPv6Prflx"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Relay,      "NoIPv6Relay"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Tunnel,     "NoIPv6Tunnel"},
          {IICEGathererTypes::FilterPolicy_NoIPv6Permanent,  "NoIPv6Permanent"},
          {IICEGathererTypes::FilterPolicy_NoIPv6,           "NoIPv6"},
          {IICEGathererTypes::FilterPolicy_NoHost,           "NoHost"},
          {IICEGathererTypes::FilterPolicy_NoSrflx,          "NoSrflx"},
          {IICEGathererTypes::FilterPolicy_NoPrflx,          "NoPrflx"},
          {IICEGathererTypes::FilterPolicy_NoRelay,          "NoRelay"},
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

    UseServicesHelper::SplitMap splits;
    UseServicesHelper::split(policyStr, splits, ',');

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
  ElementPtr IICEGathererTypes::Options::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICEGathererTypes::Options");

    UseServicesHelper::debugAppend(resultEl, "continuous gathering", mContinuousGathering);

    if (mICEServers.size() > 0) {
      ElementPtr serversEl = Element::create("servers");

      for (auto iter = mICEServers.begin(); iter != mICEServers.end(); ++iter) {
        auto server = (*iter);
        UseServicesHelper::debugAppend(serversEl, server.toDebug());
      }
      UseServicesHelper::debugAppend(resultEl, serversEl);
    }

    UseServicesHelper::debugAppend(resultEl, "candidates warm until", mKeepCandidatesWarmUntil);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::Options::hash() const
  {
    SHA1Hasher hasher;

    hasher.update(mContinuousGathering ? "Options:true:policy:" : "Options:false:policy:");
    for (auto iter = mInterfacePolicy.begin(); iter != mInterfacePolicy.end(); ++iter) {
      auto policy = (*iter);
      hasher.update(policy.hash());
      hasher.update(":");
    }
    hasher.update("servers:");
    for (auto iter = mICEServers.begin(); iter != mICEServers.end(); ++iter) {
      auto server = (*iter);
      hasher.update(server.hash());
      hasher.update(":");
    }

    hasher.update(string(mKeepCandidatesWarmUntil));

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::Server::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICEGathererTypes::Server");

    if (mURLs.size() > 0) {
      ElementPtr urlsEl = Element::create("urls");

      for (auto iter = mURLs.begin(); iter != mURLs.end(); ++iter) {
        auto url = (*iter);
        UseServicesHelper::debugAppend(urlsEl, "url", url);
      }
      UseServicesHelper::debugAppend(resultEl, urlsEl);
    }

    UseServicesHelper::debugAppend(resultEl, "username", mUserName);
    UseServicesHelper::debugAppend(resultEl, "credential", mCredential);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::Server::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("Server:");
    for (auto iter = mURLs.begin(); iter != mURLs.end(); ++iter) {
      auto url = (*iter);
      hasher.update(url);
      hasher.update(":");
    }
    hasher.update("credentials:");
    hasher.update(mUserName);
    hasher.update(":");
    hasher.update(mCredential);

    return hasher.final();
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IICEGathererTypes::InterfacePolicy::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICEGathererTypes::InterfacePolicy");

    UseServicesHelper::debugAppend(resultEl, "interface type", mInterfaceType);
    UseServicesHelper::debugAppend(resultEl, "gather policy", toString(mGatherPolicy));

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICEGathererTypes::InterfacePolicy::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("InterfacePolicy:");
    hasher.update(mInterfaceType);
    hasher.update(":");
    hasher.update(toString(mGatherPolicy));
    return hasher.final();
  }
  
}
