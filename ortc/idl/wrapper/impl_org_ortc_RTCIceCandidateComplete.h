
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidateComplete.h"

#include <ortc/IICETypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidateComplete : public wrapper::org::ortc::RTCIceCandidateComplete
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETypes::CandidateComplete, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceCandidateComplete, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceCandidateComplete, WrapperType);
          RTCIceCandidateCompleteWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCIceCandidateComplete() noexcept;
          virtual ~RTCIceCandidateComplete() noexcept;

          // methods RTCIceGathererCandidate
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCIceGathererCandidate
          wrapper::org::ortc::RTCIceComponent get_component() noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidateComplete() noexcept override;

          // properties RTCIceCandidateComplete
          bool get_complete() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          static NativeTypePtr toNative(wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

