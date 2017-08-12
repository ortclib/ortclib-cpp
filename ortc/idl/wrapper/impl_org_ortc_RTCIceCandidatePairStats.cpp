
#include "impl_org_ortc_RTCIceCandidatePairStats.h"
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
wrapper::impl::org::ortc::RTCIceCandidatePairStats::RTCIceCandidatePairStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper::org::ortc::RTCIceCandidatePairStats::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidatePairStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairStats::~RTCIceCandidatePairStats()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceCandidatePairStats::toJson()
{
  return Json::toWrapper(native_->createElement("RTCIceCandidatePairStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePairStats::wrapper_init_org_ortc_RTCIceCandidatePairStats()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePairStats::wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::RTCIceCandidatePairStatsPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCIceCandidatePairStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePairStats::wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_transportId()
{
  return native_->mTransportID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_localCandidateId()
{
  return native_->mLocalCandidateID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_remoteCandidateId()
{
  return native_->mRemoteCandidateID;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairState wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_state()
{
  return Helper::toWrapper(native_->mState);
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_priority()
{
  return native_->mPriority;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_nominated()
{
  return native_->mNominated;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_writable()
{
  return native_->mWritable;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_readable()
{
  return native_->mReadable;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_bytesSent()
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_bytesReceived()
{
  return native_->mBytesReceived;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_roundTripTime()
{
  return native_->mRoundTripTime;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_availableOutgoingBitrate()
{
  return native_->mAvailableOutgoingBitrate;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_availableIncomingBitrate()
{
  return native_->mAvailableIncomingBitrate;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairStatsPtr wrapper::impl::org::ortc::RTCIceCandidatePairStats::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCIceCandidatePairStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidatePairStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairStats::NativeStatsPtr wrapper::impl::org::ortc::RTCIceCandidatePairStats::toNative(wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCIceCandidatePairStats>(wrapper)->native_;
}

