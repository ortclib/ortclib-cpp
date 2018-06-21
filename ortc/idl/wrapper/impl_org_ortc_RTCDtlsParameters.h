
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

          RTCDtlsParameters() noexcept;
          virtual ~RTCDtlsParameters() noexcept;

          // methods RTCDtlsParameters
          void wrapper_init_org_ortc_RTCDtlsParameters() noexcept override;
          void wrapper_init_org_ortc_RTCDtlsParameters(wrapper::org::ortc::RTCDtlsParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCDtlsParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

