
#include "impl_org_webRtc_RTCDataChannelEvent.h"
#include "impl_org_webRtc_RTCDataChannel.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDataChannelEvent::RTCDataChannelEvent() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelEventPtr wrapper::org::webRtc::RTCDataChannelEvent::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCDataChannelEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDataChannelEvent::~RTCDataChannelEvent() noexcept
{
  thisWeak_.reset();
}


//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelPtr wrapper::impl::org::webRtc::RTCDataChannelEvent::get_channel() noexcept
{
  return dataChannel_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseDataChannelPtr value) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->dataChannel_ = value;
  return result;
}
