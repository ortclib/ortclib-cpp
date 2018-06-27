
#include "impl_org_webrtc_RTCConfiguration.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration::NativeType, NativeType);

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
wrapper::impl::org::webrtc::RTCConfiguration::~RTCConfiguration()
{
  ZS_ASSERT(native_);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCConfiguration::wrapper_init_org_webrtc_RTCConfiguration() noexcept
{
  native_ = make_shared<NativeType>();
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
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(native);
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
  return converted->native_;
}
