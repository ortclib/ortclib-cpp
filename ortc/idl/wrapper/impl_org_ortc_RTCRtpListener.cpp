
#include "impl_org_ortc_RTCRtpListener.h"
#include "impl_org_ortc_RTCRtpUnhandledEvent.h"
#include "impl_org_ortc_RTCRtpTransport.h"
#include "impl_org_ortc_RTCRtpHeaderExtensionParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpListener::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpListener::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpListener::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpListener::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpListener::RTCRtpListener() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpListenerPtr wrapper::org::ortc::RTCRtpListener::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpListener>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpListener::~RTCRtpListener() noexcept
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCRtpListener::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpListener::wrapper_init_org_ortc_RTCRtpListener(wrapper::org::ortc::RTCRtpTransportPtr transport) noexcept
{
  native_ = NativeType::create(thisWeak_.lock(), RTCRtpTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpListener::wrapper_init_org_ortc_RTCRtpListener(
  wrapper::org::ortc::RTCRtpTransportPtr transport,
  shared_ptr< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > > headerExtensions
  ) noexcept(false)
{
  ::ortc::IRTPTypes::HeaderExtensionParametersList nativeList;
  if (headerExtensions) {
    for (auto iter = headerExtensions->begin(); iter != headerExtensions->end(); ++iter) {
      auto native = RTCRtpHeaderExtensionParameters::toNative(*iter);
      if (!native) continue;
      nativeList.push_back(*native);
    }
  }
  native_ = NativeType::create(thisWeak_.lock(), RTCRtpTransport::toNative(transport), nativeList);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpListener::setHeaderExtensions(shared_ptr< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > > headerExtensions) noexcept(false)
{
  ::ortc::IRTPTypes::HeaderExtensionParametersList nativeList;
  if (headerExtensions) {
    for (auto iter = headerExtensions->begin(); iter != headerExtensions->end(); ++iter) {
      auto native = RTCRtpHeaderExtensionParameters::toNative(*iter);
      if (!native) continue;
      nativeList.push_back(*native);
    }
  }
  native_->setHeaderExtensions(nativeList);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCRtpListener::get_objectId() noexcept
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpTransportPtr wrapper::impl::org::ortc::RTCRtpListener::get_transport() noexcept
{
  return RTCRtpTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpListener::wrapper_onObserverCountChanged(size_t count) noexcept
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onRTPListenerUnhandledRTP(
  IRTPListenerPtr listener,
  SSRCType ssrc,
  PayloadType payloadType,
  const char *mid,
  const char *rid
) noexcept
{
  onUnhandled(RTCRtpUnhandledEvent::toWrapper(ssrc, payloadType, mid, rid));
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
