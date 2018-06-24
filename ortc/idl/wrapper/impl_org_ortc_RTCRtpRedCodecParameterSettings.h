
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpRedCodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpRedCodecParameterSettings : public wrapper::org::ortc::RTCRtpRedCodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::REDCodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpRedCodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpRedCodecParameterSettings, WrapperType);
          RTCRtpRedCodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpRedCodecParameterSettings() noexcept;
          virtual ~RTCRtpRedCodecParameterSettings() noexcept;

          // methods RTCRtpRedCodecParameterSettings
          void wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings() noexcept override;
          void wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings(wrapper::org::ortc::RTCRtpRedCodecParameterSettingsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

