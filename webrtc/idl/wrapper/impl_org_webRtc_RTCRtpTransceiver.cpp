
#include "impl_org_webRtc_RTCRtpTransceiver.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtptransceiverinterface.h"
#include "pc/rtptransceiver.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);

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
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpTransceiver::setCodecPreferences(shared_ptr< list< wrapper::org::webRtc::RTCRtpCodecCapabilityPtr > > codecs) noexcept(false)
{
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCRtpTransceiver::get_mid() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSenderPtr wrapper::impl::org::webRtc::RTCRtpTransceiver::get_sender() noexcept
{
  wrapper::org::webRtc::RTCRtpSenderPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiverPtr wrapper::impl::org::webRtc::RTCRtpTransceiver::get_receiver() noexcept
{
  wrapper::org::webRtc::RTCRtpReceiverPtr result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCRtpTransceiver::get_stopped() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverDirection wrapper::impl::org::webRtc::RTCRtpTransceiver::get_direction() noexcept
{
  wrapper::org::webRtc::RTCRtpTransceiverDirection result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpTransceiver::set_direction(wrapper::org::webRtc::RTCRtpTransceiverDirection value) noexcept
{
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCRtpTransceiverDirection > wrapper::impl::org::webRtc::RTCRtpTransceiver::get_currentDirection() noexcept
{
  Optional< wrapper::org::webRtc::RTCRtpTransceiverDirection > result {};
  return result;
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
