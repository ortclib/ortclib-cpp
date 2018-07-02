
#include "impl_org_webRtc_RTCBitrateParameters.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCBitrateParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCBitrateParameters::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCBitrateParameters::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCBitrateParameters::RTCBitrateParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCBitrateParametersPtr wrapper::org::webRtc::RTCBitrateParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCBitrateParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCBitrateParameters::~RTCBitrateParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCBitrateParameters::wrapper_init_org_webRtc_RTCBitrateParameters() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  if (from.min_bitrate_bps.has_value()) {
    to.minBitrateBps = SafeInt<decltype(to.minBitrateBps)::value_type>(from.min_bitrate_bps.value());
  }
  if (from.current_bitrate_bps.has_value()) {
    to.currentBitrateBps = SafeInt<decltype(to.currentBitrateBps)::value_type>(from.current_bitrate_bps.value());
  }
  if (from.max_bitrate_bps.has_value()) {
    to.maxBitrateBps = SafeInt<decltype(to.maxBitrateBps)::value_type>(from.max_bitrate_bps.value());
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  if (from.minBitrateBps.has_value()) {
    to.min_bitrate_bps = (int)SafeInt<int>(from.minBitrateBps.value());
  }
  if (from.currentBitrateBps.has_value()) {
    to.current_bitrate_bps = (int)SafeInt<int>(from.currentBitrateBps.value());
  }
  if (from.maxBitrateBps.has_value()) {
    to.max_bitrate_bps = (int)SafeInt<int>(from.maxBitrateBps.value());
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
