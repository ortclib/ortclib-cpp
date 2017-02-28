
#include "impl_org_ortc_OrtcLibWithDispatcher.h"
#include "impl_org_ortc_OrtcLib.h"
#include "impl_org_ortc_Dispatcher.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OrtcLibWithDispatcher::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OrtcLibWithDispatcher::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OrtcLibWithDispatcher::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::OrtcLibWithDispatcher::~OrtcLibWithDispatcher()
{
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLibWithDispatcher::setup(wrapper::org::ortc::DispatcherPtr dispatcher)
{
  typedef zsLib::AnyHolder< Windows::UI::Core::CoreDispatcher^ > AnyDispatchHolder;

  auto any = wrapper::impl::org::ortc::Dispatcher::toNative(dispatcher);
  if (!any) {
    wrapper::org::ortc::OrtcLib::setup();
    return;
  }

  auto holder = std::dynamic_pointer_cast<AnyDispatchHolder>(wrapper::impl::org::ortc::Dispatcher::toNative(dispatcher));
  if (!holder) {
    wrapper::org::ortc::OrtcLib::setup();
    return;
  }

  NativeType::setup(holder->value_);
}


