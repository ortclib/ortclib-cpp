

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/org_webrtc_RTCError.h>

#include <zsLib/types.h>
#include <zsLib/String.h>

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
  if (!error) throw ref new Platform::FailureException(L"No error object provided.");

  auto message = Internal::Helper::ToCx_String(error->get_message());

  switch (error->get_type())
  {
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_none:                 break;
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedOperation: throw ref new Platform::Exception(E_ILLEGAL_METHOD_CALL, message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_unsupportedParameter: throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_UNSUPPORTED_TYPE), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidParameter:     throw ref new Platform::InvalidArgumentException(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidRange:         throw ref new Platform::OutOfBoundsException(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_syntaxError:          throw ref new Platform::Exception(MK_E_SYNTAX, message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidState:         throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_INVALID_STATE), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_invalidModification:  throw ref new Platform::ChangedStateException(message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_networkError:         throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_UNEXP_NET_ERR), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_resourceExhausted:    throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_DEVICE_NO_RESOURCES), message);
    case wrapper::org::webRtc::RTCErrorType::RTCErrorType_internalError:        throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_INTERNAL_ERROR), message);
  }
  throw ref new Platform::FailureException(message);
}
