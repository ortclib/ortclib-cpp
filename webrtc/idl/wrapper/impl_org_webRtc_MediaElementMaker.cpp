
#include "impl_org_webRtc_MediaElementMaker.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaElementMaker::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MediaElementMaker::~MediaElementMaker() noexcept
{
}

#if 0
//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr wrapper::org::webRtc::MediaElementMaker::bind(AnyPtr element) noexcept
{
  wrapper::org::webRtc::MediaElementPtr result {};
  return result;
}

//------------------------------------------------------------------------------
AnyPtr wrapper::org::webRtc::MediaElementMaker::extract(wrapper::org::webRtc::MediaElementPtr element) noexcept
{
  AnyPtr result {};
  return result;
}
#endif //0

