
#include "impl_org_ortc_RTCIdentity.h"
#include "impl_org_ortc_RTCIdentityAssertion.h"
#include "impl_org_ortc_RTCIdentityResult.h"
#include "impl_org_ortc_RTCIdentityError.h"
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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentity::RTCIdentity()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIdentityPtr wrapper::org::ortc::RTCIdentity::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentity>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentity::~RTCIdentity()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIdentity::wrapper_init_org_ortc_RTCIdentity(wrapper::org::ortc::RTCDtlsTransportPtr transport)
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > wrapper::impl::org::ortc::RTCIdentity::getIdentityAssertion(String provider)
{
  return getIdentityAssertion(provider, String(), String());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > wrapper::impl::org::ortc::RTCIdentity::getIdentityAssertion(
  String provider,
  String protocol
  )
{
  return getIdentityAssertion(provider, protocol, String());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > wrapper::impl::org::ortc::RTCIdentity::getIdentityAssertion(
  String provider,
  String protocol,
  String username
  )
{
  typedef PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > PromiseWithType;

  auto result = PromiseWithType::create(Helper::getGuiQueue());

  if (protocol.isEmpty()) {
    protocol = "default";
  }

  auto promise = native_->getIdentityAssertion(provider, protocol, username.hasData() ? username.c_str() : NULL);

  promise->thenClosure([promise, result] {
    if (promise->isRejected()) {
      typedef AnyHolder< RTCIdentityErrorPtr > AnyHolderType;

      auto nativeReason = promise->reason();
      if (nativeReason) {
        auto reason = make_shared<AnyHolderType>();
        reason->value_ = RTCIdentityError::toWrapper(nativeReason);
        result->reject(reason);
        return;
      }
      Helper::reject(promise, result);
      return;
    }
    result->resolve(RTCIdentityResult::toWrapper(promise->value()));
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityAssertionPtr > > wrapper::impl::org::ortc::RTCIdentity::setIdentityAssertion(String assertion)
{
  ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityAssertionPtr >, PromiseWithType);

  auto promise = native_->setIdentityAssertion(assertion);
  if (!promise) return PromiseWithTypePtr();

  auto result = PromiseWithType::create(Helper::getGuiQueue());

  promise->thenClosure([promise, result] {
    if (promise->isRejected()) {
      Helper::reject(promise, result);
      return;
    }
    result->resolve(RTCIdentityAssertion::toWrapper(promise->value()));
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCIdentity::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIdentityAssertionPtr wrapper::impl::org::ortc::RTCIdentity::get_peerIdentity()
{
  return RTCIdentityAssertion::toWrapper(native_->peerIdentity());
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentityPtr wrapper::impl::org::ortc::RTCIdentity::toWrapper(NativeIdentityPtr native)
{
  if (!native) return RTCIdentityPtr();
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIdentity>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIdentity::NativeIdentityPtr wrapper::impl::org::ortc::RTCIdentity::toNative(wrapper::org::ortc::RTCIdentityPtr wrapper)
{
  if (!wrapper) return NativeIdentityPtr();
  return std::dynamic_pointer_cast<wrapper::impl::org::ortc::RTCIdentity>(wrapper)->native_;
}
