
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

          RTCRtpRtxCodecParameterSettings() noexcept;
          virtual ~RTCRtpRtxCodecParameterSettings() noexcept;

          // methods RTCRtpRtxCodecCapabilityParameters
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // methods RTCRtpRtxCodecParameterSettings
          void wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings() noexcept override;
          void wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings(wrapper::org::ortc::RTCRtpRtxCodecParameterSettingsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

