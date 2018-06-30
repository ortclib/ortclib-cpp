
#include "impl_org_ortc_adapter_RTCSessionDescription.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionDescription.h"
#include "impl_org_ortc_RTCRtpCapabilities.h"
#include "impl_org_ortc_RTCRtpParameters.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"

#include <ortc/adapter/IHelper.h>

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

namespace wrapper { namespace impl { namespace org { namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(org_ortc_wrapper_adapter); } } } } }

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescription::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescription::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescription::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescription::RTCSessionDescription() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::org::ortc::adapter::RTCSessionDescription::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescription>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescription::~RTCSessionDescription() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescription::wrapper_init_org_ortc_adapter_RTCSessionDescription(
  wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type,
  String description
  ) noexcept(false)
{
  native_ = NativeType::create(Helper::toNative(type), description);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescription::wrapper_init_org_ortc_adapter_RTCSessionDescription(
  wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type,
  wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr description
  ) noexcept(false)
{
  auto wrapperDescription = RTCSessionDescriptionDescription::toNative(description);
  ZS_THROW_INVALID_ARGUMENT_IF(!wrapperDescription);
  native_ = NativeType::create(Helper::toNative(type), *wrapperDescription);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpParametersPtr wrapper::org::ortc::adapter::RTCSessionDescription::convertCapabilitiesToParameters(wrapper::org::ortc::RTCRtpCapabilitiesPtr capabilitites) noexcept
{
  auto native = wrapper::impl::org::ortc::RTCRtpCapabilities::toNative(capabilitites);
  if (!native) return RTCRtpParametersPtr();

  auto result = ::ortc::adapter::IHelper::capabilitiesToParameters(*native);
  return wrapper::impl::org::ortc::RTCRtpParameters::toWrapper(result);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCapabilitiesPtr wrapper::org::ortc::adapter::RTCSessionDescription::ConvertParametersToCapabilitites(wrapper::org::ortc::RTCRtpParametersPtr parameters) noexcept
{
  auto native = wrapper::impl::org::ortc::RTCRtpParameters::toNative(parameters);
  if (!native) return RTCRtpCapabilitiesPtr();

  auto result = ::ortc::adapter::IHelper::parametersToCapabilities(*native);
  return wrapper::impl::org::ortc::RTCRtpCapabilities::toWrapper(result);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_objectId() noexcept
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_type() noexcept
{
  return Helper::toWrapper(native_->type());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSdpType wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_sdpType() noexcept
{
  auto type = native_->type();
  ZS_ASSERT_MESSAGE(NativeType::SignalingType_JSON != type, "JSON based signaling is not supported in the adapter");

  switch (type)
  {
    case NativeType::SignalingType_JSON:        break;
    case NativeType::SignalingType_SDPOffer:    return wrapper::org::ortc::adapter::RTCSdpType_offer;
    case NativeType::SignalingType_SDPPranswer: return wrapper::org::ortc::adapter::RTCSdpType_pranswer;
    case NativeType::SignalingType_SDPAnswer:   return wrapper::org::ortc::adapter::RTCSdpType_answer;
    case NativeType::SignalingType_SDPRollback: return wrapper::org::ortc::adapter::RTCSdpType_rollback;
  }

  wrapper::org::ortc::adapter::RTCSdpType result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_isJsonSignalling() noexcept
{
  return NativeType::SignalingType_JSON == native_->type();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_isSdpSignaling() noexcept
{
  return NativeType::SignalingType_JSON != native_->type();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_description() noexcept
{
  return RTCSessionDescriptionDescription::toWrapper(native_->description());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_formattedDescription() noexcept
{
  return native_->formattedDescription();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescription::get_sdp() noexcept
{
  return native_->formattedDescription();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!impl) return NativeTypePtr();

  return impl->native_;
}
