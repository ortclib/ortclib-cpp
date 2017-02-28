
#include "impl_org_ortc_RTCIdentityAssertion.h"

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
wrapper::impl::org::ortc::RTCIdentityAssertion::RTCIdentityAssertion()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIdentityAssertionPtr wrapper::org::ortc::RTCIdentityAssertion::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentityAssertion>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityAssertion::~RTCIdentityAssertion()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIdentityAssertion::wrapper_init_org_ortc_RTCIdentityAssertion()
{
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityAssertionPtr wrapper::impl::org::ortc::RTCIdentityAssertion::toWrapper(NativeAssertionPtr native)
{
  if (!native) return RTCIdentityAssertionPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityAssertionPtr wrapper::impl::org::ortc::RTCIdentityAssertion::toWrapper(const NativeAssertion &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentityAssertion>();
  pThis->thisWeak_ = pThis;

  pThis->idp = native.mIDP;
  pThis->name = native.mName;

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityAssertion::NativeAssertionPtr wrapper::impl::org::ortc::RTCIdentityAssertion::toNative(wrapper::org::ortc::RTCIdentityAssertionPtr wrapper)
{
  if (!wrapper) return NativeAssertionPtr();

  auto result = make_shared<NativeAssertion>();
  result->mIDP = wrapper->idp;
  result->mName = wrapper->name;
  return result;
}
