
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpEncodingParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpEncodingParameters : public wrapper::org::ortc::RTCRtpEncodingParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::EncodingParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpEncodingParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpEncodingParameters, WrapperType);
          RTCRtpEncodingParametersWeakPtr thisWeak_;

          RTCRtpEncodingParameters();
          virtual ~RTCRtpEncodingParameters();

          // methods RTCRtpEncodingParameters
          virtual void wrapper_init_org_ortc_RTCRtpEncodingParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpEncodingParameters(wrapper::org::ortc::RTCRtpEncodingParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpEncodingParameters(wrapper::org::ortc::JsonPtr json) override;
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

