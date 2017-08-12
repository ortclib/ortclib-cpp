
#include "impl_org_ortc_MessageQueue.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MessageQueue::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MessageQueue::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MessageQueue::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MessageQueue::MessageQueue()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MessageQueuePtr wrapper::org::ortc::MessageQueue::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MessageQueue>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MessageQueue::~MessageQueue()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MessageQueuePtr wrapper::org::ortc::MessageQueue::getDefaultForUi()
{
  return wrapper::impl::org::ortc::MessageQueue::toWrapper(zsLib::IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue());
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

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!impl) return NativeTypePtr();

  return impl->native_;
}
