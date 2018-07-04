
#include "impl_org_webRtc_RTCRSAParams.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/sslidentity.h"
#include "impl_org_webRtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRSAParams::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRSAParams::RTCRSAParams() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRSAParamsPtr wrapper::org::webRtc::RTCRSAParams::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRSAParams>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRSAParams::~RTCRSAParams() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRSAParams::wrapper_init_org_webRtc_RTCRSAParams() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.modSize = SafeInt<decltype(to.modSize)::value_type>(from.mod_size);
  to.publicExponent = SafeInt<decltype(to.publicExponent)::value_type>(from.pub_exp);
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  if (from.modSize.has_value()) {
    to.mod_size = SafeInt<decltype(to.mod_size)>(from.modSize.value());
  }
  if (from.publicExponent.has_value()) {
    to.pub_exp = SafeInt<decltype(to.pub_exp)>(from.publicExponent.value());
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
  cloned->mod_size = rtc::kRsaDefaultModSize;
  cloned->pub_exp = rtc::kRsaDefaultExponent;
  apply(*converted, *cloned);
  return cloned;
}
