
#include "impl_org_webrtc_RTCIceServer.h"
#include "impl_org_webrtc_enums.h"

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


// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCIceServer::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCIceServer::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCIceServer::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::IEnum, UseEnum);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCIceServer::RTCIceServer() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCIceServerPtr wrapper::org::webrtc::RTCIceServer::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCIceServer>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCIceServer::~RTCIceServer()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCIceServer::wrapper_init_org_webrtc_RTCIceServer() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCIceServer::wrapper_init_org_webrtc_RTCIceServer(wrapper::org::webrtc::RTCIceServerPtr source) noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.uri = from.uri;
  to.urls = make_shared< list< String > >();
  for (auto iter = from.urls.begin(); iter != from.urls.end(); ++iter) {
    to.urls->push_back(*iter);
  }
  to.username = from.username;
  to.credential = from.password;
  // to.oauthCredential = from.
  // to.credentialType = from.
  to.tlsCertPolicy = UseEnum::toWrapper(from.tls_cert_policy);
  to.hostname = from.hostname;
  to.tlsAlpnProtocols = make_shared< list< String > >();
  for (auto iter = from.tls_alpn_protocols.begin(); iter != from.tls_alpn_protocols.end(); ++iter) {
    to.tlsAlpnProtocols->push_back(*iter);
  }
  to.tlsEllipticCurves = make_shared< list< String > >();
  for (auto iter = from.tls_elliptic_curves.begin(); iter != from.tls_elliptic_curves.end(); ++iter) {
    to.tlsEllipticCurves->push_back(*iter);
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  to.uri = from.uri;
  if (from.urls) {
    for (auto iter = from.urls->begin(); iter != from.urls->end(); ++iter) {
      to.urls.push_back(*iter);
    }
  }
  to.username = from.username;
  to.password = from.credential;
  // to.oauthCredential = from.
  // to.credentialType = from.
  to.tls_cert_policy = UseEnum::toNative(from.tlsCertPolicy);
  to.hostname = from.hostname;
  if (from.tlsAlpnProtocols) {
    for (auto iter = from.tlsAlpnProtocols->begin(); iter != from.tlsAlpnProtocols->end(); ++iter) {
      to.tls_alpn_protocols.push_back(*iter);
    }
  }
  if (from.tlsEllipticCurves) {
    for (auto iter = from.tlsEllipticCurves->begin(); iter != from.tlsEllipticCurves->end(); ++iter) {
      to.tls_elliptic_curves.push_back(*iter);
    }
  }
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  apply(native, *result);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypePtr();
  auto cloned = make_shared<NativeType>();
  apply(*converted, *cloned);
  return cloned;
}
