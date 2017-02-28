
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

            RTCIceCandidate();
            virtual ~RTCIceCandidate();

            // methods RTCIceCandidate
            virtual void wrapper_init_org_ortc_adapter_RTCIceCandidate() override;
            virtual void wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::JsonPtr json) override;
            virtual String toSdp() override;
            virtual wrapper::org::ortc::JsonPtr toJson() override;
            virtual String hash() override;

            // properties RTCIceCandidate
            virtual String get_sdpMid() override;
            virtual void set_sdpMid(String value) override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

