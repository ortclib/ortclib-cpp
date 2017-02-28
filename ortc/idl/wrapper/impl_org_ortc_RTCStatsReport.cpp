
#include "impl_org_ortc_RTCStatsReport.h"
#include "impl_org_ortc_RTCStats.h"

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
wrapper::impl::org::ortc::RTCStatsReport::RTCStatsReport()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCStatsReportPtr wrapper::org::ortc::RTCStatsReport::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStatsReport>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsReport::~RTCStatsReport()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCStatsPtr wrapper::impl::org::ortc::RTCStatsReport::getStats(String id)
{
  return RTCStats::toWrapper(native_->getStats(id));
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCStatsReport::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
shared_ptr< list< String > > wrapper::impl::org::ortc::RTCStatsReport::get_statsIds()
{
  return native_->getStatesIDs();
}

wrapper::impl::org::ortc::RTCStatsReportPtr wrapper::impl::org::ortc::RTCStatsReport::toWrapper(::ortc::IStatsReportPtr native)
{
  if (!native) return RTCStatsReportPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStatsReport>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}
