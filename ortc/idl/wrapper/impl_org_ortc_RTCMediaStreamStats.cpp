
#include "impl_org_ortc_RTCMediaStreamStats.h"
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
wrapper::impl::org::ortc::RTCMediaStreamStats::RTCMediaStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper::org::ortc::RTCMediaStreamStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMediaStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamStats::~RTCMediaStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCMediaStreamStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCMediaStreamStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCMediaStreamStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCMediaStreamStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamStats::wrapper_init_org_ortc_RTCMediaStreamStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamStats::wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::RTCMediaStreamStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCMediaStreamStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamStats::wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::get_streamId()
{
  return native_->mStreamID;
}

//------------------------------------------------------------------------------
shared_ptr< list< String > > wrapper::impl::org::ortc::RTCMediaStreamStats::get_trackIds()
{
  return make_shared< list<String> >(native_->mTrackIDs);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamStatsPtr wrapper::impl::org::ortc::RTCMediaStreamStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCMediaStreamStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMediaStreamStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamStats::NativeStatsPtr wrapper::impl::org::ortc::RTCMediaStreamStats::toNative(wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCMediaStreamStats>(wrapper)->native_;
}
