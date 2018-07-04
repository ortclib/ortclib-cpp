
#include "impl_org_webRtc_RTCDtlsFingerprint.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/sslfingerprint.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtlsFingerprint::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDtlsFingerprint::RTCDtlsFingerprint() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtlsFingerprintPtr wrapper::org::webRtc::RTCDtlsFingerprint::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCDtlsFingerprint>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDtlsFingerprint::~RTCDtlsFingerprint() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDtlsFingerprint::wrapper_init_org_webRtc_RTCDtlsFingerprint() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDtlsFingerprint::wrapper_init_org_webRtc_RTCDtlsFingerprint(wrapper::org::webRtc::RTCDtlsFingerprintPtr source) noexcept
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const std::string &alg, NativeTypeUniPtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->algorithm = alg;
  result->value = native->GetRfc4572Fingerprint();
  return result;
}

//------------------------------------------------------------------------------
NativeTypeUniPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypeUniPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypeUniPtr();

  std::unique_ptr<NativeType> native(::rtc::SSLFingerprint::CreateFromRfc4572(converted->algorithm, converted->value));
  return std::move(native);
}
