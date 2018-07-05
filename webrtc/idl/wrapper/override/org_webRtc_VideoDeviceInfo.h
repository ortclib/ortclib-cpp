
#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO
#pragma once

#include "types.h"

namespace wrapper {
  namespace org {
    namespace webRtc {

      struct VideoDeviceInfo
      {
        static VideoDeviceInfoPtr wrapper_create() noexcept;
        virtual ~VideoDeviceInfo() noexcept {}

        virtual AnyPtr get_info() noexcept = 0;
      };

    } // webRtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO
