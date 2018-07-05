
#include "impl_org_webRtc_RTCAnswerOptions.h"
#include "impl_org_webRtc_RTCOfferAnswerOptions.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCAnswerOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCOfferAnswerOptions, UseOfferAnswerOptions);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCAnswerOptions::RTCAnswerOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCAnswerOptionsPtr wrapper::org::webRtc::RTCAnswerOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCAnswerOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCAnswerOptions::~RTCAnswerOptions() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCAnswerOptions::wrapper_init_org_webRtc_RTCAnswerOptions() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCAnswerOptions::wrapper_init_org_webRtc_RTCAnswerOptions(wrapper::org::webRtc::RTCAnswerOptionsPtr source) noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  UseOfferAnswerOptions::apply(from, to);
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  UseOfferAnswerOptions::apply(from, to);
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
