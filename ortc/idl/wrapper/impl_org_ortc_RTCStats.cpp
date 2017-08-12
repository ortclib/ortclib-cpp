
#include "impl_org_ortc_RTCStats.h"
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
wrapper::impl::org::ortc::RTCStats::RTCStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCStatsPtr wrapper::org::ortc::RTCStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStats::~RTCStats()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCStats::wrapper_init_org_ortc_RTCStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCStats::wrapper_init_org_ortc_RTCStats(wrapper::org::ortc::RTCStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCStats();
    return; 
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCStats::wrapper_init_org_ortc_RTCStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsPtr wrapper::impl::org::ortc::RTCStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStats::NativeStatsPtr wrapper::impl::org::ortc::RTCStats::toNative(wrapper::org::ortc::RTCStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCStats>(wrapper)->native_;
}
