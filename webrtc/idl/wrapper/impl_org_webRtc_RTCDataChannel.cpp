
#include "impl_org_webRtc_RTCDataChannel.h"
#include "impl_org_webRtc_MessageEvent.h"
#include "impl_org_webRtc_WebrtcLib.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_enums.h"

#include "impl_org_webRtc_pre_include.h"
#include "pc/datachannel.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>

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


// borrow types from call defintions
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel::NativeType, NativeType);

typedef WrapperImplType::NativeScopedPtr NativeScopedPtr;

typedef wrapper::impl::org::webRtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MessageEvent, UseMessageEvent);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static NativeType *unproxy(NativeType *native)
{
  if (!native) return native;
  auto converted = dynamic_cast<::webrtc::DataChannel *>(native);
  if (!converted) return native;

  return WRAPPER_DEPROXIFY_CLASS(::webrtc, DataChannel, converted);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDataChannel::RTCDataChannel() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelPtr wrapper::org::webRtc::RTCDataChannel::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCDataChannel>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDataChannel::~RTCDataChannel() noexcept
{
  thisWeak_.reset();
  teardownObserver();
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCStatsReportPtr > > wrapper::impl::org::webRtc::RTCDataChannel::getStats(wrapper::org::webRtc::RTCStatsTypeSetPtr statTypes) noexcept(false)
{
#pragma ZS_BUILD_NOTE("TODO","getStats")
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCStatsReportPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannel::close() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  notifiedOpen_ = true;
  notifiedClosed_ = true;
  native_->Close();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannel::send(String text) noexcept(false)
{
  bufferLowNotified_ = false;

  ZS_ASSERT(native_);
  if (!native_) return;

  auto wrapperValue = UseMessageEvent::toWrapper(text);
  auto nativeValue = UseMessageEvent::toNative(wrapperValue);
  native_->Send(nativeValue);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannel::send(SecureByteBlockPtr data) noexcept(false)
{
  bufferLowNotified_ = false;

  ZS_ASSERT(native_);
  if (!native_) return;

  auto wrapperValue = UseMessageEvent::toWrapper(data);
  auto nativeValue = UseMessageEvent::toNative(wrapperValue);
  native_->Send(nativeValue);
}

//------------------------------------------------------------------------------
unsigned short wrapper::impl::org::webRtc::RTCDataChannel::get_id() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return 0;
  return SafeInt<unsigned short>(native_->id());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCDataChannel::get_label() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  return native_->label();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCDataChannel::get_ordered() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  return native_->ordered();
}

//------------------------------------------------------------------------------
Optional< ::zsLib::Milliseconds > wrapper::impl::org::webRtc::RTCDataChannel::get_maxPacketLifeTime() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return ::zsLib::Milliseconds();
  auto max = native_->maxRetransmitTime();
  if (max < 0) return Optional< ::zsLib::Milliseconds >();
  return ::zsLib::Milliseconds(SafeInt<::zsLib::Milliseconds::rep>());
}

//------------------------------------------------------------------------------
Optional< unsigned short > wrapper::impl::org::webRtc::RTCDataChannel::get_maxRetransmits() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< unsigned short >();
  auto value = native_->maxRetransmits();
  if (value < 0) return Optional< unsigned short >();
  return value;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCDataChannel::get_protocol() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  return native_->protocol();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCDataChannel::get_negotiated() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_veryLow;
  return native_->negotiated();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCPriorityType wrapper::impl::org::webRtc::RTCDataChannel::get_priority() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_veryLow;

#pragma ZS_BUILD_NOTE("LATER","Does not appear to have any method to get / set priority")
  return wrapper::org::webRtc::RTCPriorityType::RTCPriorityType_medium;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelState wrapper::impl::org::webRtc::RTCDataChannel::get_readyState() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCDataChannelState::RTCDataChannelState_closed;

  return UseEnum::toWrapper(native_->state());
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::webRtc::RTCDataChannel::get_bufferedAmount() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return 0;

  return native_->buffered_amount();
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::webRtc::RTCDataChannel::get_bufferedAmountLowThreshold() noexcept
{
  return bufferLowThreshold_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannel::set_bufferedAmountLowThreshold(uint64_t value) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;

  bufferLowThreshold_ = value;

  auto bufferedAmount = native_->buffered_amount();
  if (bufferedAmount < value) {
    // notify async to self object about buffer amount change to cause a notification
    observer_->OnBufferedAmountChange(bufferedAmount);
  }
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCDataChannel::get_binaryType() noexcept
{
  zsLib::AutoLock lock(lock_);
  return binaryType_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannel::set_binaryType(String value) noexcept
{
  zsLib::AutoLock lock(lock_);
  binaryType_ = value;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDataChannel::wrapper_onObserverCountChanged(ZS_MAYBE_USED() size_t count) noexcept
{
  ZS_MAYBE_USED(count);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverStateChange() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  auto state = native_->state();
  switch (state) {
    case ::webrtc::DataChannelInterface::DataState::kConnecting:  break;
    case ::webrtc::DataChannelInterface::DataState::kOpen: {
      if (!notifiedOpen_.exchange(true)) onOpen();
      break;
    }
    case ::webrtc::DataChannelInterface::DataState::kClosing: {
      if (!notifiedOpen_.exchange(true)) onOpen();
      break;
    }
    case ::webrtc::DataChannelInterface::DataState::kClosed: {
      if (!notifiedClosed_.exchange(true)) onClose();
      break;
    }
  }
  
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverMessage(const ::webrtc::DataBuffer& buffer) noexcept
{
  auto messageEvent = UseMessageEvent::toWrapper(buffer);
  onMessage(messageEvent);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverBufferedAmountChange(ZS_MAYBE_USED() uint64_t previous_amount) noexcept
{
  ZS_MAYBE_USED(previous_amount);
  ZS_ASSERT(native_);
  if (!native_) return;

  auto bufferedAmount = native_->buffered_amount();

  if (bufferedAmount < bufferLowThreshold_) {
    if (!bufferLowNotified_.exchange(true)) onBufferedAmountLow();
  }
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver()
{
  if (!native_) return;
  if (observer_) return;

  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
  native_->RegisterObserver(observer_.get());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver()
{
  if (!observer_) return;
  if (!native_) return;

  native_->UnregisterObserver();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  auto original = unproxy(native);

  // search for original non-proxied pointer in map
  auto wrapper = mapperSingleton().getExistingOrCreateNew(original, [native]() {
    auto result = make_shared<WrapperImplType>();
    result->thisWeak_ = result;
    result->native_ = rtc::scoped_refptr<NativeType>(native); // only use proxy and never original pointer
    result->setupObserver();
    return result;
  });
  return wrapper;
}

//------------------------------------------------------------------------------
rtc::scoped_refptr<NativeType> WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}

