
#include "impl_org_webRtc_RTCIceCandidate.h"
#include "impl_org_webRtc_RTCIceCandidateInit.h"
#include "impl_org_webRtc_RTCError.h"
#include "impl_org_webRtc_enums.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/jsepicecandidate.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>

#include <sstream>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidate::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError, UseError);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidateInit, UseIceCandidateInit);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCIceCandidate::RTCIceCandidate() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceCandidatePtr wrapper::org::webRtc::RTCIceCandidate::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCIceCandidate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCIceCandidate::~RTCIceCandidate() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCIceCandidate::wrapper_init_org_webRtc_RTCIceCandidate() noexcept
{
  native_ = std::make_unique<CricketNativeType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCIceCandidate::wrapper_init_org_webRtc_RTCIceCandidate(wrapper::org::webRtc::RTCIceCandidateInitPtr init) noexcept(false)
{
  if (!init) {
    wrapper_init_org_webRtc_RTCIceCandidate();
    return;
  }
  webrtc::SdpParseError error;
  NativeTypeUniPtr candidate(webrtc::CreateIceCandidate(init->sdpMid, init->sdpMLineIndex, init->candidate, &error));

  if (!candidate) {
    std::stringstream ss;
    ss << error.line;
    if (error.line.length() > 0) {
      ss << " <- ";
    }
    ss << error.description;
    throw UseError::toWrapper(::webrtc::RTCError(::webrtc::RTCErrorType::SYNTAX_ERROR, ss.str()));
  }

  native_ = std::make_unique<CricketNativeType>(candidate->candidate());
  mid_ = candidate->sdp_mid();
  mLineIndex = SafeInt<decltype(mLineIndex)::value_type>(candidate->sdp_mline_index());
  serverUrl_ = candidate->server_url();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceCandidateInitPtr wrapper::impl::org::webRtc::RTCIceCandidate::toJson() noexcept
{
  return UseIceCandidateInit::toWrapper(thisWeak_.lock());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCIceCandidate::get_candidate() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  ::zsLib::AutoLock lock(lock_);

  ::webrtc::JsepIceCandidate temp(mid_, mLineIndex.has_value() ? mLineIndex.value() : 0, *native_);
  String result;
  temp.ToString(&result);
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCIceCandidate::get_sdpMid() noexcept
{
  ::zsLib::AutoLock lock(lock_);
  return mid_;
}

//------------------------------------------------------------------------------
Optional< unsigned short > wrapper::impl::org::webRtc::RTCIceCandidate::get_sdpMLineIndex() noexcept
{
  ::zsLib::AutoLock lock(lock_);
  return mLineIndex;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCIceCandidate::get_foundation() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  ::zsLib::AutoLock lock(lock_);
  return native_->foundation();
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCIceComponent > wrapper::impl::org::webRtc::RTCIceCandidate::get_component() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< wrapper::org::webRtc::RTCIceComponent >();

  ::zsLib::AutoLock lock(lock_);
  auto value = native_->component();
  try {
    return UseEnum::toWrapperRTCIceComponent(value);
  } catch (const zsLib::Exceptions::InvalidArgument &) {
  }
  return Optional< wrapper::org::webRtc::RTCIceComponent >();
}

//------------------------------------------------------------------------------
Optional< unsigned long > wrapper::impl::org::webRtc::RTCIceCandidate::get_priority() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return 0;
  ::zsLib::AutoLock lock(lock_);
  return native_->priority();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCIceCandidate::get_ip() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  ::zsLib::AutoLock lock(lock_);
  return native_->address().HostAsURIString();
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCIceProtocol > wrapper::impl::org::webRtc::RTCIceCandidate::get_protocol() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< wrapper::org::webRtc::RTCIceProtocol >();

  ::zsLib::AutoLock lock(lock_);
  String str = native_->protocol();
  if (!str.hasData()) return Optional< wrapper::org::webRtc::RTCIceProtocol >();
  try {
    return UseEnum::toWrapperRTCIceProtocol(str.c_str());
  } catch (const zsLib::Exceptions::InvalidArgument &) {
  }
  return Optional< wrapper::org::webRtc::RTCIceProtocol >();
}

//------------------------------------------------------------------------------
Optional< uint16_t > wrapper::impl::org::webRtc::RTCIceCandidate::get_port() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< uint16_t >();
  ::zsLib::AutoLock lock(lock_);
  auto port = native_->address().port();
  if (port < 1) return Optional< uint16_t >();
  return SafeInt<uint16_t>(port);
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCIceCandidateType > wrapper::impl::org::webRtc::RTCIceCandidate::get_type() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< wrapper::org::webRtc::RTCIceCandidateType >();

  ::zsLib::AutoLock lock(lock_);
  String str = native_->type();
  if (!str.hasData()) return Optional< wrapper::org::webRtc::RTCIceCandidateType >();
  try {
    return UseEnum::toWrapperRTCIceCandidateType(str.c_str());
  } catch (const zsLib::Exceptions::InvalidArgument &) {
  }
  return Optional< wrapper::org::webRtc::RTCIceCandidateType >();
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCIceTcpCandidateType > wrapper::impl::org::webRtc::RTCIceCandidate::get_tcpType() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< wrapper::org::webRtc::RTCIceTcpCandidateType >();

  ::zsLib::AutoLock lock(lock_);
  String str = native_->tcptype();
  if (!str.hasData()) return Optional< wrapper::org::webRtc::RTCIceTcpCandidateType >();
  try {
    return UseEnum::toWrapperRTCIceTcpCandidateType(str.c_str());
  } catch (const zsLib::Exceptions::InvalidArgument &) {
  }
  return Optional< wrapper::org::webRtc::RTCIceTcpCandidateType >();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCIceCandidate::get_relatedAddress() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  ::zsLib::AutoLock lock(lock_);
  return native_->related_address().HostAsURIString();
}

//------------------------------------------------------------------------------
Optional< uint16_t > wrapper::impl::org::webRtc::RTCIceCandidate::get_relatedPort() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return Optional< uint16_t >();
  ::zsLib::AutoLock lock(lock_);
  auto port = native_->related_address().port();
  if (port < 1) return Optional< uint16_t >();
  return SafeInt<uint16_t>(port);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCIceCandidate::get_usernameFragment() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();
  ::zsLib::AutoLock lock(lock_);
  return native_->username();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const UseIceCandidateInit &candidate) noexcept
{
  auto mline = candidate.sdpMLineIndex;
  ::webrtc::SdpParseError error;
  auto result = ::webrtc::CreateIceCandidate(candidate.sdpMid, mline.has_value() ? mline.value() : -1, candidate.candidate, &error);
  return toWrapper(result);
}

WrapperImplTypePtr WrapperImplType::toWrapper(UseIceCandidateInitPtr candidate) noexcept
{
  if (!candidate) return WrapperImplTypePtr();
  return toWrapper(*candidate);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const CricketNativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = std::make_unique<CricketNativeType>(native);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const CricketNativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
shared_ptr< list< WrapperImplTypePtr > > WrapperImplType::toWrapper(const std::vector<CricketNativeType> &native) noexcept
{
  auto result = make_shared< list< WrapperImplTypePtr > >();

  for (auto iter = native.begin(); iter != native.end(); ++iter) {
    result->push_back(toWrapper(*iter));
  }
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = std::make_unique<CricketNativeType>(native.candidate());
  result->mid_ = native.sdp_mid();
  auto mline = native.sdp_mline_index();
  if (mline >= 0) {
    result->mLineIndex = SafeInt<decltype(result->mLineIndex)::value_type>(mline);
  }
  result->serverUrl_ = native.server_url();
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypeUniPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypeUniPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypeUniPtr();
  if (!converted->native_) return NativeTypeUniPtr();

  auto mLine = wrapper->get_sdpMLineIndex();
  auto result = std::make_unique<::webrtc::JsepIceCandidate>(wrapper->get_sdpMid(), mLine.has_value() ? mLine.value() : -1, *converted->native_);
  return std::move(result);
}
