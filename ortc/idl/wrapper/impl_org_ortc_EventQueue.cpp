
#include "impl_org_ortc_EventQueue.h"

#include <zsLib/IMessageQueueThread.h>

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
static wrapper::org::ortc::EventQueuePtr &getSingleton()
{
  static wrapper::org::ortc::EventQueuePtr singleton_ {};
  return singleton_;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::EventQueue::EventQueue() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueue::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::EventQueue>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::EventQueue::~EventQueue()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueue::getDefaultForUi() noexcept
{
#ifndef WINUWP
  auto result = std::make_shared<wrapper::impl::org::ortc::EventQueue>();
  result->queue_ = zsLib::IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue();
  return result;
#else
  return get_singleton();
#endif //ndef WINUWP
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueue::get_singleton() noexcept
{
  return getSingleton();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::EventQueue::set_singleton(wrapper::org::ortc::EventQueuePtr value) noexcept
{
  getSingleton() = value;
}
