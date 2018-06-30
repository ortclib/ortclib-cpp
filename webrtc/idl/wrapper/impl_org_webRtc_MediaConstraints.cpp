
#include "impl_org_webRtc_MediaConstraints.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaConstraints::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaConstraints::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaConstraints::NativeImplType, NativeImplType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::Constraint, WrapperConstraintType);

//------------------------------------------------------------------------------
NativeImplType::~MediaConstraintsImpl() noexcept
{
}

//------------------------------------------------------------------------------
NativeImplType::MediaConstraintsImpl() noexcept
{
}

//------------------------------------------------------------------------------
NativeImplType::MediaConstraintsImpl(
  const NativeType::Constraints& mandatory,
  const NativeType::Constraints& optional
) noexcept : mandatory_(mandatory), optional_(optional)
{
}

//------------------------------------------------------------------------------
const NativeType::Constraints& NativeImplType::GetMandatory() const
{
  return mandatory_;
}

//------------------------------------------------------------------------------
const NativeType::Constraints& NativeImplType::GetOptional() const
{
  return optional_;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MediaConstraints::MediaConstraints() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaConstraintsPtr wrapper::org::webRtc::MediaConstraints::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::MediaConstraints>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MediaConstraints::~MediaConstraints() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MediaConstraints::wrapper_init_org_webRtc_MediaConstraints() noexcept
{
  native_ = make_shared<NativeImplType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MediaConstraints::wrapper_init_org_webRtc_MediaConstraints(wrapper::org::webRtc::MediaConstraintsPtr source) noexcept
{
  if (!source) {
    native_ = make_shared<NativeImplType>();
    return;
  }
  wrapper_init_org_webRtc_MediaConstraints(source->get_mandatory(), source->get_optional());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MediaConstraints::wrapper_init_org_webRtc_MediaConstraints(
  shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > mandatory,
  shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > optional
  ) noexcept
{
  set_mandatory(mandatory);
  set_optional(optional);
}

//------------------------------------------------------------------------------
static shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > toWrapperList(const NativeType::Constraints &constraints)
{
  auto result = make_shared< list< wrapper::org::webRtc::ConstraintPtr > >();

  for (auto iter = constraints.begin(); iter != constraints.end(); ++iter) {
    auto &constraint = (*iter);
    auto converted = WrapperConstraintType::toWrapper(constraint);
    if (!converted) continue;
    result->push_back(converted);
  }
  return result;
}

//------------------------------------------------------------------------------
void fillNativeList(
  shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > value,
  NativeType::Constraints &output
)
{
  if (!value) return;
  for (auto iter = value->begin(); iter != value->end(); ++iter) {
    auto &wrapperType = (*iter);
    auto converted = WrapperConstraintType::toNative(wrapperType);
    if (!converted) continue;
    output.push_back(*converted);
  }
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > wrapper::impl::org::webRtc::MediaConstraints::get_mandatory() noexcept
{
  if (!native_) return nullptr;
  return toWrapperList(native_->GetMandatory());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MediaConstraints::set_mandatory(shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > value) noexcept
{
  if (!native_) return;

  NativeType::Constraints constraints;
  native_->mandatory_.clear();
  fillNativeList(value, native_->mandatory_);
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > wrapper::impl::org::webRtc::MediaConstraints::get_optional() noexcept
{
  if (!native_) return nullptr;
  return toWrapperList(native_->GetOptional());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MediaConstraints::set_optional(shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > value) noexcept
{
  if (!native_) return;

  NativeType::Constraints constraints;
  native_->optional_.clear();
  fillNativeList(value, native_->optional_);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto nativeImpl = make_shared<NativeImplType>(native.GetMandatory(), native.GetOptional());

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = nativeImpl;
  return result;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  ZS_ASSERT(converted);
  if (!converted) return NativeTypePtr();
  return converted->native_;
}
