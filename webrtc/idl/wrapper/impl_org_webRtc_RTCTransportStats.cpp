// Generated by zsLibEventingTool

#include "impl_org_webRtc_RTCTransportStats.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCTransportStats::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCTransportStats::RTCTransportStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCTransportStatsPtr wrapper::org::webRtc::RTCTransportStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCTransportStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCTransportStats::~RTCTransportStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::webRtc::RTCTransportStats::get_timestamp() noexcept
{
  ::zsLib::Time result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCStatsType > wrapper::impl::org::webRtc::RTCTransportStats::get_statsType() noexcept
{
  Optional< wrapper::org::webRtc::RTCStatsType > result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_statsTypeOther() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_id() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCTransportStats::get_packetsSent() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::webRtc::RTCTransportStats::get_packetsReceived() noexcept
{
  unsigned long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::webRtc::RTCTransportStats::get_bytesSent() noexcept
{
  unsigned long long result {};
  return result;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::webRtc::RTCTransportStats::get_bytesReceived() noexcept
{
  unsigned long long result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_rtcpTransportStatsId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCIceRole > wrapper::impl::org::webRtc::RTCTransportStats::get_iceRole() noexcept
{
  Optional< wrapper::org::webRtc::RTCIceRole > result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtlsTransportState wrapper::impl::org::webRtc::RTCTransportStats::get_dtlsState() noexcept
{
  wrapper::org::webRtc::RTCDtlsTransportState result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_selectedCandidatePairId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_localCertificateId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_remoteCertificateId() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_dtlsCipher() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCTransportStats::get_srtpCipher() noexcept
{
  String result {};
  return result;
}


