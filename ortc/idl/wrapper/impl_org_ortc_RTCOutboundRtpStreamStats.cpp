
#include "impl_org_ortc_RTCOutboundRtpStreamStats.h"
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
wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::RTCOutboundRtpStreamStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper::org::ortc::RTCOutboundRtpStreamStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCOutboundRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::~RTCOutboundRtpStreamStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::toJson() noexcept
{
  return Json::toWrapper(native_->createElement("RTCOutboundRtpStreamStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::hash() noexcept
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_timestamp() noexcept
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_statsType() noexcept
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_statsTypeOther() noexcept
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_id() noexcept
{
  return native_->mID;
}

//------------------------------------------------------------------------------
Optional< uint32_t > wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_ssrc() noexcept
{
  return (Optional< uint32_t >)native_->mSSRC;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_associatedStatId() noexcept
{
  return native_->mAssociatedStatID;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_isRemote() noexcept
{
  return native_->mIsRemote;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_mediaType() noexcept
{
  return native_->mMediaType;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_mediaTrackId() noexcept
{
  return native_->mMediaTrackID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_transportId() noexcept
{
  return native_->mTransportID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_codecId() noexcept
{
  return native_->mCodecID;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_firCount() noexcept
{
  return native_->mFIRCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_pliCount() noexcept
{
  return native_->mPLICount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_nackCount() noexcept
{
  return native_->mNACKCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_sliCount() noexcept
{
  return native_->mSLICount;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::wrapper_init_org_ortc_RTCOutboundRtpStreamStats() noexcept
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_ortc_RTCOutboundRtpStreamStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_packetsSent() noexcept
{
  return native_->mPacketsSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_bytesSent() noexcept
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_targetBitrate() noexcept
{
  return native_->mTargetBitrate;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_roundTripTime() noexcept
{
  return native_->mRoundTripTime;
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::toWrapper(NativeStatsPtr native) noexcept
{
  if (!native) return RTCOutboundRtpStreamStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCOutboundRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::NativeStatsPtr wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::toNative(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCOutboundRtpStreamStats>(wrapper)->native_;
}
