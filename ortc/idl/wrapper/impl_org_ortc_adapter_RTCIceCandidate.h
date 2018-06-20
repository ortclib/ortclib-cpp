
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCIceCandidate.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCIceCandidate : public wrapper::org::ortc::adapter::RTCIceCandidate
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::ICECandidate, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCIceCandidate, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCIceCandidate, WrapperType);
            RTCIceCandidateWeakPtr thisWeak_;

            RTCIceCandidate() noexcept;
            virtual ~RTCIceCandidate() noexcept;

            // methods RTCIceCandidate
            void wrapper_init_org_ortc_adapter_RTCIceCandidate() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::JsonPtr json) noexcept(false) override;
            wrapper::org::ortc::adapter::RTCIceCandidatePtr fromSdpStringWithMLineIndex(
              String sdp,
              uint64_t mlineIndex
              ) noexcept(false) override;
            wrapper::org::ortc::adapter::RTCIceCandidatePtr fromSdpStringWithMid(
              String sdp,
              String mid
              ) noexcept(false) override;
            String toSdp() noexcept override;
            wrapper::org::ortc::JsonPtr toJson() noexcept override;
            String hash() noexcept override;

            // properties RTCIceCandidate
            String get_sdpMid() noexcept override;
            void set_sdpMid(String value) noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

