
#include "impl_org_ortc_RTCSrtpSdesTransport.h"
#include "impl_org_ortc_RTCIceTransport.h"
#include "impl_org_ortc_RTCSrtpSdesParameters.h"
#include "impl_org_ortc_RTCSrtpSdesCryptoParameters.h"
#include "impl_org_ortc_ErrorEvent.h"
#include "impl_org_ortc_RTCSrtpSdesTransportLifetimeRemainingEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesTransport::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesTransport::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesTransport::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesTransport::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesTransport::RTCSrtpSdesTransport()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpSdesTransportPtr wrapper::org::ortc::RTCSrtpSdesTransport::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpSdesTransport>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesTransport::~RTCSrtpSdesTransport()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCSrtpSdesTransport::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);  
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesTransport::wrapper_init_org_ortc_RTCSrtpSdesTransport(
  wrapper::org::ortc::RTCIceTransportPtr iceTransport,
  wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr encryptParameters,
  wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr decryptParameters
  )
{
  auto nativeEnc = RTCSrtpSdesCryptoParameters::toNative(encryptParameters);
  auto nativeDec = RTCSrtpSdesCryptoParameters::toNative(decryptParameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeEnc);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeDec);

  native_ = ISRTPSDESTransport::create(thisWeak_.lock(), RTCIceTransport::toNative(iceTransport), *nativeEnc, *nativeDec);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpSdesParametersPtr wrapper::org::ortc::RTCSrtpSdesTransport::getLocalParameters()
{
  return wrapper::impl::org::ortc::RTCSrtpSdesParameters::toWrapper(NativeType::getLocalParameters());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesTransport::stop()
{
  native_->stop();
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCSrtpSdesTransport::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportPtr wrapper::impl::org::ortc::RTCSrtpSdesTransport::get_transport()
{
  return RTCIceTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportPtr wrapper::impl::org::ortc::RTCSrtpSdesTransport::get_rtcpTransport()
{
  return RTCIceTransport::toWrapper(native_->rtcpTransport());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesTransport::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onSRTPSDESTransportLifetimeRemaining(
  ISRTPSDESTransportPtr transport,
  ULONG leastLifetimeRemainingPercentageForAllKeys,
  ULONG overallLifetimeRemainingPercentage
)
{
  onLifetimeRemaining(RTCSrtpSdesTransportLifetimeRemainingEvent::toWrapper(leastLifetimeRemainingPercentageForAllKeys, overallLifetimeRemainingPercentage));
}

//------------------------------------------------------------------------------
void WrapperImplType::onSRTPSDESTransportError(
  ISRTPSDESTransportPtr transport,
  ::ortc::ErrorAnyPtr error
)
{
  onError(ErrorEvent::toWrapper(error));
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr track)
{
  if (!track) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = track;
  result->defaultSubscription_ = false;
  result->subscribe();
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

//------------------------------------------------------------------------------
void WrapperImplType::subscribe()
{
  if (defaultSubscription_) return;
  if (!native_) return;

  zsLib::AutoLock lock(lock_);
  if (subscriptionCount_ < 1) {
    if (!subscription_) return;
    subscription_->cancel();
    return;
  }
  if (subscription_) return;
  subscription_ = native_->subscribe(thisWeak_.lock());
}
