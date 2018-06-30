
#include "impl_org_ortc_RTCDataChannelStats.h"
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
wrapper::impl::org::ortc::RTCDataChannelStats::RTCDataChannelStats() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelStatsPtr wrapper::org::ortc::RTCDataChannelStats::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDataChannelStats>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelStats::~RTCDataChannelStats() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCDataChannelStats::toJson() noexcept
{
  return Json::toWrapper(native_->createElement("RTCDataChannelStats"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannelStats::hash() noexcept
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCDataChannelStats::get_timestamp() noexcept
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCDataChannelStats::get_statsType() noexcept
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannelStats::get_statsTypeOther() noexcept
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannelStats::get_id() noexcept
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannelStats::wrapper_init_org_ortc_RTCDataChannelStats() noexcept
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannelStats::wrapper_init_org_ortc_RTCDataChannelStats(wrapper::org::ortc::RTCDataChannelStatsPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_ortc_RTCDataChannelStats();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannelStats::wrapper_init_org_ortc_RTCDataChannelStats(wrapper::org::ortc::JsonPtr json) noexcept
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannelStats::get_label() noexcept
{
  return native_->mLabel;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannelStats::get_protocol() noexcept
{
  return native_->mProtocol;
}

//------------------------------------------------------------------------------
long wrapper::impl::org::ortc::RTCDataChannelStats::get_dataChannelId() noexcept
{
  return native_->mDataChannelID;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelState wrapper::impl::org::ortc::RTCDataChannelStats::get_state() noexcept
{
  return Helper::toWrapper(native_->mState);
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCDataChannelStats::get_messagesSent() noexcept
{
  return native_->mMessagesSent;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCDataChannelStats::get_bytesSent() noexcept
{
  return native_->mBytesSent;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCDataChannelStats::get_messagesReceived() noexcept
{
  return native_->mMessagesReceived;
}

//------------------------------------------------------------------------------
unsigned long long wrapper::impl::org::ortc::RTCDataChannelStats::get_bytesReceived() noexcept
{
  return native_->mBytesReceived;
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelStatsPtr wrapper::impl::org::ortc::RTCDataChannelStats::toWrapper(NativeStatsPtr native) noexcept
{
  if (!native) return RTCDataChannelStatsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDataChannelStats>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelStats::NativeStatsPtr wrapper::impl::org::ortc::RTCDataChannelStats::toNative(wrapper::org::ortc::RTCDataChannelStatsPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCDataChannelStats>(wrapper)->native_;
}

