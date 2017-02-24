// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpVp8CodecParameterSettings.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpVp8CodecParameterSettings : public wrapper::org::ortc::RTCRtpVp8CodecParameterSettings
        {
          RTCRtpVp8CodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpVp8CodecParameterSettings();
          virtual ~RTCRtpVp8CodecParameterSettings();

          // methods RTCRtpVp8CodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // methods RTCRtpVp8CodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings(wrapper::org::ortc::RTCRtpVp8CodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

