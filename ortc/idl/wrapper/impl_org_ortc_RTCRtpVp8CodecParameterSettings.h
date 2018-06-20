
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpVp8CodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpVp8CodecParameterSettings : public wrapper::org::ortc::RTCRtpVp8CodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::VP8CodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpVp8CodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpVp8CodecParameterSettings, WrapperType);
          RTCRtpVp8CodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpVp8CodecParameterSettings() noexcept;
          virtual ~RTCRtpVp8CodecParameterSettings() noexcept;

          // methods RTCRtpVp8CodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // methods RTCRtpVp8CodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings(wrapper::org::ortc::RTCRtpVp8CodecParameterSettingsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

