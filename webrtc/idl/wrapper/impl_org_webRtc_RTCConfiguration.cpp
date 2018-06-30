
#include "impl_org_webrtc_RTCConfiguration.h"
#include "impl_org_webrtc_RTCIceServer.h"
#include "impl_org_webrtc_RTCCertificate.h"
#include "impl_org_webrtc_MillisecondIntervalRange.h"

#include "impl_org_webrtc_enums.h"

#include <zsLib/SafeInt.h>

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
using zsLib::Milliseconds;

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCIceServer, UseIceServer);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCCertificate, UseCertificate);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MillisecondIntervalRange, UseMillisecondIntervalRange);


static void apply(const WrapperImplType &from, NativeType &to) noexcept;
static void apply(const NativeType &from, WrapperImplType &to) noexcept;

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCConfiguration::RTCConfiguration() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCConfigurationPtr wrapper::org::webrtc::RTCConfiguration::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCConfiguration>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCConfiguration::~RTCConfiguration() noexcept
{
  ZS_ASSERT(native_);
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::wrapper_init_org_webrtc_RTCConfiguration() noexcept
{
  native_ = make_shared<NativeType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::wrapper_init_org_webrtc_RTCConfiguration(wrapper::org::webrtc::RTCConfigurationType type) noexcept
{
  native_ = make_shared<NativeType>(UseEnum::toNative(type));
  apply(*native_, *this);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::wrapper_init_org_webrtc_RTCConfiguration(wrapper::org::webrtc::RTCConfigurationPtr source) noexcept
{
  auto native = toNative(source);
  if (!native) {
    wrapper_init_org_webrtc_RTCConfiguration();
    return;
  }
  native_ = make_shared<NativeType>(*native);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCConfiguration::get_dscp() noexcept
{
  if (!native_) return false;
  return native_->dscp();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::set_dscp(bool value) noexcept
{
  if (!native_) return;
  native_->set_dscp(value);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCConfiguration::get_cpuAdaptation() noexcept
{
  if (!native_) return false;
  return native_->cpu_adaptation();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::set_cpuAdaptation(bool value) noexcept
{
  if (!native_) return;
  native_->set_cpu_adaptation(value);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCConfiguration::get_suspendBelowMinBitrate() noexcept
{
  if (!native_) return false;
  return native_->suspend_below_min_bitrate();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::set_suspendBelowMinBitrate(bool value) noexcept
{
  if (!native_) return;
  native_->set_suspend_below_min_bitrate(value);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCConfiguration::get_prerendererSmoothing() noexcept
{
  if (!native_) return false;
  return native_->prerenderer_smoothing();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::set_prerendererSmoothing(bool value) noexcept
{
  if (!native_) return;
  native_->set_prerenderer_smoothing(value);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCConfiguration::get_experimentCpuLoadEstimator() noexcept
{
  if (!native_) return false;
  return native_->experiment_cpu_load_estimator();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::set_experimentCpuLoadEstimator(bool value) noexcept
{
  if (!native_) return;
  native_->set_experiment_cpu_load_estimator(value);
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to) noexcept
{
  to.servers.clear();
  if (from.iceServers) {
    for (auto iter = from.iceServers->begin(); iter != from.iceServers->end(); ++iter) {
      auto converted = UseIceServer::toNative(*iter);
      if (!converted) continue;
      to.servers.push_back(*converted);
    }
  }

  to.type = UseEnum::toNative(from.iceTransportPolicy);
  to.bundle_policy = UseEnum::toNative(from.bundlePolicy);
  to.rtcp_mux_policy = UseEnum::toNative(from.rtcpMuxPolicy);

  to.certificates.clear();
  if (from.certificates) {
    for (auto iter = from.certificates->begin(); iter != from.certificates->end(); ++iter) {
      auto converted = UseCertificate::toNative(*iter);
      if (!converted) continue;
      to.certificates.push_back(converted);
    }
  }

#pragma ZS_BUILD_NOTE("LATER", "peer identity is not supported in webrtc")
  // to. = from.peerIdentity;

  to.ice_candidate_pool_size = SafeInt<decltype(to.ice_candidate_pool_size)>(from.iceCandidatePoolSize);
  to.disable_ipv6 = from.disableIpv6;
  to.disable_ipv6_on_wifi = from.disableIpv6OnWifi;

  if (from.maxIpv6Networks.has_value()) {
    to.max_ipv6_networks = SafeInt<decltype(to.max_ipv6_networks)>(from.maxIpv6Networks.value());
  } else {
    to.max_ipv6_networks = ::cricket::kDefaultMaxIPv6Networks;
  }

  to.disable_link_local_networks = from.disableLinkLocalNetworks;
  to.enable_rtp_data_channel = from.enableRtpDataChannel;

  if (from.screencastMinBitrate.has_value()) {
    to.screencast_min_bitrate = (int)(SafeInt<int>(from.screencastMinBitrate.value()));
  } else {
    to.screencast_min_bitrate.reset();
  }
  if (from.combinedAudioVideoBwe.has_value()) {
    to.combined_audio_video_bwe = from.combinedAudioVideoBwe.value();
  } else {
    to.combined_audio_video_bwe.reset();
  }

  if (from.enableDtlsSrtp.has_value()) {
    to.enable_dtls_srtp = from.enableDtlsSrtp.value();
  }
  else {
    to.enable_dtls_srtp.reset();
  }
  to.tcp_candidate_policy = UseEnum::toNative(from.tcpCandidatePolicy);
  to.candidate_network_policy = UseEnum::toNative(from.candidateNetworkPolicy);
  to.audio_jitter_buffer_max_packets = SafeInt<decltype(to.audio_jitter_buffer_max_packets)>(from.audioJitterBufferMaxPackets);
  to.audio_jitter_buffer_fast_accelerate = from.audioJitterBufferFastAccelerate;

  to.ice_connection_receiving_timeout = (Milliseconds() == from.iceConnectionReceivingTimeout ? NativeType::kUndefined : SafeInt<decltype(to.ice_connection_receiving_timeout)>(from.iceConnectionReceivingTimeout.count()));
  to.ice_backup_candidate_pair_ping_interval = (Milliseconds() == from.iceBackupCandidatePairPingInterval ? NativeType::kUndefined : SafeInt<decltype(to.ice_connection_receiving_timeout)>(from.iceBackupCandidatePairPingInterval.count()));

  to.continual_gathering_policy = UseEnum::toNative(from.continualGatheringPolicy);
  to.prioritize_most_likely_ice_candidate_pairs = from.prioritizeMostLikelyIceCandidatePairs;
  to.prune_turn_ports = from.pruneTurnPorts;
  to.enable_ice_renomination = from.enableIceRenomination;
  to.redetermine_role_on_ice_restart = from.redetermineRoleOnIceRestart;

  if (Milliseconds() == from.iceCheckMinInterval) {
    to.ice_check_min_interval = (int)SafeInt<int>(from.iceCheckMinInterval.count());
  } else {
    to.ice_check_min_interval.reset();
  }
  if (Milliseconds() == from.stunCandidateKeepaliveInterval) {
    to.stun_candidate_keepalive_interval = (int)SafeInt<int>(from.stunCandidateKeepaliveInterval.count());
  }
  else {
    to.stun_candidate_keepalive_interval.reset();
  }

  if (from.iceRegatherIntervalRange.has_value()) {
    auto converted = UseMillisecondIntervalRange::toNative(from.iceRegatherIntervalRange.value());
    if (converted) {
      to.ice_regather_interval_range = *converted;
    } else {
      to.ice_regather_interval_range.reset();
    }
  } else {
    to.ice_regather_interval_range.reset();
  }

  if (from.networkPreference.has_value()) {
    to.network_preference = UseEnum::toNative(from.networkPreference.value());
  } else {
    to.network_preference.reset();
  }

  to.sdp_semantics = UseEnum::toNative(from.sdpSemantics);
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to) noexcept
{
  to.iceServers = make_shared< list< wrapper::org::webrtc::RTCIceServerPtr > >();
  for (auto iter = from.servers.begin(); iter != from.servers.end(); ++iter) {
    auto converted = UseIceServer::toWrapper(*iter);
    if (!converted) continue;
    to.iceServers->push_back(converted);
  }

  to.iceTransportPolicy = UseEnum::toWrapper(from.type);
  to.bundlePolicy = UseEnum::toWrapper(from.bundle_policy);
  to.rtcpMuxPolicy = UseEnum::toWrapper(from.rtcp_mux_policy);

  to.certificates = make_shared< list< wrapper::org::webrtc::RTCCertificatePtr > >();
  for (auto iter = from.certificates.begin(); iter != from.certificates.end(); ++iter) {
    auto converted = UseCertificate::toWrapper((*iter).get());
    if (!converted) continue;
    to.certificates->push_back(converted);
  }

#pragma ZS_BUILD_NOTE("LATER", "peer identity is not supported in webrtc")
  // to.peerIdentity = from.

  to.iceCandidatePoolSize = SafeInt<decltype(to.iceCandidatePoolSize)>(from.ice_candidate_pool_size);
  to.disableIpv6 = from.disable_ipv6;
  to.disableIpv6OnWifi = from.disable_ipv6_on_wifi;

  if (::cricket::kDefaultMaxIPv6Networks != from.max_ipv6_networks) {
    to.maxIpv6Networks = SafeInt<decltype(to.maxIpv6Networks)::value_type>(from.max_ipv6_networks);
  }

  to.disableLinkLocalNetworks = from.disable_link_local_networks;
  to.enableRtpDataChannel = from.enable_rtp_data_channel;
  if (from.screencast_min_bitrate.has_value()) {
    to.screencastMinBitrate = from.screencast_min_bitrate.value();
  }
  if (from.combined_audio_video_bwe.has_value()) {
    to.combinedAudioVideoBwe = from.combined_audio_video_bwe.value();
  }
  if (from.enable_dtls_srtp.has_value()) {
    to.enableDtlsSrtp = from.enable_dtls_srtp.value();
  }

  to.tcpCandidatePolicy = UseEnum::toWrapper(from.tcp_candidate_policy);
  to.candidateNetworkPolicy = UseEnum::toWrapper(from.candidate_network_policy);
  to.audioJitterBufferMaxPackets = SafeInt<decltype(to.audioJitterBufferMaxPackets)>(from.audio_jitter_buffer_max_packets);
  to.audioJitterBufferFastAccelerate = from.audio_jitter_buffer_fast_accelerate;

  if (NativeType::kUndefined != from.ice_connection_receiving_timeout) {
    to.iceConnectionReceivingTimeout = Milliseconds(SafeInt<Milliseconds::rep>(from.ice_connection_receiving_timeout));
  }
  if (NativeType::kUndefined != from.ice_backup_candidate_pair_ping_interval) {
    to.iceBackupCandidatePairPingInterval = Milliseconds(SafeInt<Milliseconds::rep>(from.ice_backup_candidate_pair_ping_interval));
  }

  to.continualGatheringPolicy = UseEnum::toWrapper(from.continual_gathering_policy);
  to.prioritizeMostLikelyIceCandidatePairs = from.prioritize_most_likely_ice_candidate_pairs;
  to.pruneTurnPorts = from.prune_turn_ports;
  to.enableIceRenomination = from.enable_ice_renomination;
  to.redetermineRoleOnIceRestart = from.redetermine_role_on_ice_restart;

  if (from.ice_check_min_interval.has_value()) {
    to.iceCheckMinInterval = Milliseconds(SafeInt<Milliseconds::rep>(from.ice_check_min_interval.value()));
  }

  if (from.stun_candidate_keepalive_interval.has_value()) {
    to.stunCandidateKeepaliveInterval = Milliseconds(SafeInt<Milliseconds::rep>(from.stun_candidate_keepalive_interval.value()));
  }

  if (from.ice_regather_interval_range.has_value()) {
    auto temp = UseMillisecondIntervalRange::toWrapper(from.ice_regather_interval_range.value());
    if (temp) to.iceRegatherIntervalRange = temp;
  }

  if (from.network_preference.has_value()) {
    to.networkPreference = UseEnum::toWrapper(from.network_preference.value());
  }

  to.sdpSemantics = UseEnum::toWrapper(from.sdp_semantics);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(native);
  apply(native, *result);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypePtr();
  auto cloned = make_shared<NativeType>(*converted->native_);
  apply(*converted, *cloned);
  return cloned;
}
