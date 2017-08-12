
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtlsParameters.h"

#include <ortc/IDTLSTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtlsParameters : public wrapper::org::ortc::RTCDtlsParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTLSTransportTypes::Parameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDtlsParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtlsParameters, WrapperType);
          RTCDtlsParametersWeakPtr thisWeak_;

          RTCDtlsParameters();
          virtual ~RTCDtlsParameters();

          // methods RTCDtlsParameters
          virtual void wrapper_init_org_ortc_RTCDtlsParameters() override;
          virtual void wrapper_init_org_ortc_RTCDtlsParameters(wrapper::org::ortc::RTCDtlsParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCDtlsParameters(wrapper::org::ortc::JsonPtr json) override;
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

