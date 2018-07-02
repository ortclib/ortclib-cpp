
#include "impl_org_webRtc_enums.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtcerror.h"
#include "rtc_base/sslidentity.h"
#include "api/rtptransceiverinterface.h"
#include "media/base/videocapturer.h"
#include "api/stats/rtcstats_objects.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/Stringize.h>

using ::zsLib::String;
using ::zsLib::Optional;
using ::zsLib::Any;
using ::zsLib::AnyPtr;
using ::zsLib::AnyHolder;
using ::zsLib::Promise;
using ::zsLib::PromisePtr;
using ::zsLib::PromiseWithHolder;
using ::zsLib::PromiseWithHolderPtr;
using ::zsLib::eventing::SecureByteBlock;
using ::zsLib::eventing::SecureByteBlockPtr;
using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::make_shared;
using ::std::list;
using ::std::set;
using ::std::map;

static const char kSeparator = '_';

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);

namespace wrapper { namespace impl { namespace org { namespace webRtc { ZS_DECLARE_SUBSYSTEM(wrapper_org_webRtc); } } } }

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCErrorType UseEnum::toWrapper(::webrtc::RTCErrorType value) noexcept
{
  switch (value) {
    case ::webrtc::RTCErrorType::NONE:                      return wrapper::org::webRtc::RTCErrorType::RTCErrorType_none;
    case ::webrtc::RTCErrorType::UNSUPPORTED_OPERATION:     return wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedOperation;
    case ::webrtc::RTCErrorType::UNSUPPORTED_PARAMETER:     return wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedParameter;
    case ::webrtc::RTCErrorType::INVALID_PARAMETER:         return wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidParameter;
    case ::webrtc::RTCErrorType::INVALID_RANGE:             return wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidRange;
    case ::webrtc::RTCErrorType::SYNTAX_ERROR:              return wrapper::org::webRtc::RTCErrorType::RTCErrorType_syntaxError;
    case ::webrtc::RTCErrorType::INVALID_STATE:             return wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidState;
    case ::webrtc::RTCErrorType::INVALID_MODIFICATION:      return wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidModification;
    case ::webrtc::RTCErrorType::NETWORK_ERROR:             return wrapper::org::webRtc::RTCErrorType::RTCErrorType_networkError;
    case ::webrtc::RTCErrorType::RESOURCE_EXHAUSTED:        return wrapper::org::webRtc::RTCErrorType::RTCErrorType_resourceExhausted;
    case ::webrtc::RTCErrorType::INTERNAL_ERROR:            return wrapper::org::webRtc::RTCErrorType::RTCErrorType_internalError;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCErrorType::RTCErrorType_none;
}

//-----------------------------------------------------------------------------
::webrtc::RTCErrorType UseEnum::toNative(wrapper::org::webRtc::RTCErrorType value) noexcept
{
  switch (value) {
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_none:                   return ::webrtc::RTCErrorType::NONE;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedOperation:   return ::webrtc::RTCErrorType::UNSUPPORTED_OPERATION;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedParameter:   return ::webrtc::RTCErrorType::UNSUPPORTED_PARAMETER;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidParameter:       return ::webrtc::RTCErrorType::INVALID_PARAMETER;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidRange:           return ::webrtc::RTCErrorType::INVALID_RANGE;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_syntaxError:            return ::webrtc::RTCErrorType::SYNTAX_ERROR;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidState:           return ::webrtc::RTCErrorType::INVALID_STATE;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidModification:    return ::webrtc::RTCErrorType::INVALID_MODIFICATION;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_networkError:           return ::webrtc::RTCErrorType::NETWORK_ERROR;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_resourceExhausted:      return ::webrtc::RTCErrorType::RESOURCE_EXHAUSTED;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_internalError:          return ::webrtc::RTCErrorType::INTERNAL_ERROR;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::RTCErrorType::INTERNAL_ERROR;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceTransportPolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::IceTransportsType value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::IceTransportsType::kNone:   return wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_none;
    case ::webrtc::PeerConnectionInterface::IceTransportsType::kRelay:  return wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_relay;
    case ::webrtc::PeerConnectionInterface::IceTransportsType::kNoHost: return wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_noHost;
    case ::webrtc::PeerConnectionInterface::IceTransportsType::kAll:    return wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_all;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_all;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::IceTransportsType UseEnum::toNative(wrapper::org::webRtc::RTCIceTransportPolicy value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_none:   return ::webrtc::PeerConnectionInterface::IceTransportsType::kNone;
    case wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_relay:  return ::webrtc::PeerConnectionInterface::IceTransportsType::kRelay;
    case wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_noHost: return ::webrtc::PeerConnectionInterface::IceTransportsType::kNoHost;
    case wrapper::org::webRtc::RTCIceTransportPolicy::RTCIceTransportPolicy_all:    return ::webrtc::PeerConnectionInterface::IceTransportsType::kAll;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::IceTransportsType::kAll;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCBundlePolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::BundlePolicy value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyBalanced:    return wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_balanced;
    case ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyMaxBundle:   return wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_maxBundle;
    case ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyMaxCompat:   return wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_maxCompat;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_balanced;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::BundlePolicy UseEnum::toNative(wrapper::org::webRtc::RTCBundlePolicy value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_balanced:   return ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyBalanced;
    case wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_maxBundle:  return ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyMaxBundle;
    case wrapper::org::webRtc::RTCBundlePolicy::RTCBundlePolicy_maxCompat:  return ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyMaxCompat;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyBalanced;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtcpMuxPolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::RtcpMuxPolicy value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyNegotiate:   return wrapper::org::webRtc::RTCRtcpMuxPolicy::RTCRtcpMuxPolicy_negotiate;
    case ::webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyRequire:     return wrapper::org::webRtc::RTCRtcpMuxPolicy::RTCRtcpMuxPolicy_require;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCRtcpMuxPolicy::RTCRtcpMuxPolicy_require;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::RtcpMuxPolicy UseEnum::toNative(wrapper::org::webRtc::RTCRtcpMuxPolicy value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCRtcpMuxPolicy::RTCRtcpMuxPolicy_negotiate:  return ::webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyNegotiate;
    case wrapper::org::webRtc::RTCRtcpMuxPolicy::RTCRtcpMuxPolicy_require:    return ::webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyRequire;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyRequire;
}


//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCTcpCandidatePolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::TcpCandidatePolicy value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::TcpCandidatePolicy::kTcpCandidatePolicyEnabled:   return wrapper::org::webRtc::RTCTcpCandidatePolicy::RTCTcpCandidatePolicy_enabled;
    case ::webrtc::PeerConnectionInterface::TcpCandidatePolicy::kTcpCandidatePolicyDisabled:  return wrapper::org::webRtc::RTCTcpCandidatePolicy::RTCTcpCandidatePolicy_disabled;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCTcpCandidatePolicy::RTCTcpCandidatePolicy_disabled;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::TcpCandidatePolicy UseEnum::toNative(wrapper::org::webRtc::RTCTcpCandidatePolicy value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCTcpCandidatePolicy::RTCTcpCandidatePolicy_enabled:    return ::webrtc::PeerConnectionInterface::TcpCandidatePolicy::kTcpCandidatePolicyEnabled;
    case wrapper::org::webRtc::RTCTcpCandidatePolicy::RTCTcpCandidatePolicy_disabled:   return ::webrtc::PeerConnectionInterface::TcpCandidatePolicy::kTcpCandidatePolicyDisabled;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::TcpCandidatePolicy::kTcpCandidatePolicyDisabled;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCCandidateNetworkPolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::CandidateNetworkPolicy value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy::kCandidateNetworkPolicyAll:       return wrapper::org::webRtc::RTCCandidateNetworkPolicy::RTCCandidateNetworkPolicy_all;
    case ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy::kCandidateNetworkPolicyLowCost:   return wrapper::org::webRtc::RTCCandidateNetworkPolicy::RTCCandidateNetworkPolicy_lowCost;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCCandidateNetworkPolicy::RTCCandidateNetworkPolicy_all;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::CandidateNetworkPolicy UseEnum::toNative(wrapper::org::webRtc::RTCCandidateNetworkPolicy value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCCandidateNetworkPolicy::RTCCandidateNetworkPolicy_all:      return ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy::kCandidateNetworkPolicyAll;
    case wrapper::org::webRtc::RTCCandidateNetworkPolicy::RTCCandidateNetworkPolicy_lowCost:  return ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy::kCandidateNetworkPolicyLowCost;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy::kCandidateNetworkPolicyLowCost;
}


//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCContinualGatheringPolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::ContinualGatheringPolicy value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy::GATHER_ONCE:        return wrapper::org::webRtc::RTCContinualGatheringPolicy::RTCContinualGatheringPolicy_once;
    case ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy::GATHER_CONTINUALLY: return wrapper::org::webRtc::RTCContinualGatheringPolicy::RTCContinualGatheringPolicy_continually;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCContinualGatheringPolicy::RTCContinualGatheringPolicy_once;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::ContinualGatheringPolicy UseEnum::toNative(wrapper::org::webRtc::RTCContinualGatheringPolicy value) noexcept
{
  switch (value)
  {
    case  wrapper::org::webRtc::RTCContinualGatheringPolicy::RTCContinualGatheringPolicy_once:        return ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy::GATHER_ONCE;
    case  wrapper::org::webRtc::RTCContinualGatheringPolicy::RTCContinualGatheringPolicy_continually: return ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy::GATHER_CONTINUALLY;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy::GATHER_ONCE;
}


//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCNetworkType UseEnum::toWrapper(::rtc::AdapterType value) noexcept
{
  switch (value)
  {
    case ::rtc::AdapterType::ADAPTER_TYPE_UNKNOWN:    return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_unknown;
    case ::rtc::AdapterType::ADAPTER_TYPE_ETHERNET:   return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_ethernet;
    case ::rtc::AdapterType::ADAPTER_TYPE_WIFI:       return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wifi;
    case ::rtc::AdapterType::ADAPTER_TYPE_CELLULAR:   return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_cellular;
    case ::rtc::AdapterType::ADAPTER_TYPE_VPN:        return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_vpn;
    case ::rtc::AdapterType::ADAPTER_TYPE_LOOPBACK:   return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_loopback;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_unknown;
}

//-----------------------------------------------------------------------------
::rtc::AdapterType UseEnum::toNative(wrapper::org::webRtc::RTCNetworkType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_unknown:    return ::rtc::AdapterType::ADAPTER_TYPE_UNKNOWN;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_ethernet:   return ::rtc::AdapterType::ADAPTER_TYPE_ETHERNET;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wifi:       return ::rtc::AdapterType::ADAPTER_TYPE_WIFI;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_cellular:   return ::rtc::AdapterType::ADAPTER_TYPE_CELLULAR;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_vpn:        return ::rtc::AdapterType::ADAPTER_TYPE_VPN;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_loopback:   return ::rtc::AdapterType::ADAPTER_TYPE_LOOPBACK;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_bluetooth:  return ::rtc::AdapterType::ADAPTER_TYPE_UNKNOWN;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wimax:      return ::rtc::AdapterType::ADAPTER_TYPE_UNKNOWN;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::rtc::AdapterType::ADAPTER_TYPE_UNKNOWN;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCNetworkType UseEnum::toWrapperRTCNetworkType(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCNetworkType enums[] = {
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_unknown,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_ethernet,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wifi,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_cellular,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_vpn,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_loopback,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_bluetooth,
    wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wimax
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("RTCNetworkType is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCNetworkType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_unknown:    return ::webrtc::RTCNetworkType::kUnknown;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_ethernet:   return ::webrtc::RTCNetworkType::kEthernet;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wifi:       return ::webrtc::RTCNetworkType::kWifi;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_cellular:   return ::webrtc::RTCNetworkType::kCellular;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_vpn:        return ::webrtc::RTCNetworkType::kVpn;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_loopback:   return "loopback";
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_bluetooth:  return ::webrtc::RTCNetworkType::kBluetooth;
    case wrapper::org::webRtc::RTCNetworkType::RTCNetworkType_wimax:      return ::webrtc::RTCNetworkType::kWimax;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::RTCNetworkType::kUnknown;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCSdpSemantics UseEnum::toWrapper(::webrtc::SdpSemantics value) noexcept
{
  switch (value)
  {
    case ::webrtc::SdpSemantics::kDefault:        return wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_default;
    case ::webrtc::SdpSemantics::kPlanB:          return wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_planB;
    case ::webrtc::SdpSemantics::kUnifiedPlan:    return wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_unifiedPlan;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_default;
}

//-----------------------------------------------------------------------------
::webrtc::SdpSemantics UseEnum::toNative(wrapper::org::webRtc::RTCSdpSemantics value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_default:      return ::webrtc::SdpSemantics::kDefault;
    case wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_planB:        return ::webrtc::SdpSemantics::kPlanB;
    case wrapper::org::webRtc::RTCSdpSemantics::RTCSdpSemantics_unifiedPlan:  return ::webrtc::SdpSemantics::kUnifiedPlan;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::SdpSemantics::kDefault;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCTlsCertPolicy UseEnum::toWrapper(::webrtc::PeerConnectionInterface::TlsCertPolicy value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicySecure:          return wrapper::org::webRtc::RTCTlsCertPolicy::RTCTlsCertPolicy_tlsCertPolicySecure;
    case ::webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck: return wrapper::org::webRtc::RTCTlsCertPolicy::RTCTlsCertPolicy_tlsCertPolicyInsecureNoCheck;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCTlsCertPolicy::RTCTlsCertPolicy_tlsCertPolicySecure;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::TlsCertPolicy UseEnum::toNative(wrapper::org::webRtc::RTCTlsCertPolicy value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCTlsCertPolicy::RTCTlsCertPolicy_tlsCertPolicySecure:          return ::webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicySecure;
    case wrapper::org::webRtc::RTCTlsCertPolicy::RTCTlsCertPolicy_tlsCertPolicyInsecureNoCheck: return ::webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicySecure;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCSdpType UseEnum::toWrapper(::webrtc::SdpType value) noexcept
{
  switch (value)
  {
    case ::webrtc::SdpType::kOffer:        return wrapper::org::webRtc::RTCSdpType::RTCSdpType_offer;
    case ::webrtc::SdpType::kPrAnswer:     return wrapper::org::webRtc::RTCSdpType::RTCSdpType_pranswer;
    case ::webrtc::SdpType::kAnswer:       return wrapper::org::webRtc::RTCSdpType::RTCSdpType_answer;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCSdpType::RTCSdpType_rollback;
}

//-----------------------------------------------------------------------------
::webrtc::SdpType UseEnum::toNative(wrapper::org::webRtc::RTCSdpType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCSdpType::RTCSdpType_offer:        return ::webrtc::SdpType::kOffer;
    case wrapper::org::webRtc::RTCSdpType::RTCSdpType_pranswer:     return ::webrtc::SdpType::kPrAnswer;
    case wrapper::org::webRtc::RTCSdpType::RTCSdpType_answer:       return ::webrtc::SdpType::kAnswer;
#pragma ZS_BUILD_NOTE("LATER","webrtc does not support rollback sdp type")
    case wrapper::org::webRtc::RTCSdpType::RTCSdpType_rollback:     break;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::SdpType::kPrAnswer;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourceState UseEnum::toWrapper(::webrtc::MediaSourceInterface::SourceState value) noexcept
{
  switch (value)
  {
    case ::webrtc::MediaSourceInterface::SourceState::kInitializing:    return wrapper::org::webRtc::MediaSourceState::MediaSourceState_initializing;
    case ::webrtc::MediaSourceInterface::SourceState::kLive:            return wrapper::org::webRtc::MediaSourceState::MediaSourceState_live;
    case ::webrtc::MediaSourceInterface::SourceState::kEnded:           return wrapper::org::webRtc::MediaSourceState::MediaSourceState_ended;
    case ::webrtc::MediaSourceInterface::SourceState::kMuted:           return wrapper::org::webRtc::MediaSourceState::MediaSourceState_muted;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::MediaSourceState::MediaSourceState_ended;
}

//-----------------------------------------------------------------------------
::webrtc::MediaSourceInterface::SourceState UseEnum::toNative(wrapper::org::webRtc::MediaSourceState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::MediaSourceState::MediaSourceState_initializing:   return ::webrtc::MediaSourceInterface::SourceState::kInitializing;
    case wrapper::org::webRtc::MediaSourceState::MediaSourceState_live:           return ::webrtc::MediaSourceInterface::SourceState::kLive;
    case wrapper::org::webRtc::MediaSourceState::MediaSourceState_ended:          return ::webrtc::MediaSourceInterface::SourceState::kEnded;
    case wrapper::org::webRtc::MediaSourceState::MediaSourceState_muted:          return ::webrtc::MediaSourceInterface::SourceState::kMuted;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::MediaSourceInterface::SourceState::kEnded;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::MediaStreamTrackState UseEnum::toWrapper(::webrtc::MediaStreamTrackInterface::TrackState value) noexcept
{
  switch (value)
  {
    case ::webrtc::MediaStreamTrackInterface::TrackState::kLive:    return wrapper::org::webRtc::MediaStreamTrackState::MediaStreamTrackState_live;
    case ::webrtc::MediaStreamTrackInterface::TrackState::kEnded:   return wrapper::org::webRtc::MediaStreamTrackState::MediaStreamTrackState_ended;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::MediaStreamTrackState::MediaStreamTrackState_ended;
}

//-----------------------------------------------------------------------------
::webrtc::MediaStreamTrackInterface::TrackState UseEnum::toNative(wrapper::org::webRtc::MediaStreamTrackState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::MediaStreamTrackState::MediaStreamTrackState_live:     return ::webrtc::MediaStreamTrackInterface::TrackState::kLive;
    case wrapper::org::webRtc::MediaStreamTrackState::MediaStreamTrackState_ended:    return ::webrtc::MediaStreamTrackInterface::TrackState::kEnded;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::MediaStreamTrackInterface::TrackState::kEnded;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCConfigurationType UseEnum::toWrapper(::webrtc::PeerConnectionInterface::RTCConfigurationType value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::RTCConfigurationType::kSafe:        return wrapper::org::webRtc::RTCConfigurationType::RTCConfigurationType_safe;
    case ::webrtc::PeerConnectionInterface::RTCConfigurationType::kAggressive:  return wrapper::org::webRtc::RTCConfigurationType::RTCConfigurationType_aggressive;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCConfigurationType::RTCConfigurationType_safe;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::RTCConfigurationType UseEnum::toNative(wrapper::org::webRtc::RTCConfigurationType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCConfigurationType::RTCConfigurationType_safe:         return ::webrtc::PeerConnectionInterface::RTCConfigurationType::kSafe;
    case wrapper::org::webRtc::RTCConfigurationType::RTCConfigurationType_aggressive:   return ::webrtc::PeerConnectionInterface::RTCConfigurationType::kAggressive;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::RTCConfigurationType::kSafe;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelState UseEnum::toWrapper(::webrtc::DataChannelInterface::DataState value) noexcept
{
  switch (value)
  {
    case ::webrtc::DataChannelInterface::DataState::kConnecting:    return wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_connecting;
    case ::webrtc::DataChannelInterface::DataState::kOpen:          return wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_open;
    case ::webrtc::DataChannelInterface::DataState::kClosing:       return wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closing;
    case ::webrtc::DataChannelInterface::DataState::kClosed:        return wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closed;
}

//-----------------------------------------------------------------------------
::webrtc::DataChannelInterface::DataState UseEnum::toNative(wrapper::org::webRtc::RTCDataChannelState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_connecting: return ::webrtc::DataChannelInterface::DataState::kConnecting;
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_open:       return ::webrtc::DataChannelInterface::DataState::kOpen;
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closing:    return ::webrtc::DataChannelInterface::DataState::kClosing;
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closed:     return ::webrtc::DataChannelInterface::DataState::kClosed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::DataChannelInterface::DataState::kClosed;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCDegradationPreference UseEnum::toWrapper(::webrtc::DegradationPreference value) noexcept
{
  switch (value)
  {
    case ::webrtc::DegradationPreference::MAINTAIN_FRAMERATE:   return wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_maintainFramerate;
    case ::webrtc::DegradationPreference::MAINTAIN_RESOLUTION:  return wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_maintainResolution;
    case ::webrtc::DegradationPreference::BALANCED:             return wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_balanced;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_balanced;
}

//-----------------------------------------------------------------------------
::webrtc::DegradationPreference UseEnum::toNative(wrapper::org::webRtc::RTCDegradationPreference value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_maintainFramerate:    return ::webrtc::DegradationPreference::MAINTAIN_FRAMERATE;
    case wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_maintainResolution:   return ::webrtc::DegradationPreference::MAINTAIN_RESOLUTION;
    case wrapper::org::webRtc::RTCDegradationPreference::RTCDegradationPreference_balanced:             return ::webrtc::DegradationPreference::BALANCED;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::DegradationPreference::BALANCED;
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(::webrtc::FecMechanism value) noexcept
{
  switch (value)
  {
    case ::webrtc::FecMechanism::RED:             return "red";
    case ::webrtc::FecMechanism::RED_AND_ULPFEC:  return "red+ulpfec";
    case ::webrtc::FecMechanism::FLEXFEC:         return "flexfec";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
::webrtc::FecMechanism UseEnum::toNativeFecMechanism(const char *value) noexcept(false)
{
  static const ::webrtc::FecMechanism enums[] = {
    ::webrtc::FecMechanism::RED,
    ::webrtc::FecMechanism::RED_AND_ULPFEC,
    ::webrtc::FecMechanism::FLEXFEC
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("FecMechanism is not understood: " + str);
}


//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtxStatus UseEnum::toWrapper(::webrtc::DtxStatus value) noexcept
{
  switch (value)
  {
    case ::webrtc::DtxStatus::DISABLED:   return wrapper::org::webRtc::RTCDtxStatus::RTCDtxStatus_disabled;
    case ::webrtc::DtxStatus::ENABLED:    return wrapper::org::webRtc::RTCDtxStatus::RTCDtxStatus_enabled;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCDtxStatus::RTCDtxStatus_enabled;
}

//-----------------------------------------------------------------------------
::webrtc::DtxStatus UseEnum::toNative(wrapper::org::webRtc::RTCDtxStatus value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCDtxStatus::RTCDtxStatus_disabled:   return ::webrtc::DtxStatus::DISABLED;
    case wrapper::org::webRtc::RTCDtxStatus::RTCDtxStatus_enabled:    return ::webrtc::DtxStatus::ENABLED;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::DtxStatus::ENABLED;
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(::webrtc::RtcpFeedbackType value) noexcept
{
  switch (value)
  {
    case ::webrtc::RtcpFeedbackType::CCM:           return "ccm";
    case ::webrtc::RtcpFeedbackType::NACK:          return "nack";
    case ::webrtc::RtcpFeedbackType::REMB:          return "goog-remb";
    case ::webrtc::RtcpFeedbackType::TRANSPORT_CC:  return "transport-cc";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
::webrtc::RtcpFeedbackType UseEnum::toNativeRtcpFeedbackType(const char *value) noexcept(false)
{
  static const ::webrtc::RtcpFeedbackType enums[] = {
    ::webrtc::RtcpFeedbackType::CCM,
    ::webrtc::RtcpFeedbackType::NACK,
    ::webrtc::RtcpFeedbackType::REMB,
    ::webrtc::RtcpFeedbackType::TRANSPORT_CC
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("RtcpFeedbackType is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(::webrtc::RtcpFeedbackMessageType value) noexcept
{
  switch (value)
  {
    case ::webrtc::RtcpFeedbackMessageType::GENERIC_NACK: return "nack";
    case ::webrtc::RtcpFeedbackMessageType::PLI:          return "pli";
    case ::webrtc::RtcpFeedbackMessageType::FIR:          return "fir";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
::webrtc::RtcpFeedbackMessageType UseEnum::toNativeRtcpFeedbackMessageType(const char *value) noexcept(false)
{
  static const ::webrtc::RtcpFeedbackMessageType enums[] = {
    ::webrtc::RtcpFeedbackMessageType::GENERIC_NACK,
    ::webrtc::RtcpFeedbackMessageType::PLI,
    ::webrtc::RtcpFeedbackMessageType::FIR
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("RtcpFeedbackMessageType is not understood: " + str);
}


//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCECCurve UseEnum::toWrapper(::rtc::ECCurve value) noexcept
{
  switch (value)
  {
    case ::rtc::ECCurve::EC_NIST_P256:   return wrapper::org::webRtc::RTCECCurve::RTCECCurve_nistP256;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCECCurve::RTCECCurve_nistP256;
}

//-----------------------------------------------------------------------------
::rtc::ECCurve UseEnum::toNative(wrapper::org::webRtc::RTCECCurve value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCECCurve::RTCECCurve_nistP256:   return ::rtc::ECCurve::EC_NIST_P256;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::rtc::ECCurve::EC_NIST_P256;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyType UseEnum::toWrapper(::rtc::KeyType value) noexcept
{
  switch (value)
  {
    case ::rtc::KeyType::KT_RSA:    return wrapper::org::webRtc::RTCKeyType::RTCKeyType_rsa;
    case ::rtc::KeyType::KT_ECDSA:  return wrapper::org::webRtc::RTCKeyType::RTCKeyType_ecdsa;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCKeyType::RTCKeyType_default;
}

//-----------------------------------------------------------------------------
::rtc::KeyType UseEnum::toNative(wrapper::org::webRtc::RTCKeyType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCKeyType::RTCKeyType_default:  return ::rtc::KeyType::KT_DEFAULT;
    case wrapper::org::webRtc::RTCKeyType::RTCKeyType_rsa:      return ::rtc::KeyType::KT_RSA;
    case wrapper::org::webRtc::RTCKeyType::RTCKeyType_ecdsa:    return ::rtc::KeyType::KT_RSA;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::rtc::KeyType::KT_DEFAULT;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceCandidateType UseEnum::toWrapperRTCIceCandidateType(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCIceCandidateType enums[] = {
    wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_host,
    wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_srflex,
    wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_prflx,
    wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_relay,
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("RTCIceCandidateType is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCIceCandidateType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_host:     return ::webrtc::RTCIceCandidateType::kHost;
    case wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_srflex:   return ::webrtc::RTCIceCandidateType::kSrflx;
    case wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_prflx:    return ::webrtc::RTCIceCandidateType::kPrflx;
    case wrapper::org::webRtc::RTCIceCandidateType::RTCIceCandidateType_relay:    return ::webrtc::RTCIceCandidateType::kRelay;
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceComponent UseEnum::toWrapperRTCIceComponent(int value) noexcept(false)
{
  switch (value)
  {
    case 1: return wrapper::org::webRtc::RTCIceComponent::RTCIceComponent_rtp;
    case 2: return wrapper::org::webRtc::RTCIceComponent::RTCIceComponent_rtcp;
  }
  ZS_THROW_INVALID_ARGUMENT("RTCIceComponent is not understood: " + zsLib::string(value));
}

//-----------------------------------------------------------------------------
int UseEnum::toNative(wrapper::org::webRtc::RTCIceComponent value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceComponent::RTCIceComponent_rtp:     return 1;
    case wrapper::org::webRtc::RTCIceComponent::RTCIceComponent_rtcp:   return 2;
  }
  ZS_ASSERT_FAIL("unknown type");
  return 0;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceConnectionState UseEnum::toWrapper(::webrtc::PeerConnectionInterface::IceConnectionState value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew:            return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_new;
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking:       return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_checking;
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected:      return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_connected;
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted:      return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_completed;
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed:         return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_failed;
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected:   return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_disconnected;
    case ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed:         return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_closed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_closed;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::IceConnectionState UseEnum::toNative(wrapper::org::webRtc::RTCIceConnectionState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_new:              return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew;
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_checking:         return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking;
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_connected:        return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected;
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_completed:        return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted;
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_failed:           return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed;
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_disconnected:     return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected;
    case wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_closed:           return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceGatheringState UseEnum::toWrapper(::webrtc::PeerConnectionInterface::IceGatheringState value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringNew:          return wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_new;
    case ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringGathering:    return wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_gathering;
    case ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete:     return wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_complete;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_complete;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::IceGatheringState UseEnum::toNative(wrapper::org::webRtc::RTCIceGatheringState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_new:        return ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringNew;
    case wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_gathering:  return ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringGathering;
    case wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_complete:   return ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCSignalingState UseEnum::toWrapper(::webrtc::PeerConnectionInterface::SignalingState value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::SignalingState::kStable:              return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_stable;
    case ::webrtc::PeerConnectionInterface::SignalingState::kHaveLocalOffer:      return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveLocalOffer;
    case ::webrtc::PeerConnectionInterface::SignalingState::kHaveLocalPrAnswer:   return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveLocalPranswer;
    case ::webrtc::PeerConnectionInterface::SignalingState::kHaveRemoteOffer:     return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveRemoteOffer;
    case ::webrtc::PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer:  return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveRemotePranswer;
    case ::webrtc::PeerConnectionInterface::SignalingState::kClosed:              return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_closed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_closed;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::SignalingState UseEnum::toNative(wrapper::org::webRtc::RTCSignalingState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_stable:               return ::webrtc::PeerConnectionInterface::SignalingState::kStable;
    case wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveLocalOffer:       return ::webrtc::PeerConnectionInterface::SignalingState::kHaveLocalOffer;
    case wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveLocalPranswer:    return ::webrtc::PeerConnectionInterface::SignalingState::kHaveLocalPrAnswer;
    case wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveRemoteOffer:      return ::webrtc::PeerConnectionInterface::SignalingState::kHaveRemoteOffer;
    case wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_haveRemotePranswer:   return ::webrtc::PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer;
    case wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_closed:               return ::webrtc::PeerConnectionInterface::SignalingState::kClosed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::SignalingState::kClosed;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceProtocol UseEnum::toWrapperRTCIceProtocol(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCIceProtocol enums[] = {
    wrapper::org::webRtc::RTCIceProtocol::RTCIceProtocol_udp,
    wrapper::org::webRtc::RTCIceProtocol::RTCIceProtocol_tcp
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("RTCIceProtocol is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCIceProtocol value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceProtocol::RTCIceProtocol_udp:               return "udp";
    case wrapper::org::webRtc::RTCIceProtocol::RTCIceProtocol_tcp:               return "tcp";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceTcpCandidateType UseEnum::toWrapperRTCIceTcpCandidateType(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCIceTcpCandidateType enums[] = {
    wrapper::org::webRtc::RTCIceTcpCandidateType::RTCIceTcpCandidateType_active,
    wrapper::org::webRtc::RTCIceTcpCandidateType::RTCIceTcpCandidateType_passive,
    wrapper::org::webRtc::RTCIceTcpCandidateType::RTCIceTcpCandidateType_so
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(enums) / sizeof(enums[0]); ++loop) {
    if (0 == str.compareNoCase(toString(enums[loop]))) return enums[loop];
  }

  ZS_THROW_INVALID_ARGUMENT("RTCIceTcpCandidateType is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCIceTcpCandidateType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCIceTcpCandidateType::RTCIceTcpCandidateType_active:     return "active";
    case wrapper::org::webRtc::RTCIceTcpCandidateType::RTCIceTcpCandidateType_passive:    return "passive";
    case wrapper::org::webRtc::RTCIceTcpCandidateType::RTCIceTcpCandidateType_so:         return "so";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCPriorityType UseEnum::toWrapperRTCPriorityType(double value) noexcept
{
  static const wrapper::org::webRtc::RTCPriorityType values[] =
  {
    wrapper::org::webRtc::RTCPriorityType_veryLow,
    wrapper::org::webRtc::RTCPriorityType_low,
    wrapper::org::webRtc::RTCPriorityType_medium,
    wrapper::org::webRtc::RTCPriorityType_high
   };

  for (size_t loop = 0; loop < sizeof(values) / sizeof(values[0]); ++loop) {
    if (value <= toNative(values[loop])) return values[loop];
  }
  return wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_high;
}

//-----------------------------------------------------------------------------
double UseEnum::toNative(wrapper::org::webRtc::RTCPriorityType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_veryLow:    return 0.25;
    case wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_low:        return 0.5;
    case wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_medium:     return 1.0;
    case wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_high:       return 2.0;
  }
  ZS_ASSERT_FAIL("unknown type");
  return 1.0;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverDirection UseEnum::toWrapper(::webrtc::RtpTransceiverDirection value) noexcept
{
  switch (value)
  {
    case ::webrtc::RtpTransceiverDirection::kSendRecv:   return wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_sendrecv;
    case ::webrtc::RtpTransceiverDirection::kSendOnly:   return wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_sendonly;
    case ::webrtc::RtpTransceiverDirection::kRecvOnly:   return wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_recvonly;
    case ::webrtc::RtpTransceiverDirection::kInactive:   return wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_inactive;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_inactive;
}

//-----------------------------------------------------------------------------
::webrtc::RtpTransceiverDirection UseEnum::toNative(wrapper::org::webRtc::RTCRtpTransceiverDirection value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_sendrecv:   return ::webrtc::RtpTransceiverDirection::kSendRecv;
    case wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_sendonly:   return ::webrtc::RtpTransceiverDirection::kSendOnly;
    case wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_recvonly:   return ::webrtc::RtpTransceiverDirection::kRecvOnly;
    case wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_inactive:   return ::webrtc::RtpTransceiverDirection::kInactive;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::RtpTransceiverDirection::kInactive;
}


//-----------------------------------------------------------------------------
wrapper::org::webRtc::VideoCaptureState UseEnum::toWrapper(::cricket::CaptureState value) noexcept
{
  switch (value)
  {
    case ::cricket::CaptureState::CS_STOPPED:   return wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_stopped;
    case ::cricket::CaptureState::CS_STARTING:  return wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_starting;
    case ::cricket::CaptureState::CS_RUNNING:   return wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_running;
    case ::cricket::CaptureState::CS_FAILED:    return wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_failed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_stopped;
}

//-----------------------------------------------------------------------------
::cricket::CaptureState UseEnum::toNative(wrapper::org::webRtc::VideoCaptureState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_stopped:    return ::cricket::CaptureState::CS_STOPPED;
    case wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_starting:   return ::cricket::CaptureState::CS_STARTING;
    case wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_running:    return ::cricket::CaptureState::CS_RUNNING;
    case wrapper::org::webRtc::VideoCaptureState::VideoCaptureState_failed:     return ::cricket::CaptureState::CS_FAILED;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::cricket::CaptureState::CS_STOPPED;
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCStatsOutputLevel UseEnum::toWrapper(::webrtc::PeerConnectionInterface::StatsOutputLevel value) noexcept
{
  switch (value)
  {
    case ::webrtc::PeerConnectionInterface::kStatsOutputLevelStandard:   return wrapper::org::webRtc::RTCStatsOutputLevel::RTCStatsOutputLevel_standard;
    case ::webrtc::PeerConnectionInterface::kStatsOutputLevelDebug:  return wrapper::org::webRtc::RTCStatsOutputLevel::RTCStatsOutputLevel_debug;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webRtc::RTCStatsOutputLevel::RTCStatsOutputLevel_standard;
}

//-----------------------------------------------------------------------------
::webrtc::PeerConnectionInterface::StatsOutputLevel UseEnum::toNative(wrapper::org::webRtc::RTCStatsOutputLevel value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCStatsOutputLevel::RTCStatsOutputLevel_standard: return ::webrtc::PeerConnectionInterface::kStatsOutputLevelStandard;
    case wrapper::org::webRtc::RTCStatsOutputLevel::RTCStatsOutputLevel_debug:    return ::webrtc::PeerConnectionInterface::kStatsOutputLevelDebug;
  }
  ZS_ASSERT_FAIL("unknown type");
  return ::webrtc::PeerConnectionInterface::kStatsOutputLevelStandard;
}

//-----------------------------------------------------------------------------
zsLib::Optional<::webrtc::StatsReport::Direction> UseEnum::toWrapperDirection(const char *statId) noexcept
{
  zsLib::Optional<::webrtc::StatsReport::Direction> result {};

  String str(statId);
  if (str.isEmpty()) return result;

  const char *pos = strrchr(str.c_str(), kSeparator);
  if (!pos) return result;

  String remainder(pos + 1);

  static const ::webrtc::StatsReport::Direction values[] =
  {
    ::webrtc::StatsReport::Direction::kSend,
    ::webrtc::StatsReport::Direction::kReceive
  };

  for (size_t loop = 0; loop < sizeof(values) / sizeof(values[0]); ++loop) {
    if (0 == remainder.compareNoCase(toString(values[loop]))) return values[loop];
  }
  return result;
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(::webrtc::StatsReport::Direction value) noexcept
{
  switch (value)
  {
    case ::webrtc::StatsReport::Direction::kSend:     return "send";
    case ::webrtc::StatsReport::Direction::kReceive:  return "receive";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCStatsType UseEnum::toWrapperRTCStatsType(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCStatsType values[] =
  {
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_codec,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_inboundRtp,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_outboundRtp,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_remoteInboundRtp,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_remoteOutboundRtp,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_csrc,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_peerConnection,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_dataChannel,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_stream,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_track,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_sender,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_receiver,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_transport,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_candidatePair,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_localCandidate,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_remoteCandidate,
    wrapper::org::webRtc::RTCStatsType::RTCStatsType_certificate
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(values) / sizeof(values[0]); ++loop) {
    if (0 == str.compareNoCase(toString(values[loop]))) return values[loop];
  }
  ZS_THROW_INVALID_ARGUMENT("RTCStatsType is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCStatsType value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_codec:                return ::webrtc::RTCCodecStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_inboundRtp:           return ::webrtc::RTCInboundRTPStreamStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_outboundRtp:          return ::webrtc::RTCOutboundRTPStreamStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_remoteInboundRtp:     return "remote-inbound-rtp";
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_remoteOutboundRtp:    return "remote-outbound-rtp";
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_csrc:                 return "csrc";
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_peerConnection:       return ::webrtc::RTCPeerConnectionStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_dataChannel:          return ::webrtc::RTCDataChannelStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_stream:               return ::webrtc::RTCMediaStreamStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_track:                return ::webrtc::RTCMediaStreamTrackStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_sender:               return "sender";
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_receiver:             return "receiver";
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_transport:            return ::webrtc::RTCTransportStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_candidatePair:        return ::webrtc::RTCIceCandidatePairStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_localCandidate:       return ::webrtc::RTCLocalIceCandidateStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_remoteCandidate:      return ::webrtc::RTCRemoteIceCandidateStats::kType;
    case wrapper::org::webRtc::RTCStatsType::RTCStatsType_certificate:          return ::webrtc::RTCCertificateStats::kType;
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtlsTransportState UseEnum::toWrapperRTCDtlsTransportState(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCDtlsTransportState values[] =
  {
    wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_new,
    wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_connecting,
    wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_connected,
    wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_closed,
    wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_failed,
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(values) / sizeof(values[0]); ++loop) {
    if (0 == str.compareNoCase(toString(values[loop]))) return values[loop];
  }
  ZS_THROW_INVALID_ARGUMENT("RTCDtlsTransportState is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCDtlsTransportState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_new:          return ::webrtc::RTCDtlsTransportState::kNew;
    case wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_connecting:   return ::webrtc::RTCDtlsTransportState::kConnecting;
    case wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_connected:    return ::webrtc::RTCDtlsTransportState::kConnected;
    case wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_closed:       return ::webrtc::RTCDtlsTransportState::kClosed;
    case wrapper::org::webRtc::RTCDtlsTransportState::RTCDtlsTransportState_failed:       return ::webrtc::RTCDtlsTransportState::kFailed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCStatsIceCandidatePairState UseEnum::toWrapperRTCStatsIceCandidatePairState(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCStatsIceCandidatePairState values[] =
  {
    wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_frozen,
    wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_waiting,
    wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_inProgress,
    wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_failed,
    wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_succeeded,
    wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_cancelled,
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(values) / sizeof(values[0]); ++loop) {
    if (0 == str.compareNoCase(toString(values[loop]))) return values[loop];
  }
  ZS_THROW_INVALID_ARGUMENT("RTCDtlsTransportState is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCStatsIceCandidatePairState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_frozen:     return ::webrtc::RTCStatsIceCandidatePairState::kFrozen;
    case wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_waiting:    return ::webrtc::RTCStatsIceCandidatePairState::kWaiting;
    case wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_inProgress: return ::webrtc::RTCStatsIceCandidatePairState::kInProgress;
    case wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_failed:     return ::webrtc::RTCStatsIceCandidatePairState::kFailed;
    case wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_succeeded:  return ::webrtc::RTCStatsIceCandidatePairState::kSucceeded;
    case wrapper::org::webRtc::RTCStatsIceCandidatePairState::RTCStatsIceCandidatePairState_cancelled:  return "cancelled";
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

//-----------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelState UseEnum::toWrapperRTCDataChannelState(const char *value) noexcept(false)
{
  static const wrapper::org::webRtc::RTCDataChannelState values[] =
  {
    wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_connecting,
    wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_open,
    wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closing,
    wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closed,
  };

  String str(value);

  for (size_t loop = 0; loop < sizeof(values) / sizeof(values[0]); ++loop) {
    if (0 == str.compareNoCase(toString(values[loop]))) return values[loop];
  }
  ZS_THROW_INVALID_ARGUMENT("RTCDtlsTransportState is not understood: " + str);
}

//-----------------------------------------------------------------------------
const char *UseEnum::toString(wrapper::org::webRtc::RTCDataChannelState value) noexcept
{
  switch (value)
  {
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_connecting: return ::webrtc::RTCDataChannelState::kConnecting;
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_open:       return ::webrtc::RTCDataChannelState::kOpen;
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closing:    return ::webrtc::RTCDataChannelState::kClosing;
    case wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closed:     return ::webrtc::RTCDataChannelState::kClosed;
  }
  ZS_ASSERT_FAIL("unknown type");
  return "unknown";
}

