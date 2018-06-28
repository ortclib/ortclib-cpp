
#include "impl_org_webrtc_VideoCapturer.h"

#include "impl_org_webrtc_pre_include.h"
#include "impl_org_webrtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::VideoCapturer::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::VideoCapturer::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::VideoCapturer::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::VideoCapturer::VideoCapturer() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoCapturerPtr wrapper::org::webrtc::VideoCapturer::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::VideoCapturer>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::VideoCapturer::~VideoCapturer()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::VideoCapturer::wrapper_init_org_webrtc_VideoCapturer() noexcept
{
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webrtc::VideoFormatPtr > > wrapper::impl::org::webrtc::VideoCapturer::getSupportedFormats() noexcept
{
  shared_ptr< list< wrapper::org::webrtc::VideoFormatPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoFormatPtr wrapper::impl::org::webrtc::VideoCapturer::getBestCaptureFormat(wrapper::org::webrtc::VideoFormatPtr desired) noexcept
{
  wrapper::org::webrtc::VideoFormatPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoCaptureState wrapper::impl::org::webrtc::VideoCapturer::start(wrapper::org::webrtc::VideoFormatPtr captureFormat) noexcept
{
  wrapper::org::webrtc::VideoCaptureState result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoFormatPtr wrapper::impl::org::webrtc::VideoCapturer::getCaptureFormat() noexcept
{
  wrapper::org::webrtc::VideoFormatPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::VideoCapturer::stop() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::VideoCapturer::constrainSupportedFormats(wrapper::org::webrtc::VideoFormatPtr maxFormat) noexcept
{
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::VideoCapturer::get_id() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::VideoCapturer::get_enableCameraList() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::VideoCapturer::set_enableCameraList(bool value) noexcept
{
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::VideoCapturer::get_enableVideoAdapter() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::VideoCapturer::set_enableVideoAdapter(bool value) noexcept
{
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::VideoCapturer::get_isRunning() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::VideoCapturer::get_applyRotation() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::VideoCapturer::get_isScreencast() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoCapturerInputSizePtr wrapper::impl::org::webrtc::VideoCapturer::get_inputSize() noexcept
{
  wrapper::org::webrtc::VideoCapturerInputSizePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoCaptureState wrapper::impl::org::webrtc::VideoCapturer::get_state() noexcept
{
  wrapper::org::webrtc::VideoCaptureState result {};
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypeUniPtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = std::move(native);
  return result;
}

//------------------------------------------------------------------------------
NativeTypeUniPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypeUniPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypeUniPtr();
  return NativeTypeUniPtr(std::move(converted->native_));
}
