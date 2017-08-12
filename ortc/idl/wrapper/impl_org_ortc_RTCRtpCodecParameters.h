
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCodecParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCodecParameters : public wrapper::org::ortc::RTCRtpCodecParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::CodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCodecParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCodecParameters, WrapperType);
          RTCRtpCodecParametersWeakPtr thisWeak_;

          RTCRtpCodecParameters();
          virtual ~RTCRtpCodecParameters();

          // methods RTCRtpCodecParameters
          virtual void wrapper_init_org_ortc_RTCRtpCodecParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpCodecParameters(wrapper::org::ortc::RTCRtpCodecParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpCodecParameters(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

