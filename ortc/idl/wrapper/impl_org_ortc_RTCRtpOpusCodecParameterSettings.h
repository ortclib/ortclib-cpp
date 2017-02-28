
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

          RTCRtpOpusCodecParameterSettings();
          virtual ~RTCRtpOpusCodecParameterSettings();

          // methods RTCRtpOpusCodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings(wrapper::org::ortc::RTCRtpOpusCodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

