// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtcpParameters.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtcpParameters : public wrapper::org::webRtc::RTCRtcpParameters
        {
          RTCRtcpParametersWeakPtr thisWeak_;

          RTCRtcpParameters() noexcept;
          virtual ~RTCRtcpParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtcpParameters() noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper
