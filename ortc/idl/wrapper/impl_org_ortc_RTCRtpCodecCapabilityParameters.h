
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCodecCapabilityParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCodecCapabilityParameters : public wrapper::org::ortc::RTCRtpCodecCapabilityParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(zsLib::Any, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCodecCapabilityParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCodecCapabilityParameters, WrapperType);
          RTCRtpCodecCapabilityParametersWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCRtpCodecCapabilityParameters();
          virtual ~RTCRtpCodecCapabilityParameters();

          static WrapperTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

