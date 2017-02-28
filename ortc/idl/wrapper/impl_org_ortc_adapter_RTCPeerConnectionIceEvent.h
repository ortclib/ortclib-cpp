
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCPeerConnectionIceEvent.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCPeerConnectionIceEvent : public wrapper::org::ortc::adapter::RTCPeerConnectionIceEvent
          {
            RTCPeerConnectionIceEventWeakPtr thisWeak_;
            wrapper::org::ortc::adapter::RTCIceCandidatePtr candidate_;
            String uri_;

            RTCPeerConnectionIceEvent();
            virtual ~RTCPeerConnectionIceEvent();

            // properties RTCPeerConnectionIceEvent
            virtual wrapper::org::ortc::adapter::RTCIceCandidatePtr get_candidate() override;
            virtual String get_uri() override;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

