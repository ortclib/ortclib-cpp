
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpRtxParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpRtxParameters : public wrapper::org::ortc::RTCRtpRtxParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTXParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpRtxParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpRtxParameters, WrapperType);
          RTCRtpRtxParametersWeakPtr thisWeak_;

          RTCRtpRtxParameters();
          virtual ~RTCRtpRtxParameters();

          // methods RTCRtpRtxParameters
          virtual void wrapper_init_org_ortc_RTCRtpRtxParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxParameters(wrapper::org::ortc::RTCRtpRtxParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxParameters(wrapper::org::ortc::JsonPtr json) override;
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

