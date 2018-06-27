
#pragma once

#include "types.h"

namespace webrtc
{
  enum class RTCErrorType;
}

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // (helper functions)
        //

        struct IEnum
        {
          static wrapper::org::webrtc::RTCErrorType toWrapper(::webrtc::RTCErrorType value) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

