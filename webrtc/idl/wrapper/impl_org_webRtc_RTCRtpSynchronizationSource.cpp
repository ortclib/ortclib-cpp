
#include "impl_org_webRtc_RTCRtpSynchronizationSource.h"
#include "impl_org_webRtc_RTCRtpContributingSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpreceiverinterface.h"
#include "impl_org_webRtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSynchronizationSource::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpContributingSource, UseRtpContributingSource);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpSynchronizationSource::RTCRtpSynchronizationSource() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSynchronizationSourcePtr wrapper::org::webRtc::RTCRtpSynchronizationSource::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpSynchronizationSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpSynchronizationSource::~RTCRtpSynchronizationSource() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  if (::webrtc::RtpSourceType::SSRC != native.source_type()) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;

  UseRtpContributingSource::apply(native, *result);

#pragma ZS_BUILD_NOTE("LATER","WebRTC does not appear to have a voice activity flag indicator")

  return result;
}
