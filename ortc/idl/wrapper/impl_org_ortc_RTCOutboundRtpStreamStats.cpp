
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
wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::RTCOutboundRtpStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper::org::ortc::RTCOutboundRtpStreamStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCOutboundRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::~RTCOutboundRtpStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCOutboundRtpStreamStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
Optional< uint32_t > wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_ssrc()
{
  return native_->mSSRC;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_associatedStatId()
{
  return native_->mAssociatedStatID;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_isRemote()
{
  return native_->mIsRemote;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_mediaType()
{
  return native_->mMediaType;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_mediaTrackId()
{
  return native_->mMediaTrackID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_transportId()
{
  return native_->mTransportID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_codecId()
{
  return native_->mCodecID;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_firCount()
{
  return native_->mFIRCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_pliCount()
{
  return native_->mPLICount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_nackCount()
{
  return native_->mNACKCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_sliCount()
{
  return native_->mSLICount;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::wrapper_init_org_ortc_RTCOutboundRtpStreamStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCOutboundRtpStreamStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_packetsSent()
{
  return native_->mPacketsSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_bytesSent()
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_targetBitrate()
{
  return native_->mTargetBitrate;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::get_roundTripTime()
{
  return native_->mRoundTripTime;
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCOutboundRtpStreamStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCOutboundRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::NativeStatsPtr wrapper::impl::org::ortc::RTCOutboundRtpStreamStats::toNative(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCOutboundRtpStreamStats>(wrapper)->native_;
}
