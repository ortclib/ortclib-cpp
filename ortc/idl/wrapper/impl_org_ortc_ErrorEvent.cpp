
#include "impl_org_ortc_ErrorEvent.h"
#include "impl_org_ortc_Error.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::ErrorEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::ErrorEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ErrorEvent::ErrorEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ErrorEventPtr wrapper::org::ortc::ErrorEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ErrorEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ErrorEvent::~ErrorEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ErrorPtr wrapper::impl::org::ortc::ErrorEvent::get_error()
{
  return error_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(::ortc::ErrorAnyPtr error)
{
  if (!error) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->error_ = Error::toWrapper(error);
  return pThis;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(wrapper::org::ortc::ErrorPtr error)
{
  if (!error) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->error_ = error;
  return pThis;
}
