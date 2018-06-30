
#include "impl_org_ortc_RTCDtlsTransport.h"
#include "impl_org_ortc_RTCDtlsParameters.h"
#include "impl_org_ortc_RTCIceTransport.h"
#include "impl_org_ortc_RTCCertificate.h"
#include "impl_org_ortc_RTCDtlsTransportStateChangeEvent.h"
#include "impl_org_ortc_RTCDtlsCertificateBinary.h"
#include "impl_org_ortc_ErrorEvent.h"
#include "impl_org_ortc_Helper.h"

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

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_wrapper); } } } }

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsTransport::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsTransport::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsTransport::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsTransport::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsTransport::RTCDtlsTransport() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsTransportPtr wrapper::org::ortc::RTCDtlsTransport::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDtlsTransport>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsTransport::~RTCDtlsTransport() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCDtlsTransport::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransport::wrapper_init_org_ortc_RTCDtlsTransport(
  wrapper::org::ortc::RTCIceTransportPtr iceTransport,
  shared_ptr< list< wrapper::org::ortc::RTCCertificatePtr > > certificates
  ) noexcept(false)
{
  ZS_THROW_INVALID_ARGUMENT_IF(!iceTransport);

  list< ::ortc::ICertificatePtr > nativeList;
  if (certificates) {
    for (auto iter = certificates->begin(); iter != certificates->end(); ++iter) {
      auto native = RTCCertificate::toNative(*iter);
      if (!native) continue;
      nativeList.push_back(native);
    }
  }
  
  native_ = IDTLSTransport::create(thisWeak_.lock(), RTCIceTransport::toNative(iceTransport), nativeList);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsParametersPtr wrapper::impl::org::ortc::RTCDtlsTransport::remoteParameters() noexcept
{
  return RTCDtlsParameters::toWrapper(native_->getRemoteParameters());
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCDtlsCertificateBinaryPtr > > wrapper::impl::org::ortc::RTCDtlsTransport::getRemoteCertificates() noexcept
{
  auto result = make_shared< list< wrapper::org::ortc::RTCDtlsCertificateBinaryPtr > >();
  auto native = native_->getRemoteCertificates();
  if (native) {
    for (auto iter = native->begin(); iter != native->end(); ++iter) {
      auto wrapper = RTCDtlsCertificateBinary::toWrapper(*iter);
      if (!wrapper) continue;
      result->push_back(wrapper);
    }
  }
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransport::start(wrapper::org::ortc::RTCDtlsParametersPtr remoteParameters) noexcept(false)
{
  ZS_THROW_INVALID_ARGUMENT_IF(!remoteParameters);
  auto native = RTCDtlsParameters::toNative(remoteParameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  native_->start(*native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransport::stop() noexcept
{
  native_->stop();
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCDtlsTransport::get_objectId() noexcept
{
  return native_->getID();
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCCertificatePtr > > wrapper::impl::org::ortc::RTCDtlsTransport::get_certificates() noexcept
{
  auto result = make_shared< list< wrapper::org::ortc::RTCCertificatePtr > >();
  auto native = native_->certificates();
  if (native) {
    for (auto iter = native->begin(); iter != native->end(); ++iter) {
      auto wrapper = RTCCertificate::toWrapper(*iter);
      if (!wrapper) continue;
      result->push_back(wrapper);
    }
  }
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportPtr wrapper::impl::org::ortc::RTCDtlsTransport::get_transport() noexcept
{
  return RTCIceTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsTransportState wrapper::impl::org::ortc::RTCDtlsTransport::get_state() noexcept
{
  return Helper::toWrapper(native_->state());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsParametersPtr wrapper::impl::org::ortc::RTCDtlsTransport::get_localParameters() noexcept
{
  return RTCDtlsParameters::toWrapper(native_->getLocalParameters());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransport::wrapper_onObserverCountChanged(size_t count) noexcept
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onDTLSTransportStateChange(
                                                 IDTLSTransportPtr transport,
                                                 IDTLSTransport::States state
                                                 ) noexcept
{
  onStateChange(RTCDtlsTransportStateChangeEvent::toWrapper(Helper::toWrapper(state)));
}

//------------------------------------------------------------------------------
void WrapperImplType::onDTLSTransportError(
                                           IDTLSTransportPtr transport,
                                           ::ortc::ErrorAnyPtr error
                                           ) noexcept
{
  onError(ErrorEvent::toWrapper(error));
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr track) noexcept
{
  if (!track) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = track;
  result->defaultSubscription_ = false;
  result->subscribe();
  return result;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto result = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!result) return NativeTypePtr();
  return result->native_;
}

//------------------------------------------------------------------------------
void WrapperImplType::subscribe() noexcept
{
  if (defaultSubscription_) return;
  if (!native_) return;

  zsLib::AutoLock lock(lock_);
  if (subscriptionCount_ < 1) {
    if (!subscription_) return;
    subscription_->cancel();
    return;
  }
  if (subscription_) return;
  subscription_ = native_->subscribe(thisWeak_.lock());
}

