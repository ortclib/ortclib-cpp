
#include "impl_org_ortc_MediaDeviceInfo.h"
#include "impl_org_ortc_Json.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDeviceInfo::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDeviceInfo::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaDeviceInfo::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaDeviceInfo::MediaDeviceInfo()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaDeviceInfoPtr wrapper::org::ortc::MediaDeviceInfo::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaDeviceInfo>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaDeviceInfo::~MediaDeviceInfo()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaDeviceInfo::wrapper_init_org_ortc_MediaDeviceInfo()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaDeviceInfo::wrapper_init_org_ortc_MediaDeviceInfo(wrapper::org::ortc::MediaDeviceInfoPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaDeviceInfo::wrapper_init_org_ortc_MediaDeviceInfo(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaDeviceInfo::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaDeviceInfo::hash()
{
  return toNative(thisWeak_.lock())->hash();
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
  pThis->kind = Helper::toWrapper(native.mKind);
  pThis->label = native.mLabel;
  pThis->deviceId = native.mDeviceID;
  pThis->groupId = native.mGroupID;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mKind = Helper::toNative(wrapper->kind);
  result->mLabel = wrapper->label;
  result->mDeviceID = wrapper->deviceId;
  result->mGroupID = wrapper->groupId;
  return result;
}
