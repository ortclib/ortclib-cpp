
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include "Org.Webrtc.EventQueue.h"

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueue > Org::Webrtc::implementation::EventQueue::ToCppWinrtImpl(wrapper::org::webRtc::EventQueuePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Webrtc::implementation::EventQueue>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueue > Org::Webrtc::implementation::EventQueue::ToCppWinrtImpl(Org::Webrtc::EventQueue const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::EventQueue > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::EventQueue>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueue > Org::Webrtc::implementation::EventQueue::ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueue > Org::Webrtc::implementation::EventQueue::ToCppWinrtImpl(Org::Webrtc::IEventQueue const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::EventQueue > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::EventQueue>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrt(wrapper::org::webRtc::EventQueuePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrt(Org::Webrtc::EventQueue const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value)
{
  return value.as< Org::Webrtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrt(Org::Webrtc::IEventQueue const & value)
{
  return value.as< Org::Webrtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrtInterface(wrapper::org::webRtc::EventQueuePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::IEventQueue >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrtInterface(Org::Webrtc::EventQueue const & value)
{
  return value.as< Org::Webrtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value)
{
  return value.as< Org::Webrtc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueue::ToCppWinrtInterface(Org::Webrtc::IEventQueue const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::Webrtc::implementation::EventQueue::FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value)
{
  if (!value) return wrapper::org::webRtc::EventQueuePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::Webrtc::implementation::EventQueue::FromCppWinrt(Org::Webrtc::EventQueue const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::Webrtc::implementation::EventQueue::FromCppWinrt(wrapper::org::webRtc::EventQueuePtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::Webrtc::implementation::EventQueue::FromCppWinrt(Org::Webrtc::IEventQueue const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueue Org::Webrtc::implementation::EventQueue::CastFromIEventQueue(Org::Webrtc::IEventQueue const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Webrtc_EventQueue(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::EventQueue >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueue::GetDefaultForUi()
{
  Org::Webrtc::IEventQueue result {nullptr};
  result = ::Internal::Helper::ToCppWinrt_Org_Webrtc_EventQueue(wrapper::org::webRtc::EventQueue::getDefaultForUi());
  return result;
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueue::Singleton()
{
  return ::Internal::Helper::ToCppWinrt_Org_Webrtc_EventQueue(wrapper::org::webRtc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void Org::Webrtc::implementation::EventQueue::Singleton(Org::Webrtc::IEventQueue const & value)
{
  wrapper::org::webRtc::EventQueue::set_singleton(::Internal::Helper::FromCppWinrt_Org_Webrtc_EventQueue(value));
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
