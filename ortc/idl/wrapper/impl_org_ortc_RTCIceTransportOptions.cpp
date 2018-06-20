
#include "impl_org_ortc_RTCIceTransportOptions.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportOptions::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportOptions::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportOptions::RTCIceTransportOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportOptionsPtr wrapper::org::ortc::RTCIceTransportOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceTransportOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportOptions::~RTCIceTransportOptions() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportOptions::wrapper_init_org_ortc_RTCIceTransportOptions() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportOptions::wrapper_init_org_ortc_RTCIceTransportOptions(wrapper::org::ortc::RTCIceTransportOptionsPtr source) noexcept
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportOptions::wrapper_init_org_ortc_RTCIceTransportOptions(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceTransportOptions::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCIceTransportOptions"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceTransportOptions::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->aggressiveIce = native.mAggressiveICE;
  pThis->role = Helper::toWrapper(native.mRole);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mAggressiveICE = wrapper->aggressiveIce;
  result->mRole = Helper::toNative(wrapper->role);
  return result;
}
