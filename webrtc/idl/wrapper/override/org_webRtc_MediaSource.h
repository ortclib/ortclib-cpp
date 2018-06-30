
#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#pragma once

#include <wrapper/generated/types.h>

namespace wrapper {
  namespace org {
    namespace webRtc {

      struct MediaSource
      {
        static MediaSourcePtr wrapper_create() noexcept;
        virtual ~MediaSource() noexcept {}

        virtual void wrapper_init_org_webRtc_MediaSource() noexcept {}

        virtual AnyPtr get_source() noexcept = 0;
        virtual void set_source(AnyPtr value) noexcept = 0;
      };

    } // webRtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
