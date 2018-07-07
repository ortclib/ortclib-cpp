
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include "Org.WebRtc.EventQueue.h"

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueue > Org::WebRtc::implementation::EventQueue::ToCppWinrtImpl(wrapper::org::webRtc::EventQueuePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::WebRtc::implementation::EventQueue>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueue > Org::WebRtc::implementation::EventQueue::ToCppWinrtImpl(Org::WebRtc::EventQueue const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::EventQueue > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::EventQueue>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueue > Org::WebRtc::implementation::EventQueue::ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueue > Org::WebRtc::implementation::EventQueue::ToCppWinrtImpl(Org::WebRtc::IEventQueue const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::EventQueue > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::EventQueue>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrt(wrapper::org::webRtc::EventQueuePtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::EventQueue {nullptr};
  return result.as< Org::WebRtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrt(Org::WebRtc::EventQueue const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value)
{
  if (!value) return Org::WebRtc::EventQueue {nullptr};
  return value.as< Org::WebRtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrt(Org::WebRtc::IEventQueue const & value)
{
  if (!value) return Org::WebRtc::EventQueue {nullptr};
  return value.as< Org::WebRtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrtInterface(wrapper::org::webRtc::EventQueuePtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::IEventQueue {nullptr};
  return result.as< Org::WebRtc::IEventQueue >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrtInterface(Org::WebRtc::EventQueue const & value)
{
  if (!value) return Org::WebRtc::IEventQueue {nullptr};
  return value.as< Org::WebRtc::IEventQueue >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value)
{
  if (!value) return Org::WebRtc::IEventQueue {nullptr};
  return value.as< Org::WebRtc::IEventQueue >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueue::ToCppWinrtInterface(Org::WebRtc::IEventQueue const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::WebRtc::implementation::EventQueue::FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value)
{
  if (!value) return wrapper::org::webRtc::EventQueuePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::WebRtc::implementation::EventQueue::FromCppWinrt(Org::WebRtc::EventQueue const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::WebRtc::implementation::EventQueue::FromCppWinrt(wrapper::org::webRtc::EventQueuePtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::WebRtc::implementation::EventQueue::FromCppWinrt(Org::WebRtc::IEventQueue const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueue Org::WebRtc::implementation::EventQueue::CastFromIEventQueue(Org::WebRtc::IEventQueue const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_WebRtc_EventQueue(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::EventQueue >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

#if 0
//------------------------------------------------------------------------------
Org::WebRtc::implementation::EventQueue::EventQueue(Windows::Foundation::IInspectable const & queue)
 : native_(wrapper::org::webRtc::EventQueue::wrapper_create())
{
  if (!native_) {throw hresult_error(E_POINTER);}
  native_->wrapper_init_org_webRtc_EventQueue(::Internal::Helper::FromCppWinrt(queue));
}
#endif //0

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueue::GetDefaultForUi()
{
  Org::WebRtc::IEventQueue result {nullptr};
  result = ::Internal::Helper::ToCppWinrt_Org_WebRtc_EventQueue(wrapper::org::webRtc::EventQueue::getDefaultForUi());
  return result;
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueue::Singleton()
{
  return ::Internal::Helper::ToCppWinrt_Org_WebRtc_EventQueue(wrapper::org::webRtc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void Org::WebRtc::implementation::EventQueue::Singleton(Org::WebRtc::IEventQueue const & value)
{
  wrapper::org::webRtc::EventQueue::set_singleton(::Internal::Helper::FromCppWinrt_Org_WebRtc_EventQueue(value));
}

#if 0
//------------------------------------------------------------------------------
Windows::Foundation::IInspectable Org::WebRtc::implementation::EventQueue::Queue()
{
  if (!native_) {throw hresult_error(E_POINTER);}
  return ::Internal::Helper::ToCppWinrt(native_->get_queue());
}
#endif //0

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
