
#include "impl_org_webRtc_RTCRtpTransceiver.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_RTCRtpCodecCapability.h"
#include "impl_org_webRtc_RTCRtpSender.h"
#include "impl_org_webRtc_RTCRtpReceiver.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtptransceiverinterface.h"
#include "pc/rtptransceiver.h"
#include "api/array_view.h"
#include "impl_org_webRtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver::NativeType, NativeType);

typedef WrapperImplType::NativeTypeScopedPtr NativeTypeScopedPtr;

typedef wrapper::impl::org::webRtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecCapability, UseRtpCodecCapability);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSender, UseRtpSender);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver, UseRtpReceiver);

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static NativeType *unproxy(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpTransceiver, ::webrtc::RtpTransceiver, native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpTransceiver::RTCRtpTransceiver() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::org::webRtc::RTCRtpTransceiver::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpTransceiver>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpTransceiver::~RTCRtpTransceiver() noexcept
{
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpTransceiver::stop() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;

  native_->Stop();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpTransceiver::setCodecPreferences(shared_ptr< list< wrapper::org::webRtc::RTCRtpCodecCapabilityPtr > > codecs) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return;

  if (!codecs) return;
  if (codecs->size() < 1) return;

  std::unique_ptr<::webrtc::RtpCodecCapability[]> nativeCodecs(new ::webrtc::RtpCodecCapability[codecs->size()]);

  size_t found = 0;
  for (auto iter = codecs->begin(); iter != codecs->end(); ++iter) {
    auto nativeValue = UseRtpCodecCapability::toNative(*iter);
    if (!nativeValue) continue;
    (nativeCodecs.get())[found] = *nativeValue;
      ++found;
  }

  rtc::ArrayView<::webrtc::RtpCodecCapability> view(nativeCodecs.get(), found);
  native_->SetCodecPreferences(view);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCRtpTransceiver::get_mid() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();

  auto value = native_->mid();
  if (value.has_value()) return value.value();
  return String();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSenderPtr wrapper::impl::org::webRtc::RTCRtpTransceiver::get_sender() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCRtpSenderPtr();

  return UseRtpSender::toWrapper(native_->sender());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiverPtr wrapper::impl::org::webRtc::RTCRtpTransceiver::get_receiver() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCRtpReceiverPtr();

  return UseRtpReceiver::toWrapper(native_->receiver());
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCRtpTransceiver::get_stopped() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return true;

  return native_->stopped();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverDirection wrapper::impl::org::webRtc::RTCRtpTransceiver::get_direction() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCRtpTransceiverDirection::RTCRtpTransceiverDirection_inactive;

  return UseEnum::toWrapper(native_->direction());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpTransceiver::set_direction(wrapper::org::webRtc::RTCRtpTransceiverDirection value) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;

  native_->SetDirection(UseEnum::toNative(value));
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCRtpTransceiverDirection > wrapper::impl::org::webRtc::RTCRtpTransceiver::get_currentDirection() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< wrapper::org::webRtc::RTCRtpTransceiverDirection >();

  auto result = native_->current_direction();
  if (!result.has_value()) return Optional< wrapper::org::webRtc::RTCRtpTransceiverDirection >();

  return UseEnum::toWrapper(result.value());
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver() noexcept
{
  //  if (!native_) return;
  //  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver() noexcept
{
  //  if (!observer_) return;
  //  if (!native_) return;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  NativeType *original = unproxy(native);

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
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypeScopedPtr native) noexcept
{
  return toWrapper(native.get());
}

//------------------------------------------------------------------------------
NativeTypeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}
