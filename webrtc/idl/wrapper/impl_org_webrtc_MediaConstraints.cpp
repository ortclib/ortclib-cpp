
#include "impl_org_webrtc_MediaConstraints.h"
#include "impl_org_webrtc_Constraint.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaConstraints::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaConstraints::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaConstraints::NativeImplType, NativeImplType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::Constraint, WrapperConstraintType);

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
wrapper::impl::org::webrtc::MediaConstraints::MediaConstraints() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaConstraintsPtr wrapper::org::webrtc::MediaConstraints::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::MediaConstraints>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaConstraints::~MediaConstraints()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaConstraints::wrapper_init_org_webrtc_MediaConstraints() noexcept
{
  native_ = make_shared<NativeImplType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaConstraints::wrapper_init_org_webrtc_MediaConstraints(wrapper::org::webrtc::MediaConstraintsPtr source) noexcept
{
  if (!source) {
    native_ = make_shared<NativeImplType>();
    return;
  }
  wrapper_init_org_webrtc_MediaConstraints(source->get_mandatory(), source->get_optional());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaConstraints::wrapper_init_org_webrtc_MediaConstraints(
  shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > mandatory,
  shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > optional
  ) noexcept
{
  set_mandatory(mandatory);
  set_optional(optional);
}

//------------------------------------------------------------------------------
static shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > toWrapperList(const NativeType::Constraints &constraints)
{
  auto result = make_shared< list< wrapper::org::webrtc::ConstraintPtr > >();

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
  shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > value,
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
shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > wrapper::impl::org::webrtc::MediaConstraints::get_mandatory() noexcept
{
  if (!native_) return nullptr;
  return toWrapperList(native_->GetMandatory());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaConstraints::set_mandatory(shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > value) noexcept
{
  if (!native_) return;

  NativeType::Constraints constraints;
  native_->mandatory_.clear();
  fillNativeList(value, native_->mandatory_);
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > wrapper::impl::org::webrtc::MediaConstraints::get_optional() noexcept
{
  if (!native_) return nullptr;
  return toWrapperList(native_->GetOptional());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaConstraints::set_optional(shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > value) noexcept
{
  if (!native_) return;

  NativeType::Constraints constraints;
  native_->optional_.clear();
  fillNativeList(value, native_->optional_);
}


#pragma ZS_BUILD_NOTE("TODO","(robin) no conversion routines are present")
