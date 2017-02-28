
#include "impl_org_ortc_RTCCertificate.h"
#include "impl_org_ortc_RTCDtlsFingerprint.h"
#include "impl_org_ortc_Helper.h"
#include "impl_org_ortc_Json.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificate::RTCCertificate()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCCertificatePtr wrapper::org::ortc::RTCCertificate::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCCertificate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificate::~RTCCertificate()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCCertificatePtr > > wrapper::org::ortc::RTCCertificate::generateCertificate()
{
  return impl::org::ortc::RTCCertificate::toWrapper(::ortc::ICertificate::generateCertificate());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCCertificatePtr > > wrapper::org::ortc::RTCCertificate::generateCertificate(String keygenAlgorithm)
{
  if (keygenAlgorithm.isEmpty()) return generateCertificate();
  return impl::org::ortc::RTCCertificate::toWrapper(::ortc::ICertificate::generateCertificate(keygenAlgorithm));
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCCertificatePtr > > wrapper::org::ortc::RTCCertificate::generateCertificate(wrapper::org::ortc::JsonPtr keygenAlgorithm)
{
  if (!keygenAlgorithm) return generateCertificate();

  auto native = impl::org::ortc::Json::toNative(keygenAlgorithm);
  if (!native) return generateCertificate();

  return impl::org::ortc::RTCCertificate::toWrapper(::ortc::ICertificate::generateCertificate(native));
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCCertificate::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCCertificate::get_expires()
{
  return native_->expires();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsFingerprintPtr wrapper::impl::org::ortc::RTCCertificate::get_fingerprint()
{
  return RTCDtlsFingerprint::toWrapper(native_->fingerprint());
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificate::PromiseWithTypePtr wrapper::impl::org::ortc::RTCCertificate::toWrapper(NativePromiseWithCertificatePtr promise)
{
  if (!promise) return PromiseWithTypePtr();

  auto result = PromiseWithType::create(Helper::getGuiQueue());
  promise->thenClosure([result, promise] {
    if (promise->isRejected()) {
      Helper::reject(promise, result);
      return;
    }

    result->resolve(toWrapper(promise->value()));
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificatePtr wrapper::impl::org::ortc::RTCCertificate::toWrapper(NativeTypePtr native)
{
  if (!native) return RTCCertificatePtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCCertificate>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificate::NativeTypePtr wrapper::impl::org::ortc::RTCCertificate::toNative(wrapper::org::ortc::RTCCertificatePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  return std::dynamic_pointer_cast<RTCCertificate>(wrapper)->native_;
}
