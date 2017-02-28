
#include "impl_org_ortc_RTCSctpTransportStateChangeEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent::RTCSctpTransportStateChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportStateChangeEventPtr wrapper::org::ortc::RTCSctpTransportStateChangeEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent::~RTCSctpTransportStateChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportState wrapper::impl::org::ortc::RTCSctpTransportStateChangeEvent::get_state()
{
  return Helper::toWrapper(native_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType native)
{
  if (!native) return WrapperImplTypePtr();
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}
