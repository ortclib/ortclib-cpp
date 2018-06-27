
#include "impl_org_webrtc_enums.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/rtcerror.h"
#include "impl_org_webrtc_post_include.h"

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

wrapper::org::webrtc::RTCErrorType wrapper::impl::org::webrtc::IEnum::toWrapper(::webrtc::RTCErrorType value) noexcept
{
  switch (value) {
    case ::webrtc::RTCErrorType::NONE: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_none;
    case ::webrtc::RTCErrorType::UNSUPPORTED_OPERATION: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_unsupportedOperation;
    case ::webrtc::RTCErrorType::UNSUPPORTED_PARAMETER: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_unsupportedParameter;
    case ::webrtc::RTCErrorType::INVALID_PARAMETER: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_invalidParameter;
    case ::webrtc::RTCErrorType::INVALID_RANGE: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_invalidRange;
    case ::webrtc::RTCErrorType::SYNTAX_ERROR: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_syntaxError;
    case ::webrtc::RTCErrorType::INVALID_STATE: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_invalidState;
    case ::webrtc::RTCErrorType::INVALID_MODIFICATION: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_invalidModification;
    case ::webrtc::RTCErrorType::NETWORK_ERROR: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_networkError;
    case ::webrtc::RTCErrorType::RESOURCE_EXHAUSTED: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_resourceExhausted;
    case ::webrtc::RTCErrorType::INTERNAL_ERROR: return wrapper::org::webrtc::RTCErrorType::RTCErrorType_internalError;
  }
  ZS_ASSERT_FAIL("unknown type");
  return wrapper::org::webrtc::RTCErrorType::RTCErrorType_none;
}
