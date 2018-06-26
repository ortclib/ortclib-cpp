
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include "Org.Webrtc.EventQueueMaker.h"
#include "Org.Webrtc.EventQueue.h"

#include <wrapper/impl_org_webrtc_EventQueue.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtImpl(wrapper::org::webrtc::EventQueueMakerPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Webrtc::implementation::EventQueueMaker>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtImpl(Org::Webrtc::EventQueueMaker const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::EventQueueMaker>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtImpl(Org::Webrtc::IEventQueueMaker const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::EventQueueMaker>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrt(wrapper::org::webrtc::EventQueueMakerPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrt(Org::Webrtc::EventQueueMaker const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value)
{
  return value.as< Org::Webrtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrt(Org::Webrtc::IEventQueueMaker const & value)
{
  return value.as< Org::Webrtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtInterface(wrapper::org::webrtc::EventQueueMakerPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::IEventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtInterface(Org::Webrtc::EventQueueMaker const & value)
{
  return value.as< Org::Webrtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value)
{
  return value.as< Org::Webrtc::EventQueueMaker >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueueMaker Org::Webrtc::implementation::EventQueueMaker::ToCppWinrtInterface(Org::Webrtc::IEventQueueMaker const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::EventQueueMakerPtr Org::Webrtc::implementation::EventQueueMaker::FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value)
{
  if (!value) return wrapper::org::webrtc::EventQueueMakerPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::EventQueueMakerPtr Org::Webrtc::implementation::EventQueueMaker::FromCppWinrt(Org::Webrtc::EventQueueMaker const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::EventQueueMakerPtr Org::Webrtc::implementation::EventQueueMaker::FromCppWinrt(wrapper::org::webrtc::EventQueueMakerPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::EventQueueMakerPtr Org::Webrtc::implementation::EventQueueMaker::FromCppWinrt(Org::Webrtc::IEventQueueMaker const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Webrtc::EventQueueMaker Org::Webrtc::implementation::EventQueueMaker::CastFromIEventQueueMaker(Org::Webrtc::IEventQueueMaker const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Webrtc_EventQueueMaker(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webrtc::EventQueueMaker >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Org::Webrtc::IEventQueue Org::Webrtc::implementation::EventQueueMaker::BindQueue(Windows::UI::Core::CoreDispatcher const & queue)
{
  Org::Webrtc::IEventQueue result {nullptr};
  result = Org::Webrtc::implementation::EventQueue::ToCppWinrtInterface(wrapper::impl::org::webrtc::EventQueue::toWrapper(queue));
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Core::CoreDispatcher Org::Webrtc::implementation::EventQueueMaker::ExtractQueue(Org::Webrtc::IEventQueue const & queue)
{
  Windows::UI::Core::CoreDispatcher result {nullptr};
  result = wrapper::impl::org::webrtc::EventQueue::toNative_winrt(Org::Webrtc::implementation::EventQueue::FromCppWinrt(queue));
  return result;
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
