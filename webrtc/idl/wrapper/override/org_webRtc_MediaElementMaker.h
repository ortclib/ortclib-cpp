
#pragma once

#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER

#include "types.h"

namespace wrapper {
  namespace org {
    namespace webRtc {

      struct MediaElementMaker
      {
        virtual ~MediaElementMaker() noexcept {}

#if 0
        static wrapper::org::webRtc::MediaElementPtr bind(AnyPtr element) noexcept;
        static AnyPtr extract(wrapper::org::webRtc::MediaElementPtr element) noexcept;
#endif //0
      };

    } // webRtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER
