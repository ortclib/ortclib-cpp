
#include "impl_org_webrtc_EventQueue.h"
#include "impl_org_webrtc_EventQueueMaker.h"

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
wrapper::impl::org::webrtc::EventQueueMaker::~EventQueueMaker() noexcept
{
}

#if 0
//------------------------------------------------------------------------------
wrapper::org::webrtc::EventQueuePtr wrapper::org::webrtc::EventQueueMaker::bindQueue(AnyPtr queue) noexcept
{
  wrapper::org::webrtc::EventQueuePtr result = wrapper::org::webrtc::EventQueue::wrapper_create();
  result->wrapper_init_org_webrtc_EventQueue(queue);
  return result;
}

//------------------------------------------------------------------------------
AnyPtr wrapper::org::webrtc::EventQueueMaker::extractQueue(wrapper::org::webrtc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  return queue->get_queue();
}
#endif //0