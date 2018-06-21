
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidatePair.h"

#include <ortc/IICETransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidatePair : public wrapper::org::ortc::RTCIceCandidatePair
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransportTypes::CandidatePair, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceCandidatePair, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceCandidatePair, WrapperType);
          RTCIceCandidatePairWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCIceCandidatePair() noexcept;
          virtual ~RTCIceCandidatePair() noexcept;

          // methods RTCIceCandidatePair
          void wrapper_init_org_ortc_RTCIceCandidatePair() noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidatePair(wrapper::org::ortc::RTCIceCandidatePairPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidatePair(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCIceCandidatePair
          wrapper::org::ortc::RTCIceCandidatePtr get_local() noexcept override;
          void set_local(wrapper::org::ortc::RTCIceCandidatePtr value) noexcept override;
          wrapper::org::ortc::RTCIceCandidatePtr get_remote() noexcept override;
          void set_remote(wrapper::org::ortc::RTCIceCandidatePtr value) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

