
#include "impl_org_ortc_OverconstrainedError.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OverconstrainedError::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OverconstrainedError::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OverconstrainedError::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::OverconstrainedError::OverconstrainedError()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::OverconstrainedErrorPtr wrapper::org::ortc::OverconstrainedError::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::OverconstrainedError>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::OverconstrainedError::~OverconstrainedError()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::OverconstrainedError::wrapper_init_org_ortc_OverconstrainedError()
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return OverconstrainedErrorPtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->name = native->mName;
  pThis->message = native->mMessage;
  pThis->constraint = native->mConstraint;
  return pThis;
}
