
#include "impl_org_webRtc_VideoCapturerInputSize.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoCapturerInputSize::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoCapturerInputSize::VideoCapturerInputSize() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoCapturerInputSizePtr wrapper::org::webRtc::VideoCapturerInputSize::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::VideoCapturerInputSize>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoCapturerInputSize::~VideoCapturerInputSize() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoCapturerInputSize::wrapper_init_org_webRtc_VideoCapturerInputSize() noexcept
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(int width, int height) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->width = SafeInt<decltype(result->width)>(width);
  result->height = SafeInt<decltype(result->height)>(height);
  return result;
}
