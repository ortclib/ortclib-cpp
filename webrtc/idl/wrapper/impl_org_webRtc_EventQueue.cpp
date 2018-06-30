
#ifdef WINUWP

#ifdef __cplusplus_winrt
#include <windows.ui.core.h>
#endif //__cplusplus_winrt

#ifdef __has_include
#if __has_include(<winrt/windows.ui.core.h>)
#include <winrt/windows.ui.core.h>
#endif //__has_include(<winrt/windows.ui.core.h>)
#endif //__has_include

#else

#include <zsLib/IMessageQueue.h>
#endif //WINUWP

#include "impl_org_webRtc_EventQueue.h"

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

#ifdef WINUWP
#ifdef __cplusplus_winrt

namespace wrapper { namespace impl { namespace org { namespace webRtc {
        ZS_DECLARE_STRUCT_PTR(EventQueueWrapperAnyCx);

        struct EventQueueWrapperAnyCx : Any
        {
          Windows::UI::Core::CoreDispatcher^ queue_ {nullptr};
        };
} } } }

#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

namespace wrapper { namespace impl { namespace org { namespace webRtc {
        ZS_DECLARE_STRUCT_PTR(EventQueueWrapperAnyCx);

        struct EventQueueWrapperAnyWinrt : Any
        {
          winrt::Windows::UI::Core::CoreDispatcher queue_ {nullptr};
        };
} } } }

#endif // CPPWINRT_VERSION

#else


namespace wrapper { namespace impl { namespace org { namespace webRtc {
        ZS_DECLARE_STRUCT_PTR(EventQueueWrapperAny);

        struct EventQueueWrapperAny : Any
        {
          ::zsLib::IMessageQueuePtr queue_ {nullptr};
        };
} } } }

#endif //WINUWP


//------------------------------------------------------------------------------
static wrapper::org::webRtc::EventQueuePtr &getSingleton() noexcept
{
  static wrapper::org::webRtc::EventQueuePtr singleton_ {};
  return singleton_;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::EventQueue::EventQueue() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr wrapper::org::webRtc::EventQueue::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::EventQueue>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::EventQueue::~EventQueue() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr wrapper::org::webRtc::EventQueue::getDefaultForUi() noexcept
{
#ifndef WINUWP
  auto result {std::make_shared<wrapper::impl::org::webRtc::EventQueue>()};
  auto any {std::make_shared<wrapper::impl::org::webRtc::EventQueueWrapperAny>()};
  any->queue_ = zsLib::IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue();
  result->queue_ = any;
  return result;
#else
  return get_singleton();
#endif //ndef WINUWP
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr wrapper::org::webRtc::EventQueue::get_singleton() noexcept
{
  return getSingleton();
}

//------------------------------------------------------------------------------
void wrapper::org::webRtc::EventQueue::set_singleton(wrapper::org::webRtc::EventQueuePtr value) noexcept
{
  getSingleton() = value;
}

#ifdef WINUWP
#ifdef __cplusplus_winrt

wrapper::org::webRtc::EventQueuePtr wrapper::impl::org::webRtc::EventQueue::toWrapper(Windows::UI::Core::CoreDispatcher^ queue) noexcept
{
  auto any {make_shared<wrapper::impl::org::webRtc::EventQueueWrapperAnyCx>()};
  any->queue_ = queue;
  auto result = wrapper::org::webRtc::EventQueue::wrapper_create();
  result->wrapper_init_org_webRtc_EventQueue(any);
  return result;
}

Windows::UI::Core::CoreDispatcher^ wrapper::impl::org::webRtc::EventQueue::toNative_cx(wrapper::org::webRtc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  AnyPtr any = queue->get_queue();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webRtc::EventQueueWrapperAnyCx, any);
  if (!castedAny) return nullptr;
  return castedAny->queue_;
}
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

wrapper::org::webRtc::EventQueuePtr wrapper::impl::org::webRtc::EventQueue::toWrapper(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept
{  
  auto any {make_shared<wrapper::impl::org::webRtc::EventQueueWrapperAnyWinrt>()};
  any->queue_ = queue;
  auto result = wrapper::org::webRtc::EventQueue::wrapper_create();
  result->wrapper_init_org_webRtc_EventQueue(any);
  return result;
}

winrt::Windows::UI::Core::CoreDispatcher wrapper::impl::org::webRtc::EventQueue::toNative_winrt(wrapper::org::webRtc::EventQueuePtr queue) noexcept
{            
  if (!queue) return nullptr;
  AnyPtr any = queue->get_queue();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webRtc::EventQueueWrapperAnyWinrt, any);
  if (!castedAny) return nullptr;
  return castedAny->queue_;
}

#endif // CPPWINRT_VERSION

#else

wrapper::org::webRtc::EventQueuePtr wrapper::impl::org::webRtc::EventQueue::toWrapper(::zsLib::IMessageQueuePtr queue) noexcept
{  
  auto any {make_shared<wrapper::impl::org::webRtc::EventQueueWrapperAny>()};
  any->queue_ = queue;
  auto result = wrapper::org::webRtc::EventQueue::wrapper_create();
  result->wrapper_init_org_webRtc_EventQueue(any);
  return result;
}

::zsLib::IMessageQueuePtr wrapper::impl::org::webRtc::EventQueue::toNative(wrapper::org::webRtc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  AnyPtr any = queue->get_queue();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webRtc::EventQueueWrapperAny, any);
  if (!castedAny) return nullptr;
  return castedAny->queue_;
}

#endif //WINUWP
