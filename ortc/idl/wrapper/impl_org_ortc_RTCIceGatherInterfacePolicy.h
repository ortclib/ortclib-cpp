
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

          RTCIceGatherInterfacePolicy() noexcept;
          virtual ~RTCIceGatherInterfacePolicy() noexcept;

          // methods RTCIceGatherInterfacePolicy
          virtual void wrapper_init_org_ortc_RTCIceGatherInterfacePolicy() noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceGatherInterfacePolicy(wrapper::org::ortc::RTCIceGatherInterfacePolicyPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceGatherInterfacePolicy(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

