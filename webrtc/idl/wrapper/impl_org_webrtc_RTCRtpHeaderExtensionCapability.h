// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCRtpHeaderExtensionCapability.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCRtpHeaderExtensionCapability : public wrapper::org::webrtc::RTCRtpHeaderExtensionCapability
        {
          RTCRtpHeaderExtensionCapabilityWeakPtr thisWeak_;

          RTCRtpHeaderExtensionCapability() noexcept;
          virtual ~RTCRtpHeaderExtensionCapability() noexcept;
          void wrapper_init_org_webrtc_RTCRtpHeaderExtensionCapability() noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper
