
#include "impl_org_ortc_RTCMediaStreamTrackStats.h"
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
wrapper::impl::org::ortc::RTCMediaStreamTrackStats::RTCMediaStreamTrackStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCMediaStreamTrackStatsPtr wrapper::org::ortc::RTCMediaStreamTrackStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMediaStreamTrackStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamTrackStats::~RTCMediaStreamTrackStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCMediaStreamTrackStats::toJson() noexcept
{
  return Json::toWrapper(native_->createElement("RTCMediaStreamTrackStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamTrackStats::hash() noexcept
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_timestamp() noexcept
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_statsType() noexcept
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_statsTypeOther() noexcept
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_id() noexcept
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamTrackStats::wrapper_init_org_ortc_RTCMediaStreamTrackStats() noexcept
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamTrackStats::wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_ortc_RTCMediaStreamTrackStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCMediaStreamTrackStats::wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::JsonPtr json) noexcept
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_trackId() noexcept
{
  return native_->mTrackID;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_remoteSource() noexcept
{
  return native_->mRemoteSource;
}

//------------------------------------------------------------------------------
shared_ptr< list< uint32_t > > wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_ssrcIds() noexcept
{
  auto result = make_shared< list<uint32_t> >();
  for (auto iter = native_->mSSRCIDs.begin(); iter != native_->mSSRCIDs.end(); ++iter)
  {
    result->push_back(*iter);
  }
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_frameWidth() noexcept
{
  return native_->mFrameWidth;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_frameHeight() noexcept
{
  return native_->mFrameHeight;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_framesPerSecond() noexcept
{
  return native_->mFramesPerSecond;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_framesSent() noexcept
{
  return native_->mFramesSent;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_framesReceived() noexcept
{
  return native_->mFramesReceived;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_framesDecoded() noexcept
{
  return native_->mFramesDecoded;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_framesDropped() noexcept
{
  return native_->mFramesDropped;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_framesCorrupted() noexcept
{
  return native_->mFramesCorrupted;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_audioLevel() noexcept
{
  return native_->mAudioLevel;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_echoReturnLoss() noexcept
{
  return native_->mEchoReturnLoss;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCMediaStreamTrackStats::get_echoReturnLossEnhancement() noexcept
{
  return native_->mEchoReturnLossEnhancement;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamTrackStatsPtr wrapper::impl::org::ortc::RTCMediaStreamTrackStats::toWrapper(NativeStatsPtr native) noexcept
{
  if (!native) return RTCMediaStreamTrackStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCMediaStreamTrackStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCMediaStreamTrackStats::NativeStatsPtr wrapper::impl::org::ortc::RTCMediaStreamTrackStats::toNative(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCMediaStreamTrackStats>(wrapper)->native_;
}

