// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webrtc_VideoCapturerInputSize.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct VideoCapturerInputSize : public wrapper::org::webrtc::VideoCapturerInputSize
        {
          VideoCapturerInputSizeWeakPtr thisWeak_;

          VideoCapturerInputSize() noexcept;
          virtual ~VideoCapturerInputSize() noexcept;
          void wrapper_init_org_webrtc_VideoCapturerInputSize() noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper
