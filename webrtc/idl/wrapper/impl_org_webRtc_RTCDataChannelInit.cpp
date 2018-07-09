
#include "impl_org_webRtc_RTCDataChannelInit.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/datachannelinterface.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelInit::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDataChannelInit::RTCDataChannelInit() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelInitPtr wrapper::org::webRtc::RTCDataChannelInit::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCDataChannelInit>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDataChannelInit::~RTCDataChannelInit() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannelInit::wrapper_init_org_webRtc_RTCDataChannelInit() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannelInit::wrapper_init_org_webRtc_RTCDataChannelInit(wrapper::org::webRtc::RTCDataChannelInitPtr source) noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.ordered = from.ordered;
  if (from.maxRetransmitTime >= 0) {
    to.maxPacketLifetime = ::zsLib::Milliseconds(SafeInt<::zsLib::Milliseconds::rep>(from.maxRetransmitTime));
  }
  if (from.maxRetransmits >= 0) {
    to.maxRetransmits = SafeInt<decltype(to.maxRetransmits)::value_type>(from.maxRetransmits);
  }
  to.protocol = from.protocol;
  to.negotiated = from.negotiated;
  if (from.id >= 0) {
    to.id = SafeInt<decltype(to.id)::value_type>(from.id);
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  to.ordered = from.ordered;
  if (from.maxPacketLifetime.has_value()) {
    to.maxRetransmitTime = (int)SafeInt<int>(from.maxPacketLifetime.value().count());
  }
  if (from.maxRetransmits.has_value()) {
    to.maxRetransmits = (int)SafeInt<int>(from.maxRetransmits.value());
  }
  to.protocol = from.protocol;
  to.negotiated = from.negotiated;
  if (from.id >= 0) {
    to.id = (int)SafeInt<int>(from.id.value());
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

