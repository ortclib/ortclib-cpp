
#include "impl_org_webrtc_RTCCertificate.h"

#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/rtccertificate.h"
#include "impl_org_webrtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCCertificate::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCCertificate::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCCertificate::NativeType, NativeType);

typedef WrapperImplType::NativeTypeScopedPtr NativeTypeScopedPtr;

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCCertificate::RTCCertificate() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCCertificatePtr wrapper::org::webrtc::RTCCertificate::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCCertificate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCCertificate::~RTCCertificate() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::webrtc::RTCCertificate::get_expires() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return zsLib::Time();

  auto msSince1970 = native_->Expires();
  zsLib::Time t = day_point(jan / 1 / 1970);
  zsLib::Milliseconds converted = zsLib::Milliseconds(SafeInt<zsLib::Milliseconds::rep>(msSince1970));
  return t + converted;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webrtc::RTCDtlsFingerprintPtr > > wrapper::impl::org::webrtc::RTCCertificate::get_fingerprints() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return shared_ptr< list< wrapper::org::webrtc::RTCDtlsFingerprintPtr > >();

#pragma ZS_BUILD_NOTE("LATER","No direct api in webrtc to obtain certificates given algorithms")
  return make_shared< list< wrapper::org::webrtc::RTCDtlsFingerprintPtr > >();
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
NativeTypeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypeScopedPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypeScopedPtr();
  return converted->native_;
}

