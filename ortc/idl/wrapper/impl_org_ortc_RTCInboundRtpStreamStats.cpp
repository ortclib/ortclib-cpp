
#include "impl_org_ortc_RTCInboundRtpStreamStats.h"
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
wrapper::impl::org::ortc::RTCInboundRtpStreamStats::RTCInboundRtpStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCInboundRtpStreamStatsPtr wrapper::org::ortc::RTCInboundRtpStreamStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCInboundRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCInboundRtpStreamStats::~RTCInboundRtpStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCInboundRtpStreamStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCInboundRtpStreamStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
Optional< uint32_t > wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_ssrc()
{
  return native_->mSSRC;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_associatedStatId()
{
  return native_->mAssociatedStatID;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_isRemote()
{
  return native_->mIsRemote;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_mediaType()
{
  return native_->mMediaType;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_mediaTrackId()
{
  return native_->mMediaTrackID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_transportId()
{
  return native_->mTransportID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_codecId()
{
  return native_->mCodecID;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_firCount()
{
  return native_->mFIRCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_pliCount()
{
  return native_->mPLICount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_nackCount()
{
  return native_->mNACKCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_sliCount()
{
  return native_->mSLICount;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCInboundRtpStreamStats::wrapper_init_org_ortc_RTCInboundRtpStreamStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCInboundRtpStreamStats::wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCInboundRtpStreamStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCInboundRtpStreamStats::wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_packetsReceived()
{
  return native_->mPacketsReceived;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_bytesReceived()
{
  return native_->mBytesReceived;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_packetsLost()
{
  return native_->mPacketsLost;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_jitter()
{
  return native_->mJitter;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_fractionLost()
{
  return native_->mFractionLost;
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::impl::org::ortc::RTCInboundRtpStreamStats::get_endToEndDelay()
{
  return native_->mEndToEndDelay;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCInboundRtpStreamStatsPtr wrapper::impl::org::ortc::RTCInboundRtpStreamStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCInboundRtpStreamStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCInboundRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCInboundRtpStreamStats::NativeStatsPtr wrapper::impl::org::ortc::RTCInboundRtpStreamStats::toNative(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCInboundRtpStreamStats>(wrapper)->native_;
}

