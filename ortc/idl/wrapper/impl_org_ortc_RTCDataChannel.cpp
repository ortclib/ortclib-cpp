
#include "impl_org_ortc_RTCDataChannel.h"
#include "impl_org_ortc_RTCDataChannelParameters.h"
#include "impl_org_ortc_RTCDataTransport.h"
#include "impl_org_ortc_RTCMessageEvent.h"
#include "impl_org_ortc_RTCDataChannelStateChangeEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannel::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannel::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannel::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannel::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannel::RTCDataChannel() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelPtr wrapper::org::ortc::RTCDataChannel::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDataChannel>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannel::~RTCDataChannel() noexcept
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCDataChannel::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::wrapper_init_org_ortc_RTCDataChannel(
  wrapper::org::ortc::RTCDataTransportPtr transport,
  wrapper::org::ortc::RTCDataChannelParametersPtr params
  ) noexcept
{
  auto native = RTCDataChannelParameters::toNative(params);
  ZS_ASSERT(params);
  native_ = NativeType::create(thisWeak_.lock(), RTCDataTransport::toNative(transport), *native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::close() noexcept
{
  native_->close();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::send(String text) noexcept
{
  native_->send(text);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::send(SecureByteBlockPtr data) noexcept
{
  if (!data) return;
  native_->send(*data);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCDataChannel::get_objectId() noexcept
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataTransportPtr wrapper::impl::org::ortc::RTCDataChannel::get_transport() noexcept
{
  return RTCDataTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelParametersPtr wrapper::impl::org::ortc::RTCDataChannel::get_parameters() noexcept
{
  return RTCDataChannelParameters::toWrapper(native_->parameters());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelState wrapper::impl::org::ortc::RTCDataChannel::get_readyState() noexcept
{
  return Helper::toWrapper(native_->readyState());
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCDataChannel::get_bufferedAmount() noexcept
{
  return SafeInt<uint64_t>(native_->bufferedAmount());
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCDataChannel::get_bufferedAmountLowThreshold() noexcept
{
  return SafeInt<uint64_t>(native_->bufferedAmountLowThreshold());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::set_bufferedAmountLowThreshold(uint64_t value) noexcept
{
  native_->bufferedAmountLowThreshold(SafeInt<size_t>(value));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannel::get_binaryType() noexcept
{
  return native_->binaryType();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::set_binaryType(String value) noexcept
{
  native_->binaryType(value);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannel::wrapper_onObserverCountChanged(size_t count) noexcept
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onDataChannelStateChange(
  IDataChannelPtr channel,
  ::ortc::IDataChannelTypes::States state
) noexcept
{
  onStateChange(RTCDataChannelStateChangeEvent::toWrapper(state));

  switch (state) {
    case ::ortc::IDataChannelTypes::State_Open:   onOpen(); break;
    case ::ortc::IDataChannelTypes::State_Closed: onClose(); break;
    default: break;
  }
}

//------------------------------------------------------------------------------
void WrapperImplType::onDataChannelError(
  IDataChannelPtr channel,
  ::ortc::ErrorAnyPtr error
) noexcept
{
  onError(ErrorEvent::toWrapper(error));
}

//------------------------------------------------------------------------------
void WrapperImplType::onDataChannelBufferedAmountLow(IDataChannelPtr channel) noexcept
{
  onBufferedAmountLow();
}

//------------------------------------------------------------------------------
void WrapperImplType::onDataChannelMessage(
  IDataChannelPtr channel,
  MessageEventDataPtr data
) noexcept
{
  onMessage(RTCMessageEvent::toWrapper(data));
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
