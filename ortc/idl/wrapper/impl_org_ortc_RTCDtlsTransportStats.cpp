
#include "impl_org_ortc_RTCDtlsTransportStats.h"
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
wrapper::impl::org::ortc::RTCDtlsTransportStats::RTCDtlsTransportStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsTransportStatsPtr wrapper::org::ortc::RTCDtlsTransportStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDtlsTransportStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsTransportStats::~RTCDtlsTransportStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCDtlsTransportStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCDtlsTransportStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtlsTransportStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCDtlsTransportStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCDtlsTransportStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtlsTransportStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtlsTransportStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransportStats::wrapper_init_org_ortc_RTCDtlsTransportStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransportStats::wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::RTCDtlsTransportStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCDtlsTransportStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsTransportStats::wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtlsTransportStats::get_localCertificateId()
{
  return native_->mLocalCertificateID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtlsTransportStats::get_remoteCertificateId()
{
  return native_->mRemoteCertificateID;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsTransportStatsPtr wrapper::impl::org::ortc::RTCDtlsTransportStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCDtlsTransportStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDtlsTransportStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsTransportStats::NativeStatsPtr wrapper::impl::org::ortc::RTCDtlsTransportStats::toNative(wrapper::org::ortc::RTCDtlsTransportStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCDtlsTransportStats>(wrapper)->native_;
}
