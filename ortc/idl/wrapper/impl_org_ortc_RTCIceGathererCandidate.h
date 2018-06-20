
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGathererCandidate.h"

#include <ortc/IICETypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGathererCandidate : public wrapper::org::ortc::RTCIceGathererCandidate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETypes::GatherCandidate, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGathererCandidate, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGathererCandidate, WrapperType);
          RTCIceGathererCandidateWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCIceGathererCandidate() noexcept;
          virtual ~RTCIceGathererCandidate() noexcept;

          // methods RTCIceGathererCandidate
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCIceGathererCandidate
          virtual wrapper::org::ortc::RTCIceComponent get_component() noexcept override;

          static WrapperTypePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

