
#include "impl_org_ortc_RTCCodec.h"
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
wrapper::impl::org::ortc::RTCCodec::RTCCodec()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCCodecPtr wrapper::org::ortc::RTCCodec::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCCodec>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCodec::~RTCCodec()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCCodec::toJson()
{
  return Json::toWrapper(native_->createElement("RTCCodec"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCodec::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
::zsLib::Time wrapper::impl::org::ortc::RTCCodec::get_timestamp()
{
  return native_->mTimestamp;
}

//------------------------------------------------------------------------------
Optional< wrapper::org::ortc::RTCStatsType > wrapper::impl::org::ortc::RTCCodec::get_statsType()
{
  if (!native_->mStatsType.hasValue()) return Optional< wrapper::org::ortc::RTCStatsType >();
  return Helper::toWrapper(native_->mStatsType.value());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCodec::get_statsTypeOther()
{
  return native_->mStatsTypeOther;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCodec::get_id()
{
  return native_->mID;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCCodec::wrapper_init_org_ortc_RTCCodec()
{
  native_ = make_shared<NativeStats>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCCodec::wrapper_init_org_ortc_RTCCodec(wrapper::org::ortc::RTCCodecPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_RTCCodec();
    return;
  }
  native_ = NativeStats::create(*toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCCodec::wrapper_init_org_ortc_RTCCodec(wrapper::org::ortc::JsonPtr json)
{
  native_ = NativeStats::create(Json::toNative(json));
}

//------------------------------------------------------------------------------
Optional< uint8_t > wrapper::impl::org::ortc::RTCCodec::get_payloadType()
{
  return native_->mPayloadType;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCodec::get_codec()
{
  return native_->mCodec;
}

//------------------------------------------------------------------------------
unsigned long wrapper::impl::org::ortc::RTCCodec::get_clockRate()
{
  return native_->mClockRate;
}

//------------------------------------------------------------------------------
Optional< unsigned long > wrapper::impl::org::ortc::RTCCodec::get_channels()
{
  return native_->mChannels;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCCodec::get_parameters()
{
  return native_->mParameters;
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCodecPtr wrapper::impl::org::ortc::RTCCodec::toWrapper(NativeStatsPtr native)
{
  if (!native) return RTCCodecPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCCodec>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCCodec::NativeStatsPtr wrapper::impl::org::ortc::RTCCodec::toNative(wrapper::org::ortc::RTCCodecPtr wrapper)
{
  if (!wrapper) return NativeStatsPtr();
  return std::dynamic_pointer_cast<RTCCodec>(wrapper)->native_;
}
