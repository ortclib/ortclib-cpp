
#include "impl_org_ortc_RTCSrtpSdesTransportLifetimeRemainingEvent.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::RTCSrtpSdesTransportLifetimeRemainingEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEventPtr wrapper::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::~RTCSrtpSdesTransportLifetimeRemainingEvent()
{
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::get_leastLifetimeRemainingPercentageForAllKeys()
{
  return leastLifetimeRemainingPercentageForAllKeys_;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent::get_overallLifetimeRemainingPercentage()
{
  return overallLifetimeRemainingPercentage_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(
  ULONG leastLifetimeRemainingPercentageForAllKeys,
  ULONG overallLifetimeRemainingPercentage
)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->leastLifetimeRemainingPercentageForAllKeys_ = leastLifetimeRemainingPercentageForAllKeys;
  pThis->overallLifetimeRemainingPercentage_ = overallLifetimeRemainingPercentage;
  return pThis;
}
