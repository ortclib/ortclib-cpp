
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCPeerConnectionIceEvent.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCPeerConnectionIceEvent : public wrapper::org::ortc::adapter::RTCPeerConnectionIceEvent
          {
            ZS_DECLARE_TYPEDEF_PTR(RTCPeerConnectionIceEvent, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCPeerConnectionIceEvent, WrapperType);
            RTCPeerConnectionIceEventWeakPtr thisWeak_;
            wrapper::org::ortc::adapter::RTCIceCandidatePtr candidate_;
            String uri_;

            RTCPeerConnectionIceEvent() noexcept;
            virtual ~RTCPeerConnectionIceEvent() noexcept;

            // properties RTCPeerConnectionIceEvent
            virtual wrapper::org::ortc::adapter::RTCIceCandidatePtr get_candidate() noexcept override;
            virtual String get_uri() noexcept override;

            static WrapperImplTypePtr toWrapper(
              ::ortc::adapter::IPeerConnectionTypes::ICECandidatePtr candidate,
              const String &uri
            ) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

