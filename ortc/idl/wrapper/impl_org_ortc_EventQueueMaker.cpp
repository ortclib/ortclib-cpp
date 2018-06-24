
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

#if 0
//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr wrapper::org::ortc::EventQueueMaker::bindQueue(AnyPtr queue) noexcept
{
  wrapper::org::ortc::EventQueuePtr result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(queue);
  return result;
}

//------------------------------------------------------------------------------
AnyPtr wrapper::org::ortc::EventQueueMaker::extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  return queue->get_queue();
}
#endif //0