
#include "impl_org_ortc_Error.h"
#include "impl_org_ortc_Helper.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::Error::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::Error::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::Error::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Error::Error()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ErrorPtr wrapper::org::ortc::Error::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::Error>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Error::~Error()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::Error::wrapper_init_org_ortc_Error()
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->errorCode = SafeInt<decltype(pThis->errorCode)>(native.mErrorCode);
  pThis->name = native.mName;
  pThis->reason = native.mReason;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mErrorCode = SafeInt<decltype(result->mErrorCode)>(wrapper->errorCode);
  result->mName = wrapper->name;
  result->mReason = wrapper->reason;
  return result;
}
