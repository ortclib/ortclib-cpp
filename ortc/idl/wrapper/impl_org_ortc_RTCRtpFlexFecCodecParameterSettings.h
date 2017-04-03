
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

          RTCRtpFlexFecCodecParameterSettings();
          virtual ~RTCRtpFlexFecCodecParameterSettings();

          // methods RTCRtpFlexFecCodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // methods RTCRtpFlexFecCodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings(wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

