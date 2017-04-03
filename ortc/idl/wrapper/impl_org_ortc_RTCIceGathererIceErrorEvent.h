
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGathererIceErrorEvent.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGathererIceErrorEvent : public wrapper::org::ortc::RTCIceGathererIceErrorEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGathererTypes::ErrorEvent, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGathererIceErrorEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGathererIceErrorEvent, WrapperType);
          RTCIceGathererIceErrorEventWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCIceGathererIceErrorEvent();
          virtual ~RTCIceGathererIceErrorEvent();

          // properties RTCIceGathererIceErrorEvent
          virtual wrapper::org::ortc::RTCIceCandidatePtr get_hostCandidate() override;
          virtual String get_url() override;
          virtual uint16_t get_errorCode() override;
          virtual String get_errorText() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

