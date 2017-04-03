
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

          RTCIceGathererCandidate();
          virtual ~RTCIceGathererCandidate();

          // methods RTCIceGathererCandidate
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCIceGathererCandidate
          virtual wrapper::org::ortc::RTCIceComponent get_component() override;

          static WrapperTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

