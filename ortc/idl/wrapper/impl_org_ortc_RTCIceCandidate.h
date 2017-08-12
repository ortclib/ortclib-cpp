
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidate.h"

#include <ortc/IICETypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidate : public wrapper::org::ortc::RTCIceCandidate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETypes::Candidate, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceCandidate, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceCandidate, WrapperType);
          RTCIceCandidateWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCIceCandidate();
          virtual ~RTCIceCandidate();

          // methods RTCIceGathererCandidate
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCIceGathererCandidate
          virtual wrapper::org::ortc::RTCIceComponent get_component() override;
          virtual void wrapper_init_org_ortc_RTCIceCandidate() override;

          // properties RTCIceCandidate
          virtual String get_interfaceType() override;
          virtual String get_foundation() override;
          virtual uint32_t get_priority() override;
          virtual uint32_t get_unfreezePriority() override;
          virtual wrapper::org::ortc::RTCIceProtocol get_protocol() override;
          virtual String get_ip() override;
          virtual uint16_t get_port() override;
          virtual wrapper::org::ortc::RTCIceCandidateType get_candidateType() override;
          virtual wrapper::org::ortc::RTCIceTcpCandidateType get_tcpType() override;
          virtual String get_relatedAddress() override;
          virtual uint16_t get_relatedPort() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
          static NativeTypePtr toNative(wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

