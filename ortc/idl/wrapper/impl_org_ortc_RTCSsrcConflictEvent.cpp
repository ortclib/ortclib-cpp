
#include "impl_org_ortc_RTCSsrcConflictEvent.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSsrcConflictEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSsrcConflictEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSsrcConflictEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSsrcConflictEvent::RTCSsrcConflictEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSsrcConflictEventPtr wrapper::org::ortc::RTCSsrcConflictEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSsrcConflictEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSsrcConflictEvent::~RTCSsrcConflictEvent()
{
}

//------------------------------------------------------------------------------
uint32_t wrapper::impl::org::ortc::RTCSsrcConflictEvent::get_ssrc()
{
  return native_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType native)
{
  if (!native) return WrapperImplTypePtr();
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}
