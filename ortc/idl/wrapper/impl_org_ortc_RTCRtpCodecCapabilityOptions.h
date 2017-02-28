
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCodecCapabilityOptions.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCodecCapabilityOptions : public wrapper::org::ortc::RTCRtpCodecCapabilityOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(zsLib::Any, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCodecCapabilityOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCodecCapabilityOptions, WrapperType);
          RTCRtpCodecCapabilityOptionsWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCRtpCodecCapabilityOptions();
          virtual ~RTCRtpCodecCapabilityOptions();

          static WrapperTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

