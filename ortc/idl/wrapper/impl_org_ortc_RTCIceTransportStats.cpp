
#include "impl_org_ortc_RTCIceTransportStats.h"
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
wrapper::impl::org::ortc::RTCIceTransportStats::RTCIceTransportStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportStatsPtr wrapper::org::ortc::RTCIceTransportStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceTransportStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportStats::~RTCIceTransportStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceTransportStats::toJson() noexcept
{
  return Json::toWrapper(native_->createElement("RTCIceTransportStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceTransportStats::hash() noexcept
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCIceTransportStats::get_timestamp() noexcept
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCIceTransportStats::get_statsType() noexcept
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceTransportStats::get_statsTypeOther() noexcept
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceTransportStats::get_id() noexcept
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportStats::wrapper_init_org_ortc_RTCIceTransportStats() noexcept
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportStats::wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::RTCIceTransportStatsPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_ortc_RTCIceTransportStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportStats::wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::JsonPtr json) noexcept
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceTransportStats::get_bytesSent() noexcept
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceTransportStats::get_bytesReceived() noexcept
{
  return native_->mBytesReceived;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceTransportStats::get_rtcpTransportStatsId() noexcept
{
  return native_->mRTCPTransportStatsID;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceTransportStats::get_activeConnection() noexcept
{
  return native_->mActiveConnection;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceTransportStats::get_selectedCandidatePairId() noexcept
{
  return native_->mSelectedCandidatePairID;
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportStatsPtr wrapper::impl::org::ortc::RTCIceTransportStats::toWrapper(NativeStatsPtr native) noexcept
{
  if (!native) return RTCIceTransportStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceTransportStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportStats::NativeStatsPtr wrapper::impl::org::ortc::RTCIceTransportStats::toNative(wrapper::org::ortc::RTCIceTransportStatsPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCIceTransportStats>(wrapper)->native_;
}
