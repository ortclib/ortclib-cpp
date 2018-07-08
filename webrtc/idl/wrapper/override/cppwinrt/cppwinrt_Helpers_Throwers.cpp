
#include "pch.h"

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/generated/org_webrtc_RTCError.h>

#include <zsLib/types.h>
#include <zsLib/String.h>

using namespace winrt;

typedef Internal::Helper::Throwers ThrowersType;

ThrowersType &ThrowersType::singleton() noexcept
{
  static ThrowersType result{};
  return result;
}

//#define FACILITY_WIN32 0x0007

#define CUSTOM_MAKE_HRESULT(xCustomerBit, xFacility, xErrorCode) ((HRESULT)(xErrorCode) <= 0 ? ((HRESULT)(xErrorCode)) : ((HRESULT) (((xErrorCode) & 0x0000FFFF) | ((xFacility) << 16) | ((xCustomerBit) << 29) | 0x80000000)))


void ThrowersType::customThrow(wrapper::org::webRtc::RTCErrorPtr error) noexcept(false)
{
  if (!error) throw hresult_error(E_FAIL, L"No error object provided.");

  std::wstring message = error->get_message().wstring();

  switch (error->get_type())
  {
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_none:                 break;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedOperation: throw hresult_illegal_method_call(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedParameter: throw hresult_error(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_UNSUPPORTED_TYPE), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidParameter:     throw hresult_invalid_argument(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidRange:         throw hresult_out_of_bounds(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_syntaxError:          throw hresult_error(MK_E_SYNTAX, message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidState:         throw hresult_error(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_INVALID_STATE), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidModification:  throw hresult_illegal_state_change(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_networkError:         throw hresult_error(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_UNEXP_NET_ERR), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_resourceExhausted:    throw hresult_error(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_DEVICE_NO_RESOURCES), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_internalError:        throw hresult_error(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_INTERNAL_ERROR), message);
  }
  throw hresult_error(E_FAIL, message);
}
