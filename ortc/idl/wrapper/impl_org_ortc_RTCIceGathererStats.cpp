
#include "impl_org_ortc_RTCIceGathererStats.h"
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
wrapper::impl::org::ortc::RTCIceGathererStats::RTCIceGathererStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererStatsPtr wrapper::org::ortc::RTCIceGathererStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGathererStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererStats::~RTCIceGathererStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceGathererStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCIceGathererStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCIceGathererStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCIceGathererStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGathererStats::wrapper_init_org_ortc_RTCIceGathererStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGathererStats::wrapper_init_org_ortc_RTCIceGathererStats(wrapper::org::ortc::RTCIceGathererStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCIceGathererStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGathererStats::wrapper_init_org_ortc_RTCIceGathererStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceGathererStats::get_bytesSent()
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceGathererStats::get_bytesReceived()
{
  return native_->mBytesReceived;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGathererStats::get_rtcpGathererStatsId()
{
  return native_->mRTCPGathererStatsID;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererStatsPtr wrapper::impl::org::ortc::RTCIceGathererStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCIceGathererStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGathererStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGathererStats::NativeStatsPtr wrapper::impl::org::ortc::RTCIceGathererStats::toNative(wrapper::org::ortc::RTCIceGathererStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCIceGathererStats>(wrapper)->native_;
}

