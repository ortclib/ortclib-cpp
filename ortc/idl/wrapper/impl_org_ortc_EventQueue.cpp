
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

#ifdef WINUWP
#ifdef __cplusplus_winrt

namespace wrapper { namespace impl { namespace org { namespace ortc {
        ZS_DECLARE_STRUCT_PTR(EventQueueWrapperAnyCx);

        struct EventQueueWrapperAnyCx : Any
        {
          Windows::UI::Core::CoreDispatcher^ queue_ {nullptr};
        };
} } } }

#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

namespace wrapper { namespace impl { namespace org { namespace ortc {
        ZS_DECLARE_STRUCT_PTR(EventQueueWrapperAnyCx);

        struct EventQueueWrapperAnyWinrt : Any
        {
          winrt::Windows::UI::Core::CoreDispatcher queue_ {nullptr};
        };
} } } }

#endif // CPPWINRT_VERSION

#else


namespace wrapper { namespace impl { namespace org { namespace ortc {
        ZS_DECLARE_STRUCT_PTR(EventQueueWrapperAny);

        struct EventQueueWrapperAnyWinrt : Any
        {
          ::zsLib::IMessageQueuePtr queue_ {nullptr};
        };
} } } }

#endif //WINUWP


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

#ifdef WINUWP
#ifdef __cplusplus_winrt

wrapper::org::ortc::EventQueuePtr wrapper::impl::org::ortc::EventQueue::toWrapper(Windows::UI::Core::CoreDispatcher^ queue) noexcept
{
  auto any {make_shared<wrapper::impl::org::ortc::EventQueueWrapperAnyCx>()};
  any->queue_ = queue;
  auto result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(any);
  return result;
}

Windows::UI::Core::CoreDispatcher^ wrapper::impl::org::ortc::EventQueue::toNative_cx(wrapper::org::ortc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  AnyPtr any = queue->get_queue();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::ortc::EventQueueWrapperAnyCx, any);
  if (!castedAny) return nullptr;
  return castedAny->queue_;
}
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

wrapper::org::ortc::EventQueuePtr wrapper::impl::org::ortc::EventQueue::toWrapper(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept
{  
  auto any {make_shared<wrapper::impl::org::ortc::EventQueueWrapperAnyWinrt>()};
  any->queue_ = queue;
  auto result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(any);
  return result;
}

winrt::Windows::UI::Core::CoreDispatcher wrapper::impl::org::ortc::EventQueue::toNative_winrt(wrapper::org::ortc::EventQueuePtr queue) noexcept
{            
  if (!queue) return nullptr;
  AnyPtr any = queue->get_queue();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::ortc::EventQueueWrapperAnyWinrt, any);
  if (!castedAny) return nullptr;
  return castedAny->queue_;
}

#endif // CPPWINRT_VERSION

#else

wrapper::org::ortc::EventQueuePtr wrapper::impl::org::ortc::EventQueue::toWrapper(::zsLib::IMessageQueuePtr queue) noexcept
{  
  auto any {make_shared<wrapper::impl::org::ortc::EventQueueWrapperAny>()};
  any->queue_ = queue;
  auto result = wrapper::org::ortc::EventQueue::wrapper_create();
  result->wrapper_init_org_ortc_EventQueue(any);
  return result;
}

static ::zsLib::IMessageQueuePtr wrapper::impl::org::ortc::EventQueue::toNative(wrapper::org::ortc::EventQueuePtr queue) noexcept
{
  if (!queue) return nullptr;
  AnyPtr any = queue->get_queue();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::ortc::EventQueueWrapperAny, any);
  if (!castedAny) return nullptr;
  return castedAny->queue_;
}

#endif //WINUWP
