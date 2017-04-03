
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGatherInterfacePolicy.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGatherInterfacePolicy : public wrapper::org::ortc::RTCIceGatherInterfacePolicy
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGathererTypes::InterfacePolicy, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGatherInterfacePolicy, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGatherInterfacePolicy, WrapperType);

          RTCIceGatherInterfacePolicyWeakPtr thisWeak_;

          RTCIceGatherInterfacePolicy();
          virtual ~RTCIceGatherInterfacePolicy();

          // methods RTCIceGatherInterfacePolicy
          virtual void wrapper_init_org_ortc_RTCIceGatherInterfacePolicy() override;
          virtual void wrapper_init_org_ortc_RTCIceGatherInterfacePolicy(wrapper::org::ortc::RTCIceGatherInterfacePolicyPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceGatherInterfacePolicy(wrapper::org::ortc::JsonPtr json) override;
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

