
#include "impl_org_webRtc_MessageEvent.h"

//#include "impl_org_webRtc_pre_include.h"
//#include "impl_org_webRtc_post_include.h"

#include <zsLib/eventing/IHelper.h>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MessageEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MessageEvent::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MessageEvent::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(zsLib::eventing::IHelper, UseHelper);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MessageEvent::MessageEvent() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MessageEventPtr wrapper::org::webRtc::MessageEvent::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::MessageEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MessageEvent::~MessageEvent() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
SecureByteBlockPtr wrapper::impl::org::webRtc::MessageEvent::get_binary() noexcept
{
  return native_;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::MessageEvent::get_text() noexcept
{
  if (isBinary_) return String();
  if (!native_) return String();

  return UseHelper::convertToString(*native_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto wrapper = make_shared<WrapperImplType>();
  wrapper->thisWeak_ = wrapper;
  wrapper->native_ = UseHelper::convertToBuffer(native.data.data(), native.size());
  wrapper->isBinary_ = native.binary;
  return wrapper;
}

//------------------------------------------------------------------------------
NativeType WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeType(rtc::CopyOnWriteBuffer(), true);
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  ZS_ASSERT(converted);
  if (!converted) return NativeType(rtc::CopyOnWriteBuffer(), false);
  auto buffer = converted->get_binary();
  if ((buffer) && (buffer->SizeInBytes() > 0)) {
    rtc::CopyOnWriteBuffer nativeBuffer(buffer->BytePtr(), buffer->SizeInBytes(), buffer->SizeInBytes() + sizeof(char)); // add space for nul character
    return NativeType(nativeBuffer, converted->isBinary_);
  }
  return NativeType(rtc::CopyOnWriteBuffer(), converted->isBinary_);
}
