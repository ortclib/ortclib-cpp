
#include "impl_org_ortc_RTCIceTransport.h"
#include "impl_org_ortc_RTCIceParameters.h"
#include "impl_org_ortc_RTCIceCandidate.h"
#include "impl_org_ortc_RTCIceCandidatePair.h"
#include "impl_org_ortc_RTCIceGatherer.h"
#include "impl_org_ortc_RTCIceGathererCandidate.h"
#include "impl_org_ortc_RTCIceParameters.h"
#include "impl_org_ortc_RTCIceTransportOptions.h"
#include "impl_org_ortc_RTCIceTransportStateChangeEvent.h"
#include "impl_org_ortc_RTCIceCandidatePairChangeEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransport::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransport::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransport::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceTransport::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransport::RTCIceTransport()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportPtr wrapper::org::ortc::RTCIceTransport::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceTransport>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceTransport::~RTCIceTransport()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCIceTransport::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::wrapper_init_org_ortc_RTCIceTransport()
{
  native_ = NativeType::create(thisWeak_.lock());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::wrapper_init_org_ortc_RTCIceTransport(wrapper::org::ortc::RTCIceGathererPtr gatherer)
{
  native_ = NativeType::create(thisWeak_.lock(), RTCIceGatherer::toNative(gatherer));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::start(
  wrapper::org::ortc::RTCIceGathererPtr gatherer,
  wrapper::org::ortc::RTCIceParametersPtr remoteParameters
  )
{
  auto nativeParams = RTCIceParameters::toNative(remoteParameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeParams);

  native_->start(RTCIceGatherer::toNative(gatherer), *nativeParams);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::start(
  wrapper::org::ortc::RTCIceGathererPtr gatherer,
  wrapper::org::ortc::RTCIceParametersPtr remoteParameters,
  wrapper::org::ortc::RTCIceRole role
  )
{
  auto nativeParams = RTCIceParameters::toNative(remoteParameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeParams);

  ::ortc::IICETransport::Options optionalOptions;
  optionalOptions.mRole = Helper::toNative(role);

  native_->start(RTCIceGatherer::toNative(gatherer), *nativeParams, optionalOptions);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::start(
  wrapper::org::ortc::RTCIceGathererPtr gatherer,
  wrapper::org::ortc::RTCIceParametersPtr remoteParameters,
  wrapper::org::ortc::RTCIceTransportOptionsPtr options
  )
{
  auto nativeParams = RTCIceParameters::toNative(remoteParameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeParams);

  auto nativeOptions = RTCIceTransportOptions::toNative(options);
  Optional<::ortc::IICETransport::Options> optionalOptions;
  if (nativeOptions) {
    optionalOptions = *nativeOptions;
  }

  native_->start(RTCIceGatherer::toNative(gatherer), *nativeParams, optionalOptions);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::stop()
{
  native_->stop();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportPtr wrapper::impl::org::ortc::RTCIceTransport::createAssociatedTransport()
{
  return RTCIceTransport::toWrapper(native_->createAssociatedTransport(::ortc::IICETransportDelegatePtr()));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::addRemoteCandidate(wrapper::org::ortc::RTCIceGathererCandidatePtr remoteCandidate)
{
  auto native = RTCIceGathererCandidate::toNative(remoteCandidate);
  if (!native) return;
  native_->addRemoteCandidate(*native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::setRemoteCandidates(shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > remoteCandidates)
{
  list< ::ortc::IICETypes::Candidate > nativeList;
  if (remoteCandidates) {
    for (auto iter = remoteCandidates->begin(); iter != remoteCandidates->end(); ++iter) {
      auto native = RTCIceCandidate::toNative(*iter);
      if (!native) continue;
      nativeList.push_back(*native);
    }
  }
  native_->setRemoteCandidates(nativeList);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::removeRemoteCandidate(wrapper::org::ortc::RTCIceGathererCandidatePtr remoteCandidate)
{
  auto native = RTCIceGathererCandidate::toNative(remoteCandidate);
  if (!native) return;
  native_->removeRemoteCandidate(*native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::keepWarm(wrapper::org::ortc::RTCIceCandidatePairPtr candidatePair)
{
  auto native = RTCIceCandidatePair::toNative(candidatePair);
  if (!native) return;
  native_->keepWarm(*native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::keepWarm(
  wrapper::org::ortc::RTCIceCandidatePairPtr candidatePair,
  bool keepWarm
  )
{
  auto native = RTCIceCandidatePair::toNative(candidatePair);
  if (!native) return;
  native_->keepWarm(*native, keepWarm);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCIceTransport::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererPtr wrapper::impl::org::ortc::RTCIceTransport::get_gatherer()
{
  return RTCIceGatherer::toWrapper(native_->iceGatherer());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceRole wrapper::impl::org::ortc::RTCIceTransport::get_role()
{
  return Helper::toWrapper(native_->role());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceComponent wrapper::impl::org::ortc::RTCIceTransport::get_component()
{
  return Helper::toWrapper(native_->component());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTransportState wrapper::impl::org::ortc::RTCIceTransport::get_state()
{
  return Helper::toWrapper(native_->state());
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > wrapper::impl::org::ortc::RTCIceTransport::get_remoteCandidates()
{
  auto result = make_shared< list< wrapper::org::ortc::RTCIceCandidatePtr > >();
  auto native = native_->getRemoteCandidates();
  if (native) {
    for (auto iter = native->begin(); iter != native->end(); ++iter) {
      auto wrapper = RTCIceCandidate::toWrapper(make_shared<::ortc::IICETypes::Candidate>(*iter));
      if (!wrapper) continue;
      result->push_back(wrapper);
    }
  }
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairPtr wrapper::impl::org::ortc::RTCIceTransport::get_selectedCandidatePair()
{
  return RTCIceCandidatePair::toWrapper(native_->getSelectedCandidatePair());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceParametersPtr wrapper::impl::org::ortc::RTCIceTransport::get_remoteParameters()
{
  return RTCIceParameters::toWrapper(native_->getRemoteParameters());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceTransport::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onICETransportStateChange(
  IICETransportPtr transport,
  IICETransport::States state
)
{
  onStateChange(RTCIceTransportStateChangeEvent::toWrapper(state));
}

//------------------------------------------------------------------------------
void WrapperImplType::onICETransportCandidatePairAvailable(
  IICETransportPtr transport,
  CandidatePairPtr candidatePair
)
{
  onCandidatePairAvailable(RTCIceCandidatePairChangeEvent::toWrapper(candidatePair));
}

//------------------------------------------------------------------------------
void WrapperImplType::onICETransportCandidatePairGone(
  IICETransportPtr transport,
  CandidatePairPtr candidatePair
)
{
  onCandidatePairGone(RTCIceCandidatePairChangeEvent::toWrapper(candidatePair));
}

//------------------------------------------------------------------------------
void WrapperImplType::onICETransportCandidatePairChanged(
  IICETransportPtr transport,
  CandidatePairPtr candidatePair
)
{
  onCandidatePairChange(RTCIceCandidatePairChangeEvent::toWrapper(candidatePair));
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
