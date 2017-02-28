
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpParameters : public wrapper::org::ortc::RTCRtpParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::Parameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpParameters, WrapperType);
          RTCRtpParametersWeakPtr thisWeak_;

          RTCRtpParameters();
          virtual ~RTCRtpParameters();

          // methods RTCRtpParameters
          virtual void wrapper_init_org_ortc_RTCRtpParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpParameters(wrapper::org::ortc::RTCRtpParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpParameters(wrapper::org::ortc::JsonPtr json) override;
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

