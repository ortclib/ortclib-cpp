
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

          RTCRtpCodecParameterSettings() noexcept;
          virtual ~RTCRtpCodecParameterSettings() noexcept;

          static WrapperTypePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

