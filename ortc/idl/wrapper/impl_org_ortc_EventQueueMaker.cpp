
#include "impl_org_ortc_EventQueue.h"
#include "impl_org_ortc_EventQueueMaker.h"

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
wrapper::impl::org::ortc::EventQueueMaker::~EventQueueMaker()
{
}

#ifdef WINUWP
#ifdef __cplusplus_winrt
//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueueMaker::bindQueue(Windows::UI::Core::CoreDispatcher^ queue) noexcept
{
  wrapper::org::ortc::EventQueuePtr result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(queue);
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Core::CoreDispatcher^ wrapper::org::ortc::EventQueueMaker::extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  return queue->get_queue();
}
#else // __cplusplus_winrt

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueueMaker::bindQueue(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept
{
  wrapper::org::ortc::EventQueuePtr result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(queue);
  return result;
}

//------------------------------------------------------------------------------
winrt::Windows::UI::Core::CoreDispatcher wrapper::org::ortc::EventQueueMaker::extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  return queue->get_queue();
}

#endif //__cplusplus_winrt
#else //WINUWP

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueueMaker::bindQueue(::zsLib::IMessageQueuePtr queue) noexcept
{
  wrapper::org::ortc::EventQueuePtr result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(queue);
  return result;
}

//------------------------------------------------------------------------------
::zsLib::IMessageQueuePtr wrapper::org::ortc::EventQueueMaker::extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  return queue->get_queue();
}

#endif //WINUWP
