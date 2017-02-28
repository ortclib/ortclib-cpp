
#include "impl_org_ortc_RTCRtpHeaderExtensionParameters.h"
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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::RTCRtpHeaderExtensionParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr wrapper::org::ortc::RTCRtpHeaderExtensionParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::~RTCRtpHeaderExtensionParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters(wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpHeaderExtensionParameters::hash()
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
  pThis->uri = native.mURI;
  pThis->id = SafeInt<decltype(pThis->id)>(native.mID);
  pThis->encrypt = native.mEncrypt;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mURI = wrapper->uri;
  result->mID = SafeInt<decltype(result->mID)>(wrapper->id);
  result->mEncrypt = wrapper->encrypt;
  return result;
}
