
#include "impl_org_ortc_RTCRtpHeaderExtension.h"
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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpHeaderExtension::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpHeaderExtension::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpHeaderExtension::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpHeaderExtension::RTCRtpHeaderExtension()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpHeaderExtensionPtr wrapper::org::ortc::RTCRtpHeaderExtension::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpHeaderExtension>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpHeaderExtension::~RTCRtpHeaderExtension()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpHeaderExtension::wrapper_init_org_ortc_RTCRtpHeaderExtension()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpHeaderExtension::wrapper_init_org_ortc_RTCRtpHeaderExtension(wrapper::org::ortc::RTCRtpHeaderExtensionPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpHeaderExtension::wrapper_init_org_ortc_RTCRtpHeaderExtension(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpHeaderExtension::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpHeaderExtension::hash()
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
  pThis->kind = native.mKind;
  pThis->uri = native.mURI;
  pThis->preferredId = SafeInt<decltype(pThis->preferredId)>(native.mPreferredID);
  pThis->preferredEncrypt = native.mPreferredEncrypt;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mKind = wrapper->kind;
  result->mURI = wrapper->uri;
  result->mPreferredID = SafeInt<decltype(result->mPreferredID)>(wrapper->preferredId);
  result->mPreferredEncrypt = wrapper->preferredEncrypt;
  return result;
}
