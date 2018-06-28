
#include "impl_org_webrtc_AudioOptions.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::AudioOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::AudioOptions::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::AudioOptions::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::AudioOptions::AudioOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::AudioOptionsPtr wrapper::org::webrtc::AudioOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::AudioOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::AudioOptions::~AudioOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::AudioOptions::wrapper_init_org_webrtc_AudioOptions() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  if (from.echo_cancellation.has_value()) {
    to.echoCancellation = from.echo_cancellation.value();
  }
#if defined(WEBRTC_IOS)
  if (from.ios_force_software_aec_HACK.has_value()) {
    to.iosForceSoftwareAec_HACK = from.ios_force_software_aec_HACK.value();
  }
#endif //defined(WEBRTC_IOS)
  if (from.auto_gain_control.has_value()) {
    to.autoGainControl = from.auto_gain_control.value();
  }
  if (from.noise_suppression.has_value()) {
    to.noiseSuppression = from.noise_suppression.value();
  }
  if (from.highpass_filter.has_value()) {
    to.highpassFilter = from.highpass_filter.value();
  }
  if (from.stereo_swapping.has_value()) {
    to.stereoSwapping = from.stereo_swapping.value();
  }
  if (from.audio_jitter_buffer_max_packets.has_value()) {
    to.audioJitterBufferMaxPackets = SafeInt<decltype(to.audioJitterBufferMaxPackets)::value_type>(from.audio_jitter_buffer_max_packets.value());
  }
  if (from.audio_jitter_buffer_fast_accelerate.has_value()) {
    to.audioJitterBufferFastAccelerate = from.audio_jitter_buffer_fast_accelerate.value();
  }
  if (from.typing_detection.has_value()) {
    to.typingDetection = from.typing_detection.value();
  }
  if (from.aecm_generate_comfort_noise.has_value()) {
    to.aecmGenerateComfortNoise = from.aecm_generate_comfort_noise.value();
  }
  if (from.experimental_agc.has_value()) {
    to.experimentalAgc = from.experimental_agc.value();
  }
  if (from.extended_filter_aec.has_value()) {
    to.extendedFilterAec = from.extended_filter_aec.value();
  }
  if (from.delay_agnostic_aec.has_value()) {
    to.delayAgnosticAec = from.delay_agnostic_aec.value();
  }
  if (from.experimental_ns.has_value()) {
    to.experimentalNs = from.experimental_ns.value();
  }
  if (from.intelligibility_enhancer.has_value()) {
    to.intelligibilityEnhancer = from.intelligibility_enhancer.value();
  }
  if (from.level_control.has_value()) {
    to.levelControl = from.level_control.value();
  }
  if (from.level_control_initial_peak_level_dbfs.has_value()) {
    to.levelControlInitialPeakLevelDbfs = from.level_control_initial_peak_level_dbfs.value();
  }
  if (from.residual_echo_detector.has_value()) {
    to.residualEchoDetector = from.residual_echo_detector.value();
  }
  if (from.tx_agc_target_dbov.has_value()) {
    to.txAgcTargetDbov = SafeInt<decltype(to.txAgcTargetDbov)::value_type>(from.tx_agc_target_dbov.value());
  }
  if (from.tx_agc_digital_compression_gain.has_value()) {
    to.txAgcDigitalCompressionGain = SafeInt<decltype(to.txAgcDigitalCompressionGain)::value_type>(from.tx_agc_digital_compression_gain.value());
  }
  if (from.tx_agc_limiter.has_value()) {
    to.txAgcLimiter = from.tx_agc_limiter.value();
  }
  if (from.combined_audio_video_bwe.has_value()) {
    to.combinedAudioVideoBwe = from.combined_audio_video_bwe.value();
  }
  if (from.audio_network_adaptor.has_value()) {
    to.audioNetworkAdaptor = from.audio_network_adaptor.value();
  }
  if (from.audio_network_adaptor_config.has_value()) {
    to.audioNetworkAdaptorConfig = from.audio_network_adaptor_config.value();
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  if (from.echoCancellation.has_value()) {
    to.echo_cancellation = from.echoCancellation.value();
  }
#if defined(WEBRTC_IOS)
  if (from.iosForceSoftwareAec_HACK.has_value()) {
    to.ios_force_software_aec_HACK = from.iosForceSoftwareAec_HACK.value();
  }
#endif //defined(WEBRTC_IOS)
  if (from.autoGainControl.has_value()) {
    to.auto_gain_control = from.autoGainControl.value();
  }
  if (from.noiseSuppression.has_value()) {
    to.noise_suppression = from.noiseSuppression.value();
  }
  if (from.highpassFilter.has_value()) {
    to.highpass_filter = from.highpassFilter.value();
  }
  if (from.stereoSwapping.has_value()) {
    to.stereo_swapping = from.stereoSwapping.value();
  }
  if (from.audioJitterBufferMaxPackets.has_value()) {
    to.audio_jitter_buffer_max_packets = (int)SafeInt<int>(from.audioJitterBufferMaxPackets.value());
  }
  if (from.audioJitterBufferFastAccelerate.has_value()) {
    to.audio_jitter_buffer_fast_accelerate = from.audioJitterBufferFastAccelerate.value();
  }
  if (from.typingDetection.has_value()) {
    to.typing_detection = from.typingDetection.value();
  }
  if (from.aecmGenerateComfortNoise.has_value()) {
    to.aecm_generate_comfort_noise = from.aecmGenerateComfortNoise.value();
  }
  if (from.experimentalAgc.has_value()) {
    to.experimental_agc = from.experimentalAgc.value();
  }
  if (from.extendedFilterAec.has_value()) {
    to.extended_filter_aec = from.extendedFilterAec.value();
  }
  if (from.delayAgnosticAec.has_value()) {
    to.delay_agnostic_aec = from.delayAgnosticAec.value();
  }
  if (from.experimentalNs.has_value()) {
    to.experimental_ns = from.experimentalNs.value();
  }
  if (from.intelligibilityEnhancer.has_value()) {
    to.intelligibility_enhancer = from.intelligibilityEnhancer.value();
  }
  if (from.levelControl.has_value()) {
    to.level_control = from.levelControl.value();
  }
  if (from.levelControlInitialPeakLevelDbfs.has_value()) {
    to.level_control_initial_peak_level_dbfs = from.levelControlInitialPeakLevelDbfs.value();
  }
  if (from.residualEchoDetector.has_value()) {
    to.residual_echo_detector = from.residualEchoDetector.value();
  }
  if (from.txAgcTargetDbov.has_value()) {
    to.tx_agc_target_dbov = (uint16_t)SafeInt<uint16_t>(from.txAgcTargetDbov.value());
  }
  if (from.txAgcTargetDbov.has_value()) {
    to.tx_agc_digital_compression_gain = (uint16_t)SafeInt<uint16_t>(from.txAgcDigitalCompressionGain.value());
  }
  if (from.txAgcLimiter.has_value()) {
    to.tx_agc_limiter = from.txAgcLimiter.value();
  }
  if (from.combinedAudioVideoBwe.has_value()) {
    to.combined_audio_video_bwe = from.combinedAudioVideoBwe.value();
  }
  if (from.combinedAudioVideoBwe.has_value()) {
    to.audio_network_adaptor = from.audioNetworkAdaptor.value();
  }
  if (from.audioNetworkAdaptorConfig.hasData()) {
    to.audio_network_adaptor_config = from.audioNetworkAdaptorConfig;
  }
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
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
  auto cloned = make_shared<NativeType>();
  apply(*converted, *cloned);
  return cloned;
}
