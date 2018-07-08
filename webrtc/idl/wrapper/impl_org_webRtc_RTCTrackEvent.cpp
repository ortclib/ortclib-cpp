
#include "impl_org_webRtc_RTCTrackEvent.h"
#include "impl_org_webRtc_RTCRtpReceiver.h"
#include "impl_org_webRtc_MediaStreamTrack.h"
#include "impl_org_webRtc_RTCRtpTransceiver.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCTrackEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCTrackEvent::WrapperType, WrapperType);

ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::UseRtpReceiver, UseRtpReceiver);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::UseMediaStreamTrack, UseMediaStreamTrack);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::UseRtpTransceiver, UseRtpTransceiver);


//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCTrackEvent::RTCTrackEvent() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCTrackEventPtr wrapper::org::webRtc::RTCTrackEvent::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCTrackEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCTrackEvent::~RTCTrackEvent() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiverPtr wrapper::impl::org::webRtc::RTCTrackEvent::get_receiver() noexcept
{
  return receiver_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaStreamTrackPtr wrapper::impl::org::webRtc::RTCTrackEvent::get_track() noexcept
{
  return track_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::impl::org::webRtc::RTCTrackEvent::get_transceiver() noexcept
{
  return transceiver_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseRtpReceiverPtr value) noexcept
{
  if (!value) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->receiver_ = value;
  if (value) {
    result->track_ = value->get_track();
  }
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseMediaStreamTrackPtr value) noexcept
{
  if (!value) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->track_ = value;
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseRtpTransceiverPtr value) noexcept
{
  if (!value) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->transceiver_ = value;
  if (value) {
    result->receiver_ = value->get_receiver();
    if (result->receiver_) {
      result->track_ = result->receiver_->get_track();
    }
  }
  return result;
}
