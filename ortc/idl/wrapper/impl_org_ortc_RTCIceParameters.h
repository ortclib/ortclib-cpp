
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

          RTCIceParameters();
          virtual ~RTCIceParameters();

          // methods RTCIceParameters
          virtual void wrapper_init_org_ortc_RTCIceParameters() override;
          virtual void wrapper_init_org_ortc_RTCIceParameters(wrapper::org::ortc::RTCIceParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceParameters(wrapper::org::ortc::JsonPtr json) override;
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

