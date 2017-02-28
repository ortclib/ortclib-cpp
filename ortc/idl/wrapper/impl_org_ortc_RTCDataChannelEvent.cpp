
#include "impl_org_ortc_RTCDataChannelEvent.h"
#include "impl_org_ortc_RTCDataChannel.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannelEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannelEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannelEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelEvent::RTCDataChannelEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelEventPtr wrapper::org::ortc::RTCDataChannelEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDataChannelEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelEvent::~RTCDataChannelEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelPtr wrapper::impl::org::ortc::RTCDataChannelEvent::get_dataChannel()
{
  return RTCDataChannel::toWrapper(native_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}
