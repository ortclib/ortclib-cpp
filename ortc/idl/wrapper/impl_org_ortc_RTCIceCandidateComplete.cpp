
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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidateComplete::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidateComplete::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidateComplete::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidateComplete::RTCIceCandidateComplete()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidateCompletePtr wrapper::org::ortc::RTCIceCandidateComplete::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidateComplete>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidateComplete::~RTCIceCandidateComplete()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidateComplete::wrapper_init_org_ortc_RTCIceCandidateComplete()
{
  native_ = make_shared< NativeType >();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceCandidateComplete::toJson()
{
  return Json::toWrapper(native_->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateComplete::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceComponent wrapper::impl::org::ortc::RTCIceCandidateComplete::get_component()
{
  return Helper::toWrapper(native_->mComponent);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidateComplete::get_complete()
{
  return native_->mComplete;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  return std::dynamic_pointer_cast<WrapperImplType>(wrapper)->native_;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!result) return NativeTypePtr();

  return result->native_;
}

