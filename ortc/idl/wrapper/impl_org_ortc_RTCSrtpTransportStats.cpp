
#include "impl_org_ortc_RTCSrtpTransportStats.h"
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
wrapper::impl::org::ortc::RTCSrtpTransportStats::RTCSrtpTransportStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpTransportStatsPtr wrapper::org::ortc::RTCSrtpTransportStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpTransportStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpTransportStats::~RTCSrtpTransportStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCSrtpTransportStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCSrtpTransportStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSrtpTransportStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCSrtpTransportStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCSrtpTransportStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSrtpTransportStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSrtpTransportStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpTransportStats::wrapper_init_org_ortc_RTCSrtpTransportStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpTransportStats::wrapper_init_org_ortc_RTCSrtpTransportStats(wrapper::org::ortc::RTCSrtpTransportStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCSrtpTransportStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpTransportStats::wrapper_init_org_ortc_RTCSrtpTransportStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpTransportStatsPtr wrapper::impl::org::ortc::RTCSrtpTransportStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCSrtpTransportStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpTransportStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpTransportStats::NativeStatsPtr wrapper::impl::org::ortc::RTCSrtpTransportStats::toNative(wrapper::org::ortc::RTCSrtpTransportStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCSrtpTransportStats>(wrapper)->native_;
}

