
#include "impl_org_ortc_RTCCertificateStats.h"
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
wrapper::impl::org::ortc::RTCCertificateStats::RTCCertificateStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCCertificateStatsPtr wrapper::org::ortc::RTCCertificateStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCCertificateStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificateStats::~RTCCertificateStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCCertificateStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCCertificateStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCCertificateStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCCertificateStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCCertificateStats::wrapper_init_org_ortc_RTCCertificateStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCCertificateStats::wrapper_init_org_ortc_RTCCertificateStats(wrapper::org::ortc::RTCCertificateStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCCertificateStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCCertificateStats::wrapper_init_org_ortc_RTCCertificateStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::get_fingerprint()
{
  return native_->mFingerprint;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::get_fingerprintAlgorithm()
{
  return native_->mFingerprintAlgorithm;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::get_base64Certificate()
{
  return native_->mBase64Certificate;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCertificateStats::get_issuerCertificateId()
{
  return native_->mIssuerCertificateID;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificateStatsPtr wrapper::impl::org::ortc::RTCCertificateStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCCertificateStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCCertificateStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCertificateStats::NativeStatsPtr wrapper::impl::org::ortc::RTCCertificateStats::toNative(wrapper::org::ortc::RTCCertificateStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCCertificateStats>(wrapper)->native_;
}

