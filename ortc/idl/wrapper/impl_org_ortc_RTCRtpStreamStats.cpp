
#include "impl_org_ortc_RTCRtpStreamStats.h"
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
wrapper::impl::org::ortc::RTCRtpStreamStats::RTCRtpStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpStreamStatsPtr wrapper::org::ortc::RTCRtpStreamStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpStreamStats::~RTCRtpStreamStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpStreamStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCRtpStreamStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCRtpStreamStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCRtpStreamStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpStreamStats::wrapper_init_org_ortc_RTCRtpStreamStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpStreamStats::wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::RTCRtpStreamStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCRtpStreamStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpStreamStats::wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
Optional< uint32_t > wrapper::impl::org::ortc::RTCRtpStreamStats::get_ssrc()
{
  return native_->mSSRC;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_associatedStatId()
{
  return native_->mAssociatedStatID;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCRtpStreamStats::get_isRemote()
{
  return native_->mIsRemote;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_mediaType()
{
  return native_->mMediaType;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_mediaTrackId()
{
  return native_->mMediaTrackID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_transportId()
{
  return native_->mTransportID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpStreamStats::get_codecId()
{
  return native_->mCodecID;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCRtpStreamStats::get_firCount()
{
  return native_->mFIRCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCRtpStreamStats::get_pliCount()
{
  return native_->mPLICount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCRtpStreamStats::get_nackCount()
{
  return native_->mNACKCount;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCRtpStreamStats::get_sliCount()
{
  return native_->mSLICount;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpStreamStatsPtr wrapper::impl::org::ortc::RTCRtpStreamStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCRtpStreamStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpStreamStats::NativeStatsPtr wrapper::impl::org::ortc::RTCRtpStreamStats::toNative(wrapper::org::ortc::RTCRtpStreamStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCRtpStreamStats>(wrapper)->native_;
}
