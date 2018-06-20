
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpH264CodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpH264CodecParameterSettings : public wrapper::org::ortc::RTCRtpH264CodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::H264CodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpH264CodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpH264CodecParameterSettings, WrapperType);
          RTCRtpH264CodecParameterSettingsWeakPtr thisWeak_;

          RTCRtpH264CodecParameterSettings() noexcept;
          virtual ~RTCRtpH264CodecParameterSettings() noexcept;

          // methods RTCRtpH264CodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // methods RTCRtpH264CodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecParameterSettings() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecParameterSettings(wrapper::org::ortc::RTCRtpH264CodecParameterSettingsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

