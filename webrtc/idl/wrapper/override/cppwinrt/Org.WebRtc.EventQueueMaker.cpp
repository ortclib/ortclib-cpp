
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include "Org.WebRtc.EventQueueMaker.h"
#include "Org.WebRtc.EventQueue.h"

#include <wrapper/impl_org_webRtc_EventQueue.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtImpl(wrapper::org::webRtc::EventQueueMakerPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::WebRtc::implementation::EventQueueMaker>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtImpl(Org::WebRtc::EventQueueMaker const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::EventQueueMaker>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtImpl(Org::WebRtc::IEventQueueMaker const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::EventQueueMaker>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrt(wrapper::org::webRtc::EventQueueMakerPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::EventQueueMaker {nullptr};
  return result.as< Org::WebRtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrt(Org::WebRtc::EventQueueMaker const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value)
{
  if (!value) return Org::WebRtc::EventQueueMaker {nullptr};
  return value.as< Org::WebRtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrt(Org::WebRtc::IEventQueueMaker const & value)
{
  if (!value) return Org::WebRtc::EventQueueMaker {nullptr};
  return value.as< Org::WebRtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtInterface(wrapper::org::webRtc::EventQueueMakerPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::IEventQueueMaker {nullptr};
  return result.as< Org::WebRtc::IEventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtInterface(Org::WebRtc::EventQueueMaker const & value)
{
  if (!value) return Org::WebRtc::IEventQueueMaker {nullptr};
  return value.as< Org::WebRtc::IEventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value)
{
  if (!value) return Org::WebRtc::IEventQueueMaker {nullptr};
  return value.as< Org::WebRtc::IEventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueueMaker Org::WebRtc::implementation::EventQueueMaker::ToCppWinrtInterface(Org::WebRtc::IEventQueueMaker const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueueMakerPtr Org::WebRtc::implementation::EventQueueMaker::FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value)
{
  if (!value) return wrapper::org::webRtc::EventQueueMakerPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueueMakerPtr Org::WebRtc::implementation::EventQueueMaker::FromCppWinrt(Org::WebRtc::EventQueueMaker const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueueMakerPtr Org::WebRtc::implementation::EventQueueMaker::FromCppWinrt(wrapper::org::webRtc::EventQueueMakerPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueueMakerPtr Org::WebRtc::implementation::EventQueueMaker::FromCppWinrt(Org::WebRtc::IEventQueueMaker const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueueMaker Org::WebRtc::implementation::EventQueueMaker::CastFromIEventQueueMaker(Org::WebRtc::IEventQueueMaker const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_WebRtc_EventQueueMaker(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::EventQueueMaker >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Org::WebRtc::IEventQueue Org::WebRtc::implementation::EventQueueMaker::Bind(Windows::UI::Core::CoreDispatcher const & queue)
{
  Org::WebRtc::IEventQueue result {nullptr};
  result = Org::WebRtc::implementation::EventQueue::ToCppWinrtInterface(wrapper::impl::org::webRtc::EventQueue::toWrapper(queue));
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Core::CoreDispatcher Org::WebRtc::implementation::EventQueueMaker::Extract(Org::WebRtc::IEventQueue const & queue)
{
  Windows::UI::Core::CoreDispatcher result {nullptr};
  result = wrapper::impl::org::webRtc::EventQueue::toNative_winrt(Org::WebRtc::implementation::EventQueue::FromCppWinrt(queue));
  return result;
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
