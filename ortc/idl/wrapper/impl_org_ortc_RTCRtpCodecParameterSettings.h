
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCodecParameterSettings.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCodecParameterSettings : public wrapper::org::ortc::RTCRtpCodecParameterSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(zsLib::Any, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCodecParameterSettings, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCodecParameterSettings, WrapperType);
          RTCRtpCodecParameterSettingsWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCRtpCodecParameterSettings();
          virtual ~RTCRtpCodecParameterSettings();

          static WrapperTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

