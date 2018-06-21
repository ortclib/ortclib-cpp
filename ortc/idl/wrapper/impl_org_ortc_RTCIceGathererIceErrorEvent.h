
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

          RTCIceGathererIceErrorEvent() noexcept;
          virtual ~RTCIceGathererIceErrorEvent() noexcept;

          // properties RTCIceGathererIceErrorEvent
          wrapper::org::ortc::RTCIceCandidatePtr get_hostCandidate() noexcept override;
          String get_url() noexcept override;
          uint16_t get_errorCode() noexcept override;
          String get_errorText() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

