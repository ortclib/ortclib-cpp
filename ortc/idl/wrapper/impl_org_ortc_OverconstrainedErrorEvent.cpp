
#include "impl_org_ortc_OverconstrainedErrorEvent.h"
#include "impl_org_ortc_OverconstrainedError.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::OverconstrainedErrorEvent::OverconstrainedErrorEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::OverconstrainedErrorEventPtr wrapper::org::ortc::OverconstrainedErrorEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::OverconstrainedErrorEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::OverconstrainedErrorEvent::~OverconstrainedErrorEvent()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::OverconstrainedErrorEvent::wrapper_init_org_ortc_OverconstrainedErrorEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::OverconstrainedErrorPtr wrapper::impl::org::ortc::OverconstrainedErrorEvent::get_error()
{
  return error_;
}

wrapper::impl::org::ortc::OverconstrainedErrorEventPtr wrapper::impl::org::ortc::OverconstrainedErrorEvent::toWrapper(OverconstrainedErrorPtr error)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::OverconstrainedErrorEvent>();
  pThis->thisWeak_ = pThis;
  pThis->error_ = error;
  pThis->wrapper_init_org_ortc_OverconstrainedErrorEvent();
  return pThis;
}

