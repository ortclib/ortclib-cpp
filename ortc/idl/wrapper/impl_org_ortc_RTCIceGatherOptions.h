
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

          RTCIceGatherOptions();
          virtual ~RTCIceGatherOptions();

          // methods RTCIceGatherOptions
          virtual void wrapper_init_org_ortc_RTCIceGatherOptions() override;
          virtual void wrapper_init_org_ortc_RTCIceGatherOptions(wrapper::org::ortc::RTCIceGatherOptionsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceGatherOptions(wrapper::org::ortc::JsonPtr json) override;
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

