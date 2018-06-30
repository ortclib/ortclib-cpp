
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
wrapper::impl::org::ortc::RTCIceCandidatePairStats::RTCIceCandidatePairStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper::org::ortc::RTCIceCandidatePairStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidatePairStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairStats::~RTCIceCandidatePairStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceCandidatePairStats::toJson() noexcept
{
  return Json::toWrapper(native_->createElement("RTCIceCandidatePairStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::hash() noexcept
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_timestamp() noexcept
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_statsType() noexcept
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_statsTypeOther() noexcept
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_id() noexcept
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePairStats::wrapper_init_org_ortc_RTCIceCandidatePairStats() noexcept
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePairStats::wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::RTCIceCandidatePairStatsPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_ortc_RTCIceCandidatePairStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePairStats::wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::JsonPtr json) noexcept
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_transportId() noexcept
{
  return native_->mTransportID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_localCandidateId() noexcept
{
  return native_->mLocalCandidateID;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_remoteCandidateId() noexcept
{
  return native_->mRemoteCandidateID;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairState wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_state() noexcept
{
  return Helper::toWrapper(native_->mState);
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_priority() noexcept
{
  return native_->mPriority;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_nominated() noexcept
{
  return native_->mNominated;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_writable() noexcept
{
  return native_->mWritable;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_readable() noexcept
{
  return native_->mReadable;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_bytesSent() noexcept
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_bytesReceived() noexcept
{
  return native_->mBytesReceived;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_roundTripTime() noexcept
{
  return native_->mRoundTripTime;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_availableOutgoingBitrate() noexcept
{
  return native_->mAvailableOutgoingBitrate;
}

//------------------------------------------------------------------------------
double wrapper::impl::org::ortc::RTCIceCandidatePairStats::get_availableIncomingBitrate() noexcept
{
  return native_->mAvailableIncomingBitrate;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairStatsPtr wrapper::impl::org::ortc::RTCIceCandidatePairStats::toWrapper(NativeStatsPtr native) noexcept
{
  if (!native) return RTCIceCandidatePairStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidatePairStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePairStats::NativeStatsPtr wrapper::impl::org::ortc::RTCIceCandidatePairStats::toNative(wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCIceCandidatePairStats>(wrapper)->native_;
}

