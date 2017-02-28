
#include "impl_org_ortc_RTCIceGathererCandidate.h"
#include "impl_org_ortc_RTCIceCandidate.h"
#include "impl_org_ortc_RTCIceCandidateComplete.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererCandidate::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererCandidate::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceGathererCandidate::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererCandidate::RTCIceGathererCandidate()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper::org::ortc::RTCIceGathererCandidate::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGathererCandidate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererCandidate::~RTCIceGathererCandidate()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper::org::ortc::RTCIceGathererCandidate::create(wrapper::org::ortc::JsonPtr json)
{
  auto result = ::ortc::IICETypes::GatherCandidate::create(impl::org::ortc::Json::toNative(json));
  {
    auto native = std::dynamic_pointer_cast<::ortc::IICETypes::Candidate>(result);
    if (native) return wrapper::impl::org::ortc::RTCIceCandidate::toWrapper(native);
  }
  {
    auto native = std::dynamic_pointer_cast<::ortc::IICETypes::CandidateComplete>(result);
    if (native) return wrapper::impl::org::ortc::RTCIceCandidateComplete::toWrapper(native);
  }
  return WrapperImplTypePtr();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceGathererCandidate::toJson()
{
  {
    auto native = std::dynamic_pointer_cast<::ortc::IICETypes::Candidate>(native_);
    if (native) return Json::toWrapper(native->createElement());
  }
  {
    auto native = std::dynamic_pointer_cast<::ortc::IICETypes::CandidateComplete>(native_);
    if (native) return Json::toWrapper(native->createElement());
  }
  return JsonPtr();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererCandidate::hash()
{
  {
    auto native = std::dynamic_pointer_cast<::ortc::IICETypes::Candidate>(native_);
    if (native) return native->hash();
  }
  {
    auto native = std::dynamic_pointer_cast<::ortc::IICETypes::CandidateComplete>(native_);
    if (native) return native->hash();
  }
  return String();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceComponent wrapper::impl::org::ortc::RTCIceGathererCandidate::get_component()
{
  return Helper::toWrapper(native_->mComponent);
}

//------------------------------------------------------------------------------
WrapperTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperTypePtr();

  {
    auto converted = std::dynamic_pointer_cast<::ortc::IICETypes::Candidate>(native);
    if (converted) return RTCIceCandidate::toWrapper(converted);
  }
  {
    auto converted = std::dynamic_pointer_cast<::ortc::IICETypes::CandidateComplete>(native);
    if (converted) return RTCIceCandidateComplete::toWrapper(converted);
  }

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  {
    wrapper::org::ortc::RTCIceCandidatePtr impl = std::dynamic_pointer_cast<wrapper::impl::org::ortc::RTCIceCandidate>(wrapper);
    if (impl) return RTCIceCandidate::toNative(impl);
  }

  {
    wrapper::org::ortc::RTCIceCandidateCompletePtr impl = std::dynamic_pointer_cast<wrapper::impl::org::ortc::RTCIceCandidateComplete>(wrapper);
    if (impl) return RTCIceCandidateComplete::toNative(impl);
  }

  return std::dynamic_pointer_cast<WrapperImplType>(wrapper)->native_;
}
