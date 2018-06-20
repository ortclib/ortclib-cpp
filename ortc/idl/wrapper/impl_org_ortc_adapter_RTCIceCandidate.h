
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
            virtual void wrapper_init_org_ortc_adapter_RTCIceCandidate() noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr source) noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::JsonPtr json) noexcept override;
            virtual String toSdp() noexcept override;
            virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
            virtual String hash() noexcept override;

            // properties RTCIceCandidate
            virtual String get_sdpMid() noexcept override;
            virtual void set_sdpMid(String value) noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

