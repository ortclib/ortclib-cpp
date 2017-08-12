
#include "impl_org_ortc_RTCMessageEvent.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCMessageEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCMessageEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCMessageEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMessageEvent::RTCMessageEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCMessageEventPtr wrapper::org::ortc::RTCMessageEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMessageEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMessageEvent::~RTCMessageEvent()
{
}

//------------------------------------------------------------------------------
SecureByteBlockPtr wrapper::impl::org::ortc::RTCMessageEvent::get_binary()
{
  return native_->mBinary;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMessageEvent::get_text()
{
  return native_->mText;
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

