
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

          RTCIceCandidatePair();
          virtual ~RTCIceCandidatePair();

          // methods RTCIceCandidatePair
          virtual void wrapper_init_org_ortc_RTCIceCandidatePair() override;
          virtual void wrapper_init_org_ortc_RTCIceCandidatePair(wrapper::org::ortc::RTCIceCandidatePairPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceCandidatePair(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCIceCandidatePair
          virtual wrapper::org::ortc::RTCIceCandidatePtr get_local() override;
          virtual void set_local(wrapper::org::ortc::RTCIceCandidatePtr value) override;
          virtual wrapper::org::ortc::RTCIceCandidatePtr get_remote() override;
          virtual void set_remote(wrapper::org::ortc::RTCIceCandidatePtr value) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

