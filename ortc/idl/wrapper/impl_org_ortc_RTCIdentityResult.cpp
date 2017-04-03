
#include "impl_org_ortc_RTCIdentityResult.h"

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
wrapper::impl::org::ortc::RTCIdentityResult::RTCIdentityResult()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIdentityResultPtr wrapper::org::ortc::RTCIdentityResult::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentityResult>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityResult::~RTCIdentityResult()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIdentityResult::wrapper_init_org_ortc_RTCIdentityResult()
{
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityResultPtr wrapper::impl::org::ortc::RTCIdentityResult::toWrapper(NativeResultPtr native)
{
  if (!native) return RTCIdentityResultPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityResultPtr wrapper::impl::org::ortc::RTCIdentityResult::toWrapper(const NativeResult &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentityResult>();
  pThis->thisWeak_ = pThis;
  pThis->assertion = native.mAssertion;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityResult::NativeResultPtr wrapper::impl::org::ortc::RTCIdentityResult::toNative(wrapper::org::ortc::RTCIdentityResultPtr wrapper)
{
  if (!wrapper) return NativeResultPtr();

  auto result = make_shared<NativeResult>();
  result->mAssertion = wrapper->assertion;
  return result;
}
