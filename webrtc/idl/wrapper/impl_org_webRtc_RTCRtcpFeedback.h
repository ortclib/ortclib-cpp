// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtcpFeedback.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtcpFeedback : public wrapper::org::webRtc::RTCRtcpFeedback
        {
          RTCRtcpFeedbackWeakPtr thisWeak_;

          RTCRtcpFeedback() noexcept;
          virtual ~RTCRtcpFeedback() noexcept;
          void wrapper_init_org_webRtc_RTCRtcpFeedback() noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

