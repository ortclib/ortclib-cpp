
#ifdef WINUWP

#ifdef __cplusplus_winrt
#include <windows.devices.enumeration.h>
#endif //__cplusplus_winrt

#ifdef __has_include
#if __has_include(<winrt/Windows.Devices.Enumeration.h>)
#include <winrt/Windows.Devices.Enumeration.h>
#endif //__has_include(<winrt/Windows.Devices.Enumeration.h>)
#endif //__has_include

#endif //WINUWP

#include "impl_org_webRtc_VideoDeviceInfo.h"
#include "impl_org_webRtc_helpers.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoDeviceInfo::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

#ifdef WINUWP
#ifdef __cplusplus_winrt

namespace wrapper { namespace impl { namespace org { namespace webRtc {
        ZS_DECLARE_STRUCT_PTR(VideoDeviceInfoWrapperAnyCx);

        struct VideoDeviceInfoWrapperAnyCx : Any
        {
          Windows::Devices::Enumeration::DeviceInformation^ info_ {nullptr};
        };
} } } }

#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

namespace wrapper { namespace impl { namespace org { namespace webRtc {
        ZS_DECLARE_STRUCT_PTR(VideoDeviceInfoWrapperAnyWinrt);

        struct VideoDeviceInfoWrapperAnyWinrt : Any
        {
          winrt::Windows::Devices::Enumeration::DeviceInformation info_{nullptr};
        };
} } } }

#endif // CPPWINRT_VERSION

#else


namespace wrapper { namespace impl { namespace org { namespace webRtc {
        ZS_DECLARE_STRUCT_PTR(VideoDeviceInfoWrapperAny);

        struct VideoDeviceInfoWrapperAny : Any
        {
          void *ptr {};
        };
} } } }

#endif //WINUWP

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoDeviceInfo::VideoDeviceInfo() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoDeviceInfoPtr wrapper::org::webRtc::VideoDeviceInfo::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::VideoDeviceInfo>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoDeviceInfo::~VideoDeviceInfo() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
AnyPtr wrapper::impl::org::webRtc::VideoDeviceInfo::get_info() noexcept
{
  return info_;
}



#ifdef WINUWP
#ifdef __cplusplus_winrt

wrapper::org::webRtc::VideoDeviceInfoPtr wrapper::impl::org::webRtc::VideoDeviceInfo::toWrapper(Windows::Devices::Enumeration::DeviceInformation^ info) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webRtc::VideoDeviceInfoWrapperAnyCx>() };
  any->info_ = info;
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->info_ = any;
  return result;
}

Windows::Devices::Enumeration::DeviceInformation^ wrapper::impl::org::webRtc::VideoDeviceInfo::toNative_cx(wrapper::org::webRtc::VideoDeviceInfoPtr info) noexcept
{
  if (!info) return nullptr;
  AnyPtr any = info->get_info();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webRtc::VideoDeviceInfoWrapperAnyCx, any);
  if (!castedAny) {
#ifdef CPPWINRT_VERSION
    auto result = toNative_winrt(info);
    if (result) return WRAPPER_TO_CX(Windows::Devices::Enumeration::DeviceInformation, result);
#endif //CPPWINRT_VERSION
    return nullptr;
  }
  return castedAny->info_;
}
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

wrapper::org::webRtc::VideoDeviceInfoPtr wrapper::impl::org::webRtc::VideoDeviceInfo::toWrapper(winrt::Windows::Devices::Enumeration::DeviceInformation const & info) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webRtc::VideoDeviceInfoWrapperAnyWinrt>() };
  any->info_ = info;
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->info_ = any;
  return result;
}

winrt::Windows::Devices::Enumeration::DeviceInformation wrapper::impl::org::webRtc::VideoDeviceInfo::toNative_winrt(wrapper::org::webRtc::VideoDeviceInfoPtr info) noexcept
{
  if (!info) return nullptr;
  AnyPtr any = info->get_info();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webRtc::VideoDeviceInfoWrapperAnyWinrt, any);
  if (!castedAny) {
#ifdef __cplusplus_winrt
    auto result = toNative_cx(info);
    if (result) return WRAPPER_FROM_CX(winrt::Windows::Devices::Enumeration::DeviceInformation, result);
#endif //__cplusplus_winrt
    return nullptr;
  }
  return castedAny->info_;
}

#endif // CPPWINRT_VERSION

#else

#ifdef _WIN32
#endif //_WIN32

#endif //WINUWP
