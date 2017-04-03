
#include "impl_org_ortc_RTCIceCandidateAttributes.h"
#include "impl_org_ortc_Helper.h"
#include "impl_org_ortc_Json.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidateAttributes::RTCIceCandidateAttributes()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidateAttributesPtr wrapper::org::ortc::RTCIceCandidateAttributes::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidateAttributes>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidateAttributes::~RTCIceCandidateAttributes()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceCandidateAttributes::toJson()
{
  return Json::toWrapper(native_->createElement("RTCIceCandidateAttributes"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidateAttributes::wrapper_init_org_ortc_RTCIceCandidateAttributes()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidateAttributes::wrapper_init_org_ortc_RTCIceCandidateAttributes(wrapper::org::ortc::RTCIceCandidateAttributesPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCIceCandidateAttributes();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidateAttributes::wrapper_init_org_ortc_RTCIceCandidateAttributes(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_relatedId()
{
  return native_->mRelatedID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_ipAddress()
{
  return native_->mIPAddress;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_portNumber()
{
  return native_->mPortNumber;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_transport()
{
  return native_->mTransport;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidateType wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_candidateType()
{
  return Helper::toWrapper(native_->mCandidateType);
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_priority()
{
  return native_->mPriority;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidateAttributes::get_addressSourceUrl()
{
  return native_->mAddressSourceURL;
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidateAttributesPtr wrapper::impl::org::ortc::RTCIceCandidateAttributes::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCIceCandidateAttributesPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidateAttributes>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidateAttributes::NativeStatsPtr wrapper::impl::org::ortc::RTCIceCandidateAttributes::toNative(wrapper::org::ortc::RTCIceCandidateAttributesPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCIceCandidateAttributes>(wrapper)->native_;
}

