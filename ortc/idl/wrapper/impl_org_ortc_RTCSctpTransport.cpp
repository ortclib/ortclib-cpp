
#include "impl_org_ortc_RTCSctpTransport.h"
#include "impl_org_ortc_RTCDtlsTransport.h"
#include "impl_org_ortc_RTCSctpCapabilities.h"
#include "impl_org_ortc_RTCSctpTransportListener.h"
#include "impl_org_ortc_RTCSctpTransportStateChangeEvent.h"
#include "impl_org_ortc_RTCDataChannelEvent.h"
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

using namespace wrapper::impl::org::ortc;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransport::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransport::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransport::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransport::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransport::RTCSctpTransport() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportPtr wrapper::org::ortc::RTCSctpTransport::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSctpTransport>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransport::~RTCSctpTransport() noexcept
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCSctpTransport::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransport::wrapper_init_org_ortc_RTCSctpTransport(wrapper::org::ortc::RTCDtlsTransportPtr transport) noexcept
{
  native_ = NativeType::create(thisWeak_.lock(), RTCDtlsTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransport::wrapper_init_org_ortc_RTCSctpTransport(
  wrapper::org::ortc::RTCDtlsTransportPtr transport,
  uint16_t localPort
  ) noexcept
{
  native_ = NativeType::create(thisWeak_.lock(), RTCDtlsTransport::toNative(transport), localPort);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportListenerPtr wrapper::org::ortc::RTCSctpTransport::listen(
  wrapper::org::ortc::RTCDtlsTransportPtr transport,
  wrapper::org::ortc::RTCSctpCapabilitiesPtr remoteCapabilities
  ) noexcept(false)
{
  auto native = wrapper::impl::org::ortc::RTCSctpCapabilities::toNative(remoteCapabilities);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  return wrapper::impl::org::ortc::RTCSctpTransportListener::toWrapper(NativeType::listen(::ortc::ISCTPTransportListenerDelegatePtr(), wrapper::impl::org::ortc::RTCDtlsTransport::toNative(transport), *native));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpCapabilitiesPtr wrapper::org::ortc::RTCSctpTransport::getCapabilities() noexcept
{
  return wrapper::impl::org::ortc::RTCSctpCapabilities::toWrapper(NativeType::getCapabilities());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransport::start(wrapper::org::ortc::RTCSctpCapabilitiesPtr remoteCapabilities) noexcept
{
  auto native = RTCSctpCapabilities::toNative(remoteCapabilities);
  ZS_ASSERT(native);
  native_->start(*native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransport::start(
  wrapper::org::ortc::RTCSctpCapabilitiesPtr remoteCapabilities,
  uint16_t remotePort
  ) noexcept
{
  auto native = RTCSctpCapabilities::toNative(remoteCapabilities);
  ZS_ASSERT(native);
  native_->start(*native, remotePort);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransport::stop() noexcept
{
  native_->stop();
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCSctpTransport::get_objectId() noexcept
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsTransportPtr wrapper::impl::org::ortc::RTCSctpTransport::get_transport() noexcept
{
  return RTCDtlsTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportState wrapper::impl::org::ortc::RTCSctpTransport::get_state() noexcept
{
  return Helper::toWrapper(native_->state());
}

//------------------------------------------------------------------------------
uint16_t wrapper::impl::org::ortc::RTCSctpTransport::get_port() noexcept
{
  return SafeInt<uint16_t>(native_->port());
}

//------------------------------------------------------------------------------
uint16_t wrapper::impl::org::ortc::RTCSctpTransport::get_localPort() noexcept
{
  return SafeInt<uint16_t>(native_->localPort());
}

//------------------------------------------------------------------------------
Optional< uint16_t > wrapper::impl::org::ortc::RTCSctpTransport::get_remotePort() noexcept
{
  Optional< uint16_t > result {};
  auto native = native_->remotePort();
  Helper::optionalSafeIntConvert(native, result);
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransport::wrapper_onObserverCountChanged(size_t count) noexcept
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onSCTPTransportStateChange(
  ISCTPTransportPtr transport,
  States state
) noexcept
{
  onStateChange(RTCSctpTransportStateChangeEvent::toWrapper(state));
}

//------------------------------------------------------------------------------
void WrapperImplType::onSCTPTransportDataChannel(
  ISCTPTransportPtr transport,
  ::ortc::IDataChannelPtr channel
) noexcept
{
  onDataChannel(RTCDataChannelEvent::toWrapper(channel));
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
