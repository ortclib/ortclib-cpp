
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpOpusCodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpOpusCodecParameterSettings : public wrapper::org::ortc::RTCRtpOpusCodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::OpusCodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpOpusCodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpOpusCodecParameterSettings, WrapperType);
          RTCRtpOpusCodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpOpusCodecParameterSettings() noexcept;
          virtual ~RTCRtpOpusCodecParameterSettings() noexcept;

          // methods RTCRtpOpusCodecParameterSettings
          void wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings() noexcept override;
          void wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings(wrapper::org::ortc::RTCRtpOpusCodecParameterSettingsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept override;
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

