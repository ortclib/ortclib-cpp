
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCPeerConnectionIceErrorEvent.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCPeerConnectionIceErrorEvent : public wrapper::org::ortc::adapter::RTCPeerConnectionIceErrorEvent
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::ICECandidateErrorEvent, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCPeerConnectionIceErrorEvent, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCPeerConnectionIceErrorEvent, WrapperType);
            RTCPeerConnectionIceErrorEventWeakPtr thisWeak_;
            NativeTypePtr native_;

            RTCPeerConnectionIceErrorEvent();
            virtual ~RTCPeerConnectionIceErrorEvent();

            // properties RTCPeerConnectionIceErrorEvent
            virtual wrapper::org::ortc::adapter::RTCIceCandidatePtr get_hostCandidate() override;
            virtual String get_url() override;
            virtual Optional< uint16_t > get_errorCode() override;
            virtual String get_errorText() override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

