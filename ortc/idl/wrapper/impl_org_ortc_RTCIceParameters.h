
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceParameters.h"

#include <ortc/IICETypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceParameters : public wrapper::org::ortc::RTCIceParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETypes::Parameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceParameters, WrapperType);
          RTCIceParametersWeakPtr thisWeak_;

          RTCIceParameters() noexcept;
          virtual ~RTCIceParameters() noexcept;

          // methods RTCIceParameters
          void wrapper_init_org_ortc_RTCIceParameters() noexcept override;
          void wrapper_init_org_ortc_RTCIceParameters(wrapper::org::ortc::RTCIceParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

