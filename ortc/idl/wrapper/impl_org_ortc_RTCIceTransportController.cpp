
#include "impl_org_ortc_RTCIceTransportController.h"
#include "impl_org_ortc_RTCIceTransport.h"
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

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(ortc_wrapper); } } } }

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportController::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportController::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransportController::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportController::RTCIceTransportController()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportControllerPtr wrapper::org::ortc::RTCIceTransportController::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceTransportController>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransportController::~RTCIceTransportController()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportController::wrapper_init_org_ortc_RTCIceTransportController()
{
  native_ = NativeType::create();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportController::addTransport(wrapper::org::ortc::RTCIceTransportPtr transport)
{
  return native_->addTransport(RTCIceTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransportController::addTransport(
  wrapper::org::ortc::RTCIceTransportPtr transport,
  uint64_t index
  )
{
  size_t native = SafeInt<size_t>(index);
  return native_->addTransport(RTCIceTransport::toNative(transport), native);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCIceTransportController::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCIceTransportPtr > > wrapper::impl::org::ortc::RTCIceTransportController::get_transports()
{
  auto result = make_shared< list< wrapper::org::ortc::RTCIceTransportPtr > >();
  auto native = native_->getTransports();
  for (auto iter = native.begin(); iter != native.end(); ++iter) {
    auto wrapper = RTCIceTransport::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr track)
{
  if (!track) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = track;
  return result;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!result) return NativeTypePtr();
  return result->native_;
}
