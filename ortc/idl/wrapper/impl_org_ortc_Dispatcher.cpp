
#include "impl_org_ortc_Dispatcher.h"

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
wrapper::impl::org::ortc::Dispatcher::Dispatcher()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::DispatcherPtr wrapper::org::ortc::Dispatcher::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::Dispatcher>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Dispatcher::~Dispatcher()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::Dispatcher::wrapper_init_org_ortc_Dispatcher(AnyPtr source)
{
  native_ = source;
}

//------------------------------------------------------------------------------
AnyPtr wrapper::org::ortc::Dispatcher::get_source()
{
  AnyPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Dispatcher::set_source(AnyPtr value)
{
}

//------------------------------------------------------------------------------
AnyPtr wrapper::impl::org::ortc::Dispatcher::toNative(wrapper::org::ortc::DispatcherPtr wrapper)
{
  if (!wrapper) return AnyPtr();

  auto impl = std::dynamic_pointer_cast<Dispatcher>(wrapper);
  if (!impl) return AnyPtr();

  return impl->native_;
}
