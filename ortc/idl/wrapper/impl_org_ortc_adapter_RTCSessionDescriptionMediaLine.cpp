
#include "impl_org_ortc_adapter_RTCSessionDescriptionMediaLine.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::RTCSessionDescriptionMediaLine()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLinePtr wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLine::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::~RTCSessionDescriptionMediaLine()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::toJson()
{
  {
    auto obj = std::dynamic_pointer_cast<::ortc::adapter::ISessionDescriptionTypes::RTPMediaLine>(native_);
    if (obj) return Json::toWrapper(obj->createElement());
  }
  {
    auto obj = std::dynamic_pointer_cast<::ortc::adapter::ISessionDescriptionTypes::SCTPMediaLine>(native_);
    if (obj) return Json::toWrapper(obj->createElement());
  }

  return JsonPtr();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLine::hash()
{
  {
    auto obj = std::dynamic_pointer_cast<::ortc::adapter::ISessionDescriptionTypes::RTPMediaLine>(native_);
    if (obj) return obj->hash();
  }
  {
    auto obj = std::dynamic_pointer_cast<::ortc::adapter::ISessionDescriptionTypes::SCTPMediaLine>(native_);
    if (obj) return obj->hash();
  }

  return String();
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

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  {
    auto obj = std::dynamic_pointer_cast<RTCSessionDescriptionRtpMediaLine>(wrapper);
    if (obj) return RTCSessionDescriptionRtpMediaLine::toNative(obj);
  }
  {
    auto obj = std::dynamic_pointer_cast<RTCSessionDescriptionSctpMediaLine>(wrapper);
    if (obj) return RTCSessionDescriptionSctpMediaLine::toNative(obj);
  }

  auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!impl) return NativeTypePtr();

  return impl->native_;
}
