// Generated by zsLibEventingTool

#include "impl_org_webRtc_RTCMediaStreamTrackStats.h"

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
wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::RTCMediaStreamTrackStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCMediaStreamTrackStatsPtr wrapper::org::webRtc::RTCMediaStreamTrackStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCMediaStreamTrackStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::~RTCMediaStreamTrackStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_timestamp() noexcept
{
  ::zsLib::Time result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCStatsType > wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_statsType() noexcept
{
  Optional< wrapper::org::webRtc::RTCStatsType > result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_statsTypeOther() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_id() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::wrapper_init_org_webRtc_RTCMediaStreamTrackStats() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::wrapper_init_org_webRtc_RTCMediaStreamTrackStats(wrapper::org::webRtc::RTCMediaStreamTrackStatsPtr source) noexcept
{
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_trackId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_remoteSource() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< uint32_t > > wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_ssrcIds() noexcept
{
  shared_ptr< list< uint32_t > > result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_frameWidth() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_frameHeight() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_framesPerSecond() noexcept
{
  double result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_framesSent() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_framesReceived() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_framesDecoded() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_framesDropped() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_framesCorrupted() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_audioLevel() noexcept
{
  double result {};
  return result;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_echoReturnLoss() noexcept
{
  double result {};
  return result;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::webRtc::RTCMediaStreamTrackStats::get_echoReturnLossEnhancement() noexcept
{
  double result {};
  return result;
}


