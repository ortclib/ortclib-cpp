
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtcpParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtcpParameters : public wrapper::org::ortc::RTCRtcpParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTCPParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtcpParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtcpParameters, WrapperType);
          RTCRtcpParametersWeakPtr thisWeak_;

          RTCRtcpParameters();
          virtual ~RTCRtcpParameters();

          // methods RTCRtcpParameters
          virtual void wrapper_init_org_ortc_RTCRtcpParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtcpParameters(wrapper::org::ortc::RTCRtcpParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtcpParameters(wrapper::org::ortc::JsonPtr json) override;
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

