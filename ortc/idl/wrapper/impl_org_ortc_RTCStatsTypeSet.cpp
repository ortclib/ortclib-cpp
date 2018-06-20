
#include "impl_org_ortc_RTCStatsTypeSet.h"
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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsTypeSet::RTCStatsTypeSet() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCStatsTypeSetPtr wrapper::org::ortc::RTCStatsTypeSet::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStatsTypeSet>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = make_shared<wrapper::impl::org::ortc::RTCStatsTypeSet::NativeStatsTypeSet>();
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsTypeSet::~RTCStatsTypeSet() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCStatsTypeSet::wrapper_init_org_ortc_RTCStatsTypeSet() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCStatsTypeSet::wrapper_init_org_ortc_RTCStatsTypeSet(shared_ptr< set< wrapper::org::ortc::RTCStatsType > > values) noexcept
{
  if (!values) return;
  for (auto iter = values->begin(); iter != values->end(); ++iter)
  {
    auto native = Helper::toNative(*iter);
    native_->insert(native);
  }
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCStatsTypeSet::hasStatType(wrapper::org::ortc::RTCStatsType type) noexcept
{
  return native_->hasStatType(Helper::toNative(type));
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsTypeSetPtr wrapper::impl::org::ortc::RTCStatsTypeSet::toWrapper(NativeStatsTypeSetPtr native) noexcept
{
  if (!native) return RTCStatsTypeSetPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStatsTypeSet>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsTypeSet::NativeStatsTypeSetPtr wrapper::impl::org::ortc::RTCStatsTypeSet::toNative(wrapper::org::ortc::RTCStatsTypeSetPtr wrapper) noexcept
{
  if (!wrapper) return NativeStatsTypeSetPtr();
  return std::dynamic_pointer_cast<wrapper::impl::org::ortc::RTCStatsTypeSet>(wrapper)->native_;
}
