
#include "impl_org_ortc_RTCStatsProvider.h"
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
wrapper::impl::org::ortc::RTCStatsProvider::RTCStatsProvider()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCStatsProviderPtr wrapper::org::ortc::RTCStatsProvider::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStatsProvider>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsProvider::~RTCStatsProvider()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCStatsProvider::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsProviderPtr wrapper::impl::org::ortc::RTCStatsProvider::toWrapper(NativeTypePtr native)
{
  if (!native) return RTCStatsProviderPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::RTCStatsProvider>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCStatsProvider::NativeTypePtr wrapper::impl::org::ortc::RTCStatsProvider::toNative(wrapper::org::ortc::RTCStatsProviderPtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  return std::dynamic_pointer_cast<RTCStatsProvider>(wrapper)->native_;
}
