
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
wrapper::impl::org::ortc::RTCMediaStreamStats::RTCMediaStreamStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper::org::ortc::RTCMediaStreamStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMediaStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamStats::~RTCMediaStreamStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCMediaStreamStats::toJson() noexcept
{
  return Json::toWrapper(native_->createElement("RTCMediaStreamStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::hash() noexcept
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCMediaStreamStats::get_timestamp() noexcept
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCMediaStreamStats::get_statsType() noexcept
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::get_statsTypeOther() noexcept
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::get_id() noexcept
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamStats::wrapper_init_org_ortc_RTCMediaStreamStats() noexcept
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamStats::wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::RTCMediaStreamStatsPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_ortc_RTCMediaStreamStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamStats::wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::JsonPtr json) noexcept
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamStats::get_streamId() noexcept
{
  return native_->mStreamID;
}

//------------------------------------------------------------------------------
shared_ptr< list< String > > wrapper::impl::org::ortc::RTCMediaStreamStats::get_trackIds() noexcept
{
  return make_shared< list<String> >(native_->mTrackIDs);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamStatsPtr wrapper::impl::org::ortc::RTCMediaStreamStats::toWrapper(NativeStatsPtr native) noexcept
{
  if (!native) return RTCMediaStreamStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMediaStreamStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamStats::NativeStatsPtr wrapper::impl::org::ortc::RTCMediaStreamStats::toNative(wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCMediaStreamStats>(wrapper)->native_;
}
