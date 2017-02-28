
#include "impl_org_ortc_RTCIceCandidatePairChangeEvent.h"
#include "impl_org_ortc_RTCIceCandidatePair.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidatePairChangeEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidatePairChangeEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairChangeEvent::RTCIceCandidatePairChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairChangeEventPtr wrapper::org::ortc::RTCIceCandidatePairChangeEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidatePairChangeEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairChangeEvent::~RTCIceCandidatePairChangeEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairPtr wrapper::impl::org::ortc::RTCIceCandidatePairChangeEvent::get_candidatePair()
{
  return RTCIceCandidatePair::toWrapper(candidatePair_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(::ortc::IICETransportTypes::CandidatePairPtr candidatePair)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->candidatePair_ = candidatePair;
  return pThis;
}
