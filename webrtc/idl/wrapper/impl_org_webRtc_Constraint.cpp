
#include "impl_org_webRtc_Constraint.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::Constraint::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::Constraint::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::Constraint::Constraint() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::ConstraintPtr wrapper::org::webRtc::Constraint::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::Constraint>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::Constraint::~Constraint() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::Constraint::wrapper_init_org_webRtc_Constraint() noexcept
{
  native_ = make_shared<NativeType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::Constraint::wrapper_init_org_webRtc_Constraint(wrapper::org::webRtc::ConstraintPtr source) noexcept
{
  if (!source) {
    native_ = make_shared<NativeType>();
    return;
  }
  native_ = make_shared<NativeType>(source->get_key(), source->get_value());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::Constraint::wrapper_init_org_webRtc_Constraint(
  String key,
  String value
  ) noexcept
{
  native_ = make_shared<NativeType>(key, value);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::Constraint::get_key() noexcept
{
  if (!native_) return String();
  return native_->key;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::Constraint::set_key(String value) noexcept
{
  if (!native_) return;
  native_->key = value;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::Constraint::get_value() noexcept
{
  if (!native_) return String();
  return native_->value;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::Constraint::set_value(String value) noexcept
{
  if (!native_) return;
  native_->value = value;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(native.key, native.value);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return nullptr;
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return nullptr;
  auto cast = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!cast) return nullptr;
  return cast->native_;
}
