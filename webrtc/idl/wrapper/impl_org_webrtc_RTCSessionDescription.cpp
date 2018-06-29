
#include "impl_org_webrtc_RTCSessionDescription.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/jsep.h"
#include "impl_org_webrtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCSessionDescription::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCSessionDescription::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCSessionDescription::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCSessionDescription::RTCSessionDescription() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::org::webrtc::RTCSessionDescription::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCSessionDescription>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCSessionDescription::~RTCSessionDescription() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCSessionDescription::wrapper_init_org_webrtc_RTCSessionDescription(wrapper::org::webrtc::RTCSessionDescriptionInitPtr init) noexcept(false)
{
  ZS_ASSERT(init);
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSdpType wrapper::impl::org::webrtc::RTCSessionDescription::get_sdpType() noexcept
{
  wrapper::org::webrtc::RTCSdpType result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::RTCSessionDescription::get_sdp() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  //apply(native, *result);
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
  //auto cloned = make_shared<NativeType>();
  //apply(*converted, *cloned);
  //return cloned;
#pragma ZS_BUILD_NOTE("TODO","(robin)")
  return NativeTypePtr();
}
