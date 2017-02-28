
#include "impl_org_ortc_RTCIceGathererIceErrorEvent.h"
#include "impl_org_ortc_RTCIceCandidate.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::RTCIceGathererIceErrorEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererIceErrorEventPtr wrapper::org::ortc::RTCIceGathererIceErrorEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::~RTCIceGathererIceErrorEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePtr wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::get_hostCandidate()
{
  return RTCIceCandidate::toWrapper(native_->mHostCandidate);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::get_url()
{
  return native_->mURL;
}

//------------------------------------------------------------------------------
uint16_t wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::get_errorCode()
{
  return native_->mErrorCode;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererIceErrorEvent::get_errorText()
{
  return native_->mErrorText;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}
