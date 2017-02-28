
#include "impl_org_ortc_RTCSctpTransportStats.h"
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
wrapper::impl::org::ortc::RTCSctpTransportStats::RTCSctpTransportStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpTransportStatsPtr wrapper::org::ortc::RTCSctpTransportStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSctpTransportStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportStats::~RTCSctpTransportStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCSctpTransportStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCSctpTransportStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSctpTransportStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCSctpTransportStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCSctpTransportStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSctpTransportStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSctpTransportStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransportStats::wrapper_init_org_ortc_RTCSctpTransportStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransportStats::wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::RTCSctpTransportStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCSctpTransportStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpTransportStats::wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCSctpTransportStats::get_dataChannelsOpened()
{
  return native_->mDataChannelsOpened;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCSctpTransportStats::get_dataChannelsClosed()
{
  return native_->mDataChannelsClosed;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportStatsPtr wrapper::impl::org::ortc::RTCSctpTransportStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCSctpTransportStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSctpTransportStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpTransportStats::NativeStatsPtr wrapper::impl::org::ortc::RTCSctpTransportStats::toNative(wrapper::org::ortc::RTCSctpTransportStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCSctpTransportStats>(wrapper)->native_;
}

