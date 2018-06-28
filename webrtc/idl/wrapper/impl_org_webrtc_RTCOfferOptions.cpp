
#include "impl_org_webrtc_RTCOfferOptions.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCOfferOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCOfferOptions::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCOfferOptions::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCOfferOptions::RTCOfferOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCOfferOptionsPtr wrapper::org::webrtc::RTCOfferOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCOfferOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCOfferOptions::~RTCOfferOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::wrapper_init_org_webrtc_RTCOfferOptions() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::wrapper_init_org_webrtc_RTCOfferOptions(wrapper::org::webrtc::RTCOfferOptionsPtr source) noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.voiceActivityDetection = from.voice_activity_detection;
  to.iceRestart = from.ice_restart;
  if (NativeType::kUndefined != from.offer_to_receive_video) {
    to.offerToReceiveVideo = (0 == from.offer_to_receive_video ? false : true);
  }
  if (NativeType::kUndefined != from.offer_to_receive_audio) {
    to.offerToReceiveAudio = (0 == from.offer_to_receive_audio ? false : true);
  }
  to.useRtpMux = from.use_rtp_mux;
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  to.voice_activity_detection = from.voiceActivityDetection;
  to.ice_restart = from.iceRestart;
  if (from.offerToReceiveVideo.has_value()) {
    to.offer_to_receive_video = (from.offerToReceiveVideo.value() ? 1 : 0);
  }
  if (from.offerToReceiveAudio.hasValue()) {
    to.offer_to_receive_audio = (from.offerToReceiveAudio.value() ? 1 : 0);
  }
  to.use_rtp_mux = from.useRtpMux;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  apply(native, *result);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypePtr();
  auto cloned = make_shared<NativeType>();
  apply(*converted, *cloned);
  return cloned;
}
