
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

          RTCRtpRedCodecParameterSettings();
          virtual ~RTCRtpRedCodecParameterSettings();

          // methods RTCRtpRedCodecParameterSettings
          virtual void wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings() override;
          virtual void wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings(wrapper::org::ortc::RTCRtpRedCodecParameterSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings(wrapper::org::ortc::JsonPtr json) override;
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

