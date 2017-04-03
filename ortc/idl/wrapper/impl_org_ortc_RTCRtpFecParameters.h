
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpFecParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpFecParameters : public wrapper::org::ortc::RTCRtpFecParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::FECParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpFecParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpFecParameters, WrapperType);
          RTCRtpFecParametersWeakPtr thisWeak_;

          RTCRtpFecParameters();
          virtual ~RTCRtpFecParameters();

          // methods RTCRtpFecParameters
          virtual void wrapper_init_org_ortc_RTCRtpFecParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpFecParameters(wrapper::org::ortc::RTCRtpFecParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpFecParameters(wrapper::org::ortc::JsonPtr json) override;
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

