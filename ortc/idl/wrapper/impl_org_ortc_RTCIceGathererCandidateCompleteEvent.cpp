
#include "impl_org_ortc_RTCIceGathererCandidateCompleteEvent.h"
#include "impl_org_ortc_RTCIceCandidateComplete.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererCandidateCompleteEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererCandidateCompleteEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererCandidateCompleteEvent::RTCIceGathererCandidateCompleteEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererCandidateCompleteEventPtr wrapper::org::ortc::RTCIceGathererCandidateCompleteEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGathererCandidateCompleteEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererCandidateCompleteEvent::~RTCIceGathererCandidateCompleteEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidateCompletePtr wrapper::impl::org::ortc::RTCIceGathererCandidateCompleteEvent::get_candidate()
{
  wrapper::org::ortc::RTCIceCandidateCompletePtr result {};
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(::ortc::IICEGathererTypes::CandidateCompletePtr candidate)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->candidate_ = RTCIceCandidateComplete::toWrapper(candidate);
  return pThis;
}
