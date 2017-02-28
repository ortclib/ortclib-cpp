
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpRtxCodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpRtxCodecParameterSettings : public wrapper::org::ortc::RTCRtpRtxCodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTXCodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpRtxCodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpRtxCodecParameterSettings, WrapperType);
          RTCRtpRtxCodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpRtxCodecParameterSettings();
          virtual ~RTCRtpRtxCodecParameterSettings();

          // methods RTCRtpRtxCodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // methods RTCRtpRtxCodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings(wrapper::org::ortc::RTCRtpRtxCodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

