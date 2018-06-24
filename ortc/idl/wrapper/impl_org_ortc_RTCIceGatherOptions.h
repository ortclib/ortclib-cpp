
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGatherOptions.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGatherOptions : public wrapper::org::ortc::RTCIceGatherOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGathererTypes::Options, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGatherOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGatherOptions, WrapperType);
          RTCIceGatherOptionsWeakPtr thisWeak_;

          RTCIceGatherOptions() noexcept;
          virtual ~RTCIceGatherOptions() noexcept;

          // methods RTCIceGatherOptions
          void wrapper_init_org_ortc_RTCIceGatherOptions() noexcept override;
          void wrapper_init_org_ortc_RTCIceGatherOptions(wrapper::org::ortc::RTCIceGatherOptionsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceGatherOptions(wrapper::org::ortc::JsonPtr json) noexcept override;
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

