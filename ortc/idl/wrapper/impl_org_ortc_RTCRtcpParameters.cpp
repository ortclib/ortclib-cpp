
#include "impl_org_ortc_RTCRtcpParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtcpParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtcpParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtcpParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtcpParameters::RTCRtcpParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtcpParametersPtr wrapper::org::ortc::RTCRtcpParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtcpParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtcpParameters::~RTCRtcpParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtcpParameters::wrapper_init_org_ortc_RTCRtcpParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtcpParameters::wrapper_init_org_ortc_RTCRtcpParameters(wrapper::org::ortc::RTCRtcpParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtcpParameters::wrapper_init_org_ortc_RTCRtcpParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtcpParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtcpParameters::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}
//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->ssrc = SafeInt<decltype(pThis->ssrc)>(native.mSSRC);
  pThis->cName = native.mCName;
  pThis->reducedSize = native.mReducedSize;
  pThis->mux = native.mMux;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mSSRC = SafeInt<decltype(result->mSSRC)>(wrapper->ssrc);
  result->mCName = wrapper->cName;
  result->mReducedSize = wrapper->reducedSize;
  result->mMux = wrapper->mux;
  return result;
}
