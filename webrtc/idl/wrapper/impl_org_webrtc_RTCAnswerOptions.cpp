
#include "impl_org_webrtc_RTCAnswerOptions.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCAnswerOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCAnswerOptions::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCAnswerOptions::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCAnswerOptions::RTCAnswerOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCAnswerOptionsPtr wrapper::org::webrtc::RTCAnswerOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCAnswerOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCAnswerOptions::~RTCAnswerOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCAnswerOptions::wrapper_init_org_webrtc_RTCAnswerOptions() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCAnswerOptions::wrapper_init_org_webrtc_RTCAnswerOptions(wrapper::org::webrtc::RTCAnswerOptionsPtr source) noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.voiceActivityDetection = from.voice_activity_detection;
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  to.voice_activity_detection = from.voiceActivityDetection;
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
