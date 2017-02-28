
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

          RTCRtpVp8CodecParameterSettings();
          virtual ~RTCRtpVp8CodecParameterSettings();

          // methods RTCRtpVp8CodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // methods RTCRtpVp8CodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings(wrapper::org::ortc::RTCRtpVp8CodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

