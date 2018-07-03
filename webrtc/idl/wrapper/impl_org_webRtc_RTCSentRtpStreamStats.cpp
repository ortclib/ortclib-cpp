// Generated by zsLibEventingTool

#include "impl_org_webRtc_RTCSentRtpStreamStats.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCSentRtpStreamStats::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCSentRtpStreamStats::RTCSentRtpStreamStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSentRtpStreamStatsPtr wrapper::org::webRtc::RTCSentRtpStreamStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCSentRtpStreamStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCSentRtpStreamStats::~RTCSentRtpStreamStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_timestamp() noexcept
{
  ::zsLib::Time result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCStatsType > wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_statsType() noexcept
{
  Optional< wrapper::org::webRtc::RTCStatsType > result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_statsTypeOther() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_id() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< uint32_t > wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_ssrc() noexcept
{
  Optional< uint32_t > result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_kind() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_transportId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_codecId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_firCount() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_pliCount() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_nackCount() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_sliCount() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_qpSum() noexcept
{
  unsigned long long result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCSentRtpStreamStats::wrapper_init_org_webRtc_RTCSentRtpStreamStats() noexcept
{
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_packetsSent() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_packetsDiscardedOnSend() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_fecPacketsSent() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_bytesSent() noexcept
{
  unsigned long long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::webRtc::RTCSentRtpStreamStats::get_bytesDiscardedOnSend() noexcept
{
  unsigned long long result {};
  return result;
}


