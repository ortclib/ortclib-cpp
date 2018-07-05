
#include "impl_org_webRtc_RTCRtpCapabilities.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRtpCodecCapability.h"
#include "impl_org_webRtc_RTCRtpHeaderExtensionCapability.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpparameters.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCapabilities::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecCapability, UseRtpCodecCapability);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability, UseRtpHeaderExtensionCapability);


//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCapabilities::RTCRtpCapabilities() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCapabilitiesPtr wrapper::org::webRtc::RTCRtpCapabilities::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpCapabilities>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCapabilities::~RTCRtpCapabilities() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpCapabilities::wrapper_init_org_webRtc_RTCRtpCapabilities() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.codecs = make_shared< decltype(to.codecs)::element_type >();
  to.headerExtensions = make_shared< decltype(to.headerExtensions)::element_type >();
  to.fecMechanisms = make_shared< decltype(to.fecMechanisms)::element_type >();

  for (auto iter = from.codecs.begin(); iter != from.codecs.end(); ++iter) {
    auto wrapper = UseRtpCodecCapability::toWrapper(*iter);
    if (!wrapper) continue;
    to.codecs->push_back(wrapper);
  }

  for (auto iter = from.header_extensions.begin(); iter != from.header_extensions.end(); ++iter) {
    auto wrapper = UseRtpHeaderExtensionCapability::toWrapper(*iter);
    if (!wrapper) continue;
    to.headerExtensions->push_back(wrapper);
  }

  for (auto iter = from.fec.begin(); iter != from.fec.end(); ++iter) {
    to.fecMechanisms->push_back(UseEnum::toString(*iter));
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  if (from.codecs) {
    for (auto iter = from.codecs->begin(); iter != from.codecs->end(); ++iter) {
      auto native = UseRtpCodecCapability::toNative(*iter);
      if (!native) continue;
      to.codecs.push_back(*native);
    }
  }

  if (from.headerExtensions) {
    for (auto iter = from.headerExtensions->begin(); iter != from.headerExtensions->end(); ++iter) {
      auto native = UseRtpHeaderExtensionCapability::toNative(*iter);
      if (!native) continue;
      to.header_extensions.push_back(*native);
    }
  }

  if (from.fecMechanisms) {
    for (auto iter = from.fecMechanisms->begin(); iter != from.fecMechanisms->end(); ++iter) {
      try {
        to.fec.push_back(UseEnum::toNativeFecMechanism(*iter));
      } catch (::zsLib::Exceptions::InvalidArgument &) {
      }
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

