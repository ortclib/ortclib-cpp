
#include "impl_org_ortc_RTCIdentityError.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityError::RTCIdentityError()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIdentityErrorPtr wrapper::org::ortc::RTCIdentityError::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentityError>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityError::~RTCIdentityError()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIdentityError::wrapper_init_org_ortc_RTCIdentityError()
{
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityErrorPtr wrapper::impl::org::ortc::RTCIdentityError::toWrapper(NativeErrorPtr native)
{
  if (!native) return RTCIdentityErrorPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityErrorPtr wrapper::impl::org::ortc::RTCIdentityError::toWrapper(const NativeError &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentityError>();
  pThis->thisWeak_ = pThis;
  pThis->idp = native.mIDP;
  pThis->protocol = native.mProtocol;
  pThis->loginUrl = native.mLoginURL;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityError::NativeErrorPtr wrapper::impl::org::ortc::RTCIdentityError::toWrapper(wrapper::org::ortc::RTCIdentityErrorPtr wrapper)
{
  if (!wrapper) return NativeErrorPtr();

  auto result = make_shared<NativeError>();
  result->mIDP = wrapper->idp;
  result->mProtocol = wrapper->protocol;
  result->mLoginURL = wrapper->loginUrl;
  return result;
}
