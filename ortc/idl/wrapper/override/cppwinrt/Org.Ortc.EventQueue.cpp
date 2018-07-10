
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include "Org.Ortc.EventQueue.h"

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::EventQueue > Org::Ortc::implementation::EventQueue::ToCppWinrtImpl(wrapper::org::ortc::EventQueuePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Ortc::implementation::EventQueue>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::EventQueue > Org::Ortc::implementation::EventQueue::ToCppWinrtImpl(Org::Ortc::EventQueue const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::EventQueue > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::EventQueue>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::EventQueue > Org::Ortc::implementation::EventQueue::ToCppWinrtImpl(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::EventQueue > Org::Ortc::implementation::EventQueue::ToCppWinrtImpl(Org::Ortc::IEventQueue const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::EventQueue > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::EventQueue>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Ortc::EventQueue Org::Ortc::implementation::EventQueue::ToCppWinrt(wrapper::org::ortc::EventQueuePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Ortc::EventQueue Org::Ortc::implementation::EventQueue::ToCppWinrt(Org::Ortc::EventQueue const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Ortc::EventQueue Org::Ortc::implementation::EventQueue::ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value)
{
  return value.as< Org::Ortc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Ortc::EventQueue Org::Ortc::implementation::EventQueue::ToCppWinrt(Org::Ortc::IEventQueue const & value)
{
  return value.as< Org::Ortc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Ortc::IEventQueue Org::Ortc::implementation::EventQueue::ToCppWinrtInterface(wrapper::org::ortc::EventQueuePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::IEventQueue >();
}

//------------------------------------------------------------------------------
Org::Ortc::IEventQueue Org::Ortc::implementation::EventQueue::ToCppWinrtInterface(Org::Ortc::EventQueue const & value)
{
  return value.as< Org::Ortc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Ortc::IEventQueue Org::Ortc::implementation::EventQueue::ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value)
{
  return value.as< Org::Ortc::EventQueue >();
}

//------------------------------------------------------------------------------
Org::Ortc::IEventQueue Org::Ortc::implementation::EventQueue::ToCppWinrtInterface(Org::Ortc::IEventQueue const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr Org::Ortc::implementation::EventQueue::FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value)
{
  if (!value) return wrapper::org::ortc::EventQueuePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr Org::Ortc::implementation::EventQueue::FromCppWinrt(Org::Ortc::EventQueue const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr Org::Ortc::implementation::EventQueue::FromCppWinrt(wrapper::org::ortc::EventQueuePtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr Org::Ortc::implementation::EventQueue::FromCppWinrt(Org::Ortc::IEventQueue const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Ortc::EventQueue Org::Ortc::implementation::EventQueue::Cast(Org::Ortc::IEventQueue const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Ortc_EventQueue(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::ortc::EventQueue >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Org::Ortc::IEventQueue Org::Ortc::implementation::EventQueue::GetDefaultForUi()
{
  Org::Ortc::IEventQueue result {nullptr};
  result = ::Internal::Helper::ToCppWinrt_Org_Ortc_EventQueue(wrapper::org::ortc::EventQueue::getDefaultForUi());
  return result;
}

//------------------------------------------------------------------------------
Org::Ortc::IEventQueue Org::Ortc::implementation::EventQueue::Singleton()
{
  return ::Internal::Helper::ToCppWinrt_Org_Ortc_EventQueue(wrapper::org::ortc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void Org::Ortc::implementation::EventQueue::Singleton(Org::Ortc::IEventQueue const & value)
{
  wrapper::org::ortc::EventQueue::set_singleton(::Internal::Helper::FromCppWinrt_Org_Ortc_EventQueue(value));
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_EVENTQUEUE
