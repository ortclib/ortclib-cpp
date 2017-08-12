
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

          RTCRtpH264CodecParameterSettings();
          virtual ~RTCRtpH264CodecParameterSettings();

          // methods RTCRtpH264CodecCapabilityParameters
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // methods RTCRtpH264CodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecParameterSettings(wrapper::org::ortc::RTCRtpH264CodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

