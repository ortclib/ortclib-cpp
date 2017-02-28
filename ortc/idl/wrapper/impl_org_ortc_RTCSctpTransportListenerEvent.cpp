
#include "impl_org_ortc_RTCSctpTransportListenerEvent.h"
#include "impl_org_ortc_RTCSctpTransport.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransportListenerEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransportListenerEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpTransportListenerEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportListenerEvent::RTCSctpTransportListenerEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportListenerEventPtr wrapper::org::ortc::RTCSctpTransportListenerEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSctpTransportListenerEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportListenerEvent::~RTCSctpTransportListenerEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportPtr wrapper::impl::org::ortc::RTCSctpTransportListenerEvent::get_transport()
{
  return RTCSctpTransport::toWrapper(native_);
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

