
#include "impl_org_ortc_RTCIceServer.h"
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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceServer::RTCIceServer()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceServerPtr wrapper::org::ortc::RTCIceServer::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceServer>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceServer::~RTCIceServer()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceServer::wrapper_init_org_ortc_RTCIceServer()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceServer::wrapper_init_org_ortc_RTCIceServer(wrapper::org::ortc::RTCIceServerPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceServer::wrapper_init_org_ortc_RTCIceServer(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceServer::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCIceServer"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceServer::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceServer::WrapperImplTypePtr wrapper::impl::org::ortc::RTCIceServer::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceServer::WrapperImplTypePtr wrapper::impl::org::ortc::RTCIceServer::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->urls = make_shared< list<String> >(native.mURLs);
  pThis->username = native.mUserName;
  pThis->credential = native.mCredential;
  pThis->credentialType = Helper::toWrapper(native.mCredentialType);
  return pThis;


  shared_ptr< list< String > > urls{};
  String username{};
  String credential{};
  wrapper::org::ortc::RTCIceCredentialType credentialType{ wrapper::org::ortc::RTCIceCredentialType_password };
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceServer::NativeTypePtr wrapper::impl::org::ortc::RTCIceServer::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  if (wrapper->urls) {
    result->mURLs = *(wrapper->urls);
  }
  result->mUserName = wrapper->username;
  result->mCredential = wrapper->credential;
  result->mCredentialType = Helper::toNative(wrapper->credentialType);
  return result;
}
