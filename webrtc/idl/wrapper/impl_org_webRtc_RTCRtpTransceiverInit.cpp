
#include "impl_org_webRtc_RTCRtpTransceiverInit.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRtpEncodingParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtptransceiverinterface.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiverInit::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpEncodingParameters, UseRtpEncodingParameters);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpTransceiverInit::RTCRtpTransceiverInit() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverInitPtr wrapper::org::webRtc::RTCRtpTransceiverInit::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpTransceiverInit>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpTransceiverInit::~RTCRtpTransceiverInit() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpTransceiverInit::wrapper_init_org_webRtc_RTCRtpTransceiverInit() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.direction = UseEnum::toWrapper(from.direction);
  to.sendEncodings = make_shared< decltype(to.sendEncodings)::element_type >();

  for (auto iter = from.send_encodings.begin(); iter != from.send_encodings.end(); ++iter) {
    auto wrapper = UseRtpEncodingParameters::toWrapper(*iter);
    if (!wrapper) continue;
    to.sendEncodings->push_back(wrapper);
  }

}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  to.direction = UseEnum::toNative(from.direction);
  to.send_encodings.clear();

  if (from.sendEncodings) {
    for (auto iter = from.sendEncodings->begin(); iter != from.sendEncodings->end(); ++iter) {
      auto native = UseRtpEncodingParameters::toNative(*iter);
      if (!native) continue;
      to.send_encodings.push_back(*native);
    }
  }
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
