// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCAnswerOptions.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCAnswerOptions : public wrapper::org::webrtc::RTCAnswerOptions
        {
          RTCAnswerOptionsWeakPtr thisWeak_;

          RTCAnswerOptions() noexcept;
          virtual ~RTCAnswerOptions() noexcept;

          // methods RTCAnswerOptions
          void wrapper_init_org_webrtc_RTCAnswerOptions() noexcept override;
          void wrapper_init_org_webrtc_RTCAnswerOptions(wrapper::org::webrtc::RTCAnswerOptionsPtr source) noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper
