
#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/org_ortc_Error.h>
#include <wrapper/generated/org_ortc_RTCIdentityError.h>
#include <wrapper/generated/org_ortc_OverconstrainedError.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/types.h>
#include <zsLib/String.h>

#include <sstream>

#define CUSTOM_MAKE_HRESULT(xCustomerBit, xFacility, xErrorCode) ((HRESULT)(xErrorCode) <= 0 ? ((HRESULT)(xErrorCode)) : ((HRESULT) (((xErrorCode) & 0x0000FFFF) | ((xFacility) << 16) | ((xCustomerBit) << 29) | 0x80000000)))

typedef Internal::Helper::Throwers ThrowersType;

ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

//-----------------------------------------------------------------------------
ThrowersType &ThrowersType::singleton() noexcept
{
  static ThrowersType result{};
  return result;
}

//-----------------------------------------------------------------------------
void ThrowersType::customThrow(wrapper::org::ortc::OverconstrainedErrorPtr error) noexcept(false)
{
  if (!error) throw ref new Platform::FailureException(L"No error object provided.");

  std::stringstream ss;
  ss << error->name;
  if (error->name.hasData()) ss << ": ";
  ss << error->constraint;
  if (error->message.hasData()) ss << " \"";
  ss << error->message;
  if (error->message.hasData()) ss << "\"";

  auto message = Internal::Helper::ToCx_String(String(ss.str()));

  throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, ERROR_DS_CONSTRAINT_VIOLATION), message);
}

//-----------------------------------------------------------------------------
void ThrowersType::customThrow(wrapper::org::ortc::RTCIdentityErrorPtr error) noexcept(false)
{
  if (!error) throw ref new Platform::FailureException(L"No error object provided.");

  std::stringstream ss;
  if (error->protocol.hasData()) ss << "\"";
  ss << error->protocol;
  if (error->protocol.hasData()) ss << "\";";

  if (error->loginUrl.hasData()) ss << "\"";
  ss << error->loginUrl;
  if (error->loginUrl.hasData()) ss << "\";";

  if (error->idp.hasData()) ss << "\"";
  ss << error->idp;
  if (error->idp.hasData()) ss << "\";";

  auto message = Internal::Helper::ToCx_String(String(ss.str()));

  throw ref new Platform::Exception(CUSTOM_MAKE_HRESULT(1, FACILITY_ITF, RPC_S_INVALID_AUTH_IDENTITY));
}

//-----------------------------------------------------------------------------
void ThrowersType::customThrow(wrapper::org::ortc::ErrorPtr error) noexcept(false)
{
  if (!error) throw ref new Platform::FailureException(L"No error object provided.");

  auto message = Internal::Helper::ToCx_String(String(error->name + (error->name.hasData() && error->reason.hasData() ? ": " : "") + error->reason));

  HRESULT extractedCode = (HTTP_E_STATUS_BAD_REQUEST & 0x0000FFFF);
  HRESULT otherParts = (HTTP_E_STATUS_BAD_REQUEST & 0xFFFF0000);

  extractedCode -= 400;
  extractedCode += static_cast<uint32_t>(error->errorCode);

  HRESULT mergedCode = extractedCode | otherParts;

  throw ref new Platform::Exception(mergedCode, message);
}
