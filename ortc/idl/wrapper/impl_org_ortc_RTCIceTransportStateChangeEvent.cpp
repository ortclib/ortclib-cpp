
#include "impl_org_ortc_RTCIceTransportStateChangeEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportStateChangeEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportStateChangeEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportStateChangeEvent::RTCIceTransportStateChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportStateChangeEventPtr wrapper::org::ortc::RTCIceTransportStateChangeEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceTransportStateChangeEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportStateChangeEvent::~RTCIceTransportStateChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportState wrapper::impl::org::ortc::RTCIceTransportStateChangeEvent::get_state()
{
  return Helper::toWrapper(state_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(::ortc::IICETransportTypes::States state)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->state_ = state;
  return pThis;
}
