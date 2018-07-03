
#include "impl_org_webRtc_EventQueue.h"
#include "impl_org_webRtc_EventQueueMaker.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::EventQueueMaker::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::EventQueueMaker::~EventQueueMaker() noexcept
{
}

#if 0
//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr wrapper::org::webRtc::EventQueueMaker::bindQueue(AnyPtr queue) noexcept
{
  wrapper::org::webRtc::EventQueuePtr result = wrapper::org::webRtc::EventQueue::wrapper_create();
  result->wrapper_init_org_webRtc_EventQueue(queue);
  return result;
}

//------------------------------------------------------------------------------
AnyPtr wrapper::org::webRtc::EventQueueMaker::extractQueue(wrapper::org::webRtc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  return queue->get_queue();
}
#endif //0