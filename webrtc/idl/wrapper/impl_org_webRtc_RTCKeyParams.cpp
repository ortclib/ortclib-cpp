
#include "impl_org_webRtc_RTCKeyParams.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRSAParams.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/sslidentity.h"
#include "impl_org_webRtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCKeyParams::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRSAParams, UseRsaParams);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCKeyParams::RTCKeyParams() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyParamsPtr wrapper::org::webRtc::RTCKeyParams::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCKeyParams>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCKeyParams::~RTCKeyParams() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCKeyParams::wrapper_init_org_webRtc_RTCKeyParams() noexcept
{
  native_ = make_shared<NativeType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCKeyParams::wrapper_init_org_webRtc_RTCKeyParams(wrapper::org::webRtc::RTCKeyType key_type) noexcept
{
  native_ = make_shared<NativeType>(UseEnum::toNative(key_type));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyParamsPtr wrapper::org::webRtc::RTCKeyParams::createRsa() noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(NativeType::RSA());
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyParamsPtr wrapper::org::webRtc::RTCKeyParams::createRsaWithParams(wrapper::org::webRtc::RTCRSAParamsPtr params) noexcept
{
  if (!params) return createRsa();

  auto native = UseRsaParams::toNative(params);
  if (!native) return createRsa();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(NativeType::RSA(native->mod_size, native->pub_exp));
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyParamsPtr wrapper::org::webRtc::RTCKeyParams::createEcdsa() noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(NativeType::ECDSA());
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyParamsPtr wrapper::org::webRtc::RTCKeyParams::createEcdsaWithCurve(wrapper::org::webRtc::RTCECCurve curve) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(NativeType::ECDSA(UseEnum::toNative(curve)));
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCKeyParams::get_valid() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  return native_->IsValid();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRSAParamsPtr wrapper::impl::org::webRtc::RTCKeyParams::get_rsa() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  return UseRsaParams::toWrapper(native_->rsa_params());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCECCurve wrapper::impl::org::webRtc::RTCKeyParams::get_ec() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCECCurve {};
  return UseEnum::toWrapper(native_->ec_curve());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCKeyType wrapper::impl::org::webRtc::RTCKeyParams::get_type() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCKeyType{};
  return UseEnum::toWrapper(native_->type());
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::createDefault()
{
  auto wrapper = make_shared<WrapperImplType>();
  wrapper->thisWeak_ = wrapper;
  wrapper->native_ = make_shared<NativeType>();
  return wrapper;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto wrapper = make_shared<WrapperImplType>();
  wrapper->thisWeak_ = wrapper;
  wrapper->native_ = make_shared<NativeType>(native);
  return wrapper;
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
