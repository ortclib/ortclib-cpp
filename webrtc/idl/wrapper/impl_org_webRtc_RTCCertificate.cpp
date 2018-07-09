
#include "impl_org_webRtc_RTCCertificate.h"
#include "impl_org_webRtc_RTCDtlsFingerprint.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/rtccertificate.h"
#include "rtc_base/sslidentity.h"
#include "rtc_base/sslfingerprint.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/date.h>
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

using namespace date;

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCCertificate::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtlsFingerprint, UseFingerprint);

typedef WrapperImplType::NativeTypeScopedPtr NativeTypeScopedPtr;

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCCertificate::RTCCertificate() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCCertificatePtr wrapper::org::webRtc::RTCCertificate::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCCertificate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCCertificate::~RTCCertificate() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::webRtc::RTCCertificate::get_expires() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return zsLib::Time();

  auto msSince1970 = native_->Expires();
  zsLib::Time t = day_point(jan / 1 / 1970);
  zsLib::Milliseconds converted = zsLib::Milliseconds(SafeInt<zsLib::Milliseconds::rep>(msSince1970));
  return t + converted;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCDtlsFingerprintPtr > > wrapper::impl::org::webRtc::RTCCertificate::get_fingerprints() noexcept
{
  typedef list< wrapper::org::webRtc::RTCDtlsFingerprintPtr > ResultType;

  auto result = make_shared<ResultType>();

  ZS_ASSERT(native_);
  if (!native_) return result;

  std::string digest_alg;
  auto &sslCert = native_->ssl_certificate();
  if (!sslCert.GetSignatureDigestAlgorithm(&digest_alg)) return result;

  std::unique_ptr<::rtc::SSLFingerprint> nativeFingerprint(::rtc::SSLFingerprint::Create(digest_alg, &sslCert));
  
  auto wrapperFingerprint = UseFingerprint::toWrapper(digest_alg, std::move(nativeFingerprint));
  if (wrapperFingerprint) {
    result->push_back(wrapperFingerprint);
  }
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  auto wrapper = make_shared<WrapperImplType>();
  wrapper->thisWeak_ = wrapper;
  wrapper->native_ = NativeTypeScopedPtr(native);
  return wrapper;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypeScopedPtr native) noexcept
{
  return toWrapper(native.get());
}

//------------------------------------------------------------------------------
NativeTypeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypeScopedPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypeScopedPtr();
  return converted->native_;
}

