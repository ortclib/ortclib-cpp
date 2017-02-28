
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

          RTCIceCandidateComplete();
          virtual ~RTCIceCandidateComplete();

          // methods RTCIceGathererCandidate
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCIceGathererCandidate
          virtual wrapper::org::ortc::RTCIceComponent get_component() override;
          virtual void wrapper_init_org_ortc_RTCIceCandidateComplete() override;

          // properties RTCIceCandidateComplete
          virtual bool get_complete() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
          static NativeTypePtr toNative(wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

