
#include "impl_org_webRtc_RTCIceCandidateInit.h"
#include "impl_org_webRtc_RTCIceCandidate.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidateInit::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidateInit::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCIceCandidateInit::RTCIceCandidateInit() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceCandidateInitPtr wrapper::org::webRtc::RTCIceCandidateInit::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCIceCandidateInit>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCIceCandidateInit::~RTCIceCandidateInit() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCIceCandidateInit::wrapper_init_org_webRtc_RTCIceCandidateInit() noexcept
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const UseIceCandidate &candidate) noexcept
{
  auto converted = const_cast<UseIceCandidate *>(&candidate);
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->candidate = converted->get_candidate();
  result->sdpMid = converted->get_sdpMid();
  result->sdpMLineIndex = converted->get_sdpMLineIndex();
  result->usernameFragment = converted->get_usernameFragment();
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseIceCandidatePtr candidate) noexcept
{
  if (!candidate) return WrapperImplTypePtr();
  return toWrapper(*candidate);
}
