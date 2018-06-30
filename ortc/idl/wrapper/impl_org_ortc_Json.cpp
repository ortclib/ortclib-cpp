
#include "impl_org_ortc_Json.h"

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

ZS_DECLARE_TYPEDEF_PTR(zsLib::eventing::IHelper, UseHelper);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Json::Json() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::org::ortc::Json::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::Json>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Json::~Json() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::Json::wrapper_init_org_ortc_Json(String jsonString) noexcept
{
  native_ = UseHelper::toJSON(jsonString);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::Json::toString() noexcept
{
  if (!native_) return String();
  return UseHelper::toString(native_);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::JsonPtr wrapper::impl::org::ortc::Json::toWrapper(zsLib::XML::ElementPtr rootEl) noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::Json>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = rootEl;
  return pThis;
}

//------------------------------------------------------------------------------
zsLib::XML::ElementPtr wrapper::impl::org::ortc::Json::toNative(wrapper::org::ortc::JsonPtr wrapper) noexcept
{
  if (!wrapper) return zsLib::XML::ElementPtr();
  auto result = std::dynamic_pointer_cast<wrapper::impl::org::ortc::Json>(wrapper);
  if (!result) return zsLib::XML::ElementPtr();
  return result->native_;
}
