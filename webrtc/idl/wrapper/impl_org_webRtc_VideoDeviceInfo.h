
#pragma once

#include "types.h"
#include "generated/org_webRtc_VideoDeviceInfo.h"

#ifdef CPPWINRT_VERSION
#include <winrt/Windows.Devices.Enumeration.h>
#endif // CPPWINRT_VERSION

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct VideoDeviceInfo : public wrapper::org::webRtc::VideoDeviceInfo
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::VideoDeviceInfo, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoDeviceInfo, WrapperImplType);

          AnyPtr info_ {};

          VideoDeviceInfoWeakPtr thisWeak_;

          VideoDeviceInfo() noexcept;
          virtual ~VideoDeviceInfo() noexcept;

          // properties VideoDeviceInfo
          AnyPtr get_info() noexcept override;

#ifdef WINUWP
#ifdef __cplusplus_winrt
          ZS_NO_DISCARD() static wrapper::org::webRtc::VideoDeviceInfoPtr toWrapper(Windows::Devices::Enumeration::DeviceInformation^ info) noexcept;
          ZS_NO_DISCARD() static Windows::Devices::Enumeration::DeviceInformation^ toNative_cx(wrapper::org::webRtc::VideoDeviceInfoPtr info) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
          ZS_NO_DISCARD() static wrapper::org::webRtc::VideoDeviceInfoPtr toWrapper(winrt::Windows::Devices::Enumeration::DeviceInformation const & info) noexcept;
          ZS_NO_DISCARD() static winrt::Windows::Devices::Enumeration::DeviceInformation toNative_winrt(wrapper::org::webRtc::VideoDeviceInfoPtr info) noexcept;
#endif // CPPWINRT_VERSION
#else
#ifdef _WIN32
#endif //_WIN32
#endif //WINUWP

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

