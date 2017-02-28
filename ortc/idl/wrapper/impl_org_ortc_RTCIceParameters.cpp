
#include "impl_org_ortc_RTCIceParameters.h"
#include "impl_org_ortc_Json.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceParameters::RTCIceParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceParametersPtr wrapper::org::ortc::RTCIceParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceParameters::~RTCIceParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceParameters::wrapper_init_org_ortc_RTCIceParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceParameters::wrapper_init_org_ortc_RTCIceParameters(wrapper::org::ortc::RTCIceParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceParameters::wrapper_init_org_ortc_RTCIceParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCIceParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceParameters::hash()
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
  pThis->useUnfreezePriority = native.mUseUnfreezePriority;
  pThis->usernameFragment = native.mUsernameFragment;
  pThis->password = native.mPassword;
  pThis->iceLite = native.mICELite;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mUseUnfreezePriority = wrapper->useUnfreezePriority;
  result->mUsernameFragment = wrapper->usernameFragment;
  result->mPassword = wrapper->password;
  result->mICELite = wrapper->iceLite;
  return result;
}
