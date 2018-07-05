
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/WebRtc/VideoDeviceInfo.g.h"
#include <wrapper/generated/org_webRtc_VideoDeviceInfo.h>

namespace winrt {
  namespace Org {
    namespace WebRtc {
      namespace implementation {


        struct VideoDeviceInfo : VideoDeviceInfoT<VideoDeviceInfo>
        {
          // internal
          wrapper::org::webRtc::VideoDeviceInfoPtr native_;

          struct WrapperCreate {};
          VideoDeviceInfo(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > ToCppWinrtImpl(wrapper::org::webRtc::VideoDeviceInfoPtr value);
          static winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > ToCppWinrtImpl(Org::WebRtc::VideoDeviceInfo const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > ToCppWinrtImpl(Org::WebRtc::IVideoDeviceInfo const & value);

          // ToCppWinrt
          static Org::WebRtc::VideoDeviceInfo ToCppWinrt(wrapper::org::webRtc::VideoDeviceInfoPtr value);
          static Org::WebRtc::VideoDeviceInfo ToCppWinrt(Org::WebRtc::VideoDeviceInfo const & value);
          static Org::WebRtc::VideoDeviceInfo ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value);
          static Org::WebRtc::VideoDeviceInfo ToCppWinrt(Org::WebRtc::IVideoDeviceInfo const & value);

          // ToCppWinrtInterface
          static Org::WebRtc::IVideoDeviceInfo ToCppWinrtInterface(wrapper::org::webRtc::VideoDeviceInfoPtr value);
          static Org::WebRtc::IVideoDeviceInfo ToCppWinrtInterface(Org::WebRtc::VideoDeviceInfo const & value);
          static Org::WebRtc::IVideoDeviceInfo ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value);
          static Org::WebRtc::IVideoDeviceInfo ToCppWinrtInterface(Org::WebRtc::IVideoDeviceInfo const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::VideoDeviceInfoPtr FromCppWinrt(wrapper::org::webRtc::VideoDeviceInfoPtr value);
          static wrapper::org::webRtc::VideoDeviceInfoPtr FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value);
          static wrapper::org::webRtc::VideoDeviceInfoPtr FromCppWinrt(Org::WebRtc::VideoDeviceInfo const & value);
          static wrapper::org::webRtc::VideoDeviceInfoPtr FromCppWinrt(Org::WebRtc::IVideoDeviceInfo const & value);




        public:
          VideoDeviceInfo();
          /// <summary>
          /// Cast from Org::WebRtc::IVideoDeviceInfo to VideoDeviceInfo
          /// </summary>
          static Org::WebRtc::VideoDeviceInfo CastFromIVideoDeviceInfo(Org::WebRtc::IVideoDeviceInfo const & value);

          Windows::Devices::Enumeration::DeviceInformation Info();
        };

      } // namepsace implementation

      namespace factory_implementation {

        struct VideoDeviceInfo : VideoDeviceInfoT<VideoDeviceInfo, implementation::VideoDeviceInfo>
        {
        };

      } // namespace factory_implementation

    } // namespace WebRtc
  } // namespace Org
} // namespace winrt

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO
