
#include "impl_org_ortc_RTCIceGathererStateChangeEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererStateChangeEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererStateChangeEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererStateChangeEvent::RTCIceGathererStateChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererStateChangeEventPtr wrapper::org::ortc::RTCIceGathererStateChangeEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGathererStateChangeEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererStateChangeEvent::~RTCIceGathererStateChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererState wrapper::impl::org::ortc::RTCIceGathererStateChangeEvent::get_state()
{
  return Helper::toWrapper(state_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(::ortc::IICEGathererTypes::States state)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->state_ = state;
  return pThis;
}

