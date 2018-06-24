
#include "impl_org_ortc_RTCDtmfToneChangeEvent.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfToneChangeEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfToneChangeEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfToneChangeEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtmfToneChangeEvent::RTCDtmfToneChangeEvent() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtmfToneChangeEventPtr wrapper::org::ortc::RTCDtmfToneChangeEvent::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDtmfToneChangeEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtmfToneChangeEvent::~RTCDtmfToneChangeEvent() noexcept
{
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtmfToneChangeEvent::get_tone() noexcept
{
  return native_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}


