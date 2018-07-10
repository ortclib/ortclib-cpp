

#ifndef CX_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_VideoDeviceInfo.h>

namespace Org {
  namespace WebRtc {



    public ref class VideoDeviceInfo sealed
    {
    internal:
      wrapper::org::webRtc::VideoDeviceInfoPtr native_;

      struct WrapperCreate {};
      VideoDeviceInfo(const WrapperCreate &) {}

      static VideoDeviceInfo^ ToCx(wrapper::org::webRtc::VideoDeviceInfoPtr value);
      static wrapper::org::webRtc::VideoDeviceInfoPtr FromCx(VideoDeviceInfo^ value);



    public:
      property Windows::Devices::Enumeration::DeviceInformation^ Info { Windows::Devices::Enumeration::DeviceInformation^ get(); }

    };

  } // namespace WebRtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO
