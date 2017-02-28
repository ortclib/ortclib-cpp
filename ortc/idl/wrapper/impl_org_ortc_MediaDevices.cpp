
#include "impl_org_ortc_MediaDevices.h"
#include "impl_org_ortc_MediaStreamConstraints.h"
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_MediaDeviceInfo.h"
#include "impl_org_ortc_MediaTrackSupportedConstraints.h"
#include "impl_org_ortc_Helper.h"


#include <zsLib/IHelper.h>

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDevices::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDevices::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDevices::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDevices::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaDevices::MediaDevices()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaDevicesPtr wrapper::org::ortc::MediaDevices::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaDevices>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaDevices::~MediaDevices()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackSupportedConstraintsPtr wrapper::org::ortc::MediaDevices::getSupportedConstraints()
{
  return wrapper::impl::org::ortc::MediaTrackSupportedConstraints::toWrapper(NativeType::getSupportedConstraints());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< shared_ptr< list< wrapper::org::ortc::MediaDeviceInfoPtr > > > > wrapper::org::ortc::MediaDevices::enumerateDevices()
{
  ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< shared_ptr< list< wrapper::org::ortc::MediaDeviceInfoPtr > > >, PromiseWithType);

  auto promise = NativeType::enumerateDevices();

  auto result = PromiseWithType::create(wrapper::impl::org::ortc::Helper::getGuiQueue());

  promise->thenClosure([promise, result] {
    if (promise->isRejected()) {
      wrapper::impl::org::ortc::Helper::reject(promise, result);
      return;
    }
    auto wrapperList = make_shared< list< wrapper::org::ortc::MediaDeviceInfoPtr > >();
    auto nativeList = promise->value();
    for (auto iter = nativeList->begin(); iter != nativeList->end(); ++iter) {
      auto wrapper = wrapper::impl::org::ortc::MediaDeviceInfo::toWrapper(*iter);
      if (!wrapper) continue;
      wrapperList->push_back(wrapper);
    }

    result->resolve(wrapperList);
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< shared_ptr< list< wrapper::org::ortc::MediaStreamTrackPtr > > > > wrapper::org::ortc::MediaDevices::getUserMedia(wrapper::org::ortc::MediaStreamConstraintsPtr constraints)
{
  ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< shared_ptr< list< wrapper::org::ortc::MediaStreamTrackPtr > > >, PromiseWithType);

  auto native = wrapper::impl::org::ortc::MediaStreamConstraints::toNative(constraints);
  if (!native) return PromiseWithType::createRejected(wrapper::impl::org::ortc::Helper::getGuiQueue());
  auto promise = NativeType::getUserMedia(*native);

  auto result = PromiseWithType::create(wrapper::impl::org::ortc::Helper::getGuiQueue());

  promise->thenClosure([promise, result] {
    if (promise->isRejected()) {
      wrapper::impl::org::ortc::Helper::reject(promise, result);
      return;
    }
    auto wrapperList = make_shared< list< wrapper::org::ortc::MediaStreamTrackPtr > >();
    auto nativeList = promise->value();
    for (auto iter = nativeList->begin(); iter != nativeList->end(); ++iter) {
      auto wrapper = wrapper::impl::org::ortc::MediaStreamTrack::toWrapper(*iter);
      if (!wrapper) continue;
      wrapperList->push_back(wrapper);
    }

    result->resolve(wrapperList);
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaDevicesPtr wrapper::org::ortc::MediaDevices::get_singleton()
{
  zsLib::AutoRecursiveLock lock(*zsLib::IHelper::getGlobalLock());
  static zsLib::SingletonLazySharedPtr<MediaDevices> singleton(wrapper_create());
  auto result = singleton.singleton();
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaDevices::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onMediaDevicesChanged()
{
  onDeviceChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::subscribe()
{
  zsLib::AutoLock lock(lock_);
  if (subscriptionCount_ < 1) {
    if (!subscription_) return;
    subscription_->cancel();
    return;
  }
  if (subscription_) return;
  subscription_ = ::ortc::IMediaDevices::subscribe(thisWeak_.lock());
}
