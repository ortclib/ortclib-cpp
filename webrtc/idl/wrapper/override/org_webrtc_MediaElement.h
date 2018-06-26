
#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
#pragma once

#include <wrapper/generated/types.h>

namespace wrapper {
  namespace org {
    namespace webrtc {

      struct MediaElement
      {
        static MediaElementPtr wrapper_create() noexcept;
        virtual ~MediaElement() noexcept {}

        virtual void wrapper_init_org_webrtc_MediaElement() noexcept {}

        virtual AnyPtr get_element() noexcept = 0;
        virtual void set_element(AnyPtr value) noexcept = 0;
      };

    } // webrtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
