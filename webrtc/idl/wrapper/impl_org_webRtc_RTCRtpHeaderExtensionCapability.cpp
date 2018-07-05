
#include "impl_org_webRtc_RTCRtpHeaderExtensionCapability.h"

#include "impl_org_webRtc_enums.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpparameters.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability::RTCRtpHeaderExtensionCapability() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpHeaderExtensionCapabilityPtr wrapper::org::webRtc::RTCRtpHeaderExtensionCapability::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability::~RTCRtpHeaderExtensionCapability() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability::wrapper_init_org_webRtc_RTCRtpHeaderExtensionCapability() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.uri = from.uri;
  if (from.preferred_id.has_value()) {
    to.preferredId = SafeInt<decltype(to.preferredId)::value_type>(from.preferred_id.value());
  }
  to.preferredEncrypt = from.preferred_encrypt;
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  to.uri = from.uri;
  if (from.preferredId.has_value()) {
    to.preferred_id = (int)SafeInt<int>(from.preferredId.value());
  }
  to.preferred_encrypt = from.preferredEncrypt;
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
