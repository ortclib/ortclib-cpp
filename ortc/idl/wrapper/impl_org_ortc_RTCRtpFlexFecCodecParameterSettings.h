
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpFlexFecCodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpFlexFecCodecParameterSettings : public wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::FlexFECCodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpFlexFecCodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettings, WrapperType);
          RTCRtpFlexFecCodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpFlexFecCodecParameterSettings() noexcept;
          virtual ~RTCRtpFlexFecCodecParameterSettings() noexcept;

          // methods RTCRtpFlexFecCodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // methods RTCRtpFlexFecCodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings(wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettingsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

