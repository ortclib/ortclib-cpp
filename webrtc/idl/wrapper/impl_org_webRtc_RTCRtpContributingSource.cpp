
#include "impl_org_webRtc_RTCRtpContributingSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpreceiverinterface.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>

#include <cmath>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpContributingSource::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpContributingSource::RTCRtpContributingSource() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpContributingSourcePtr wrapper::org::webRtc::RTCRtpContributingSource::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpContributingSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpContributingSource::~RTCRtpContributingSource() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void WrapperImplType::apply(const NativeType &from, WrapperType &to)
{

  auto tempLevel = from.audio_level();
  if (tempLevel.has_value()) {
    auto levelValue = tempLevel.value();
    if (127 != levelValue) {
      to.audioLevel = pow(((double)10.0), ((-1.0 *((double)levelValue)) / 20.0));
    }
  }
  to.source = SafeInt<decltype(to.source)>(from.source_id());
  to.timestamp = ::zsLib::Milliseconds(SafeInt<::zsLib::Milliseconds::rep>(from.timestamp_ms()));
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  if (::webrtc::RtpSourceType::CSRC != native.source_type()) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;

  apply(native, *result);

  return result;
}
