
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCSessionDescriptionInit.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCSessionDescriptionInit : public wrapper::org::webRtc::RTCSessionDescriptionInit
        {
          RTCSessionDescriptionInitWeakPtr thisWeak_;

          RTCSessionDescriptionInit() noexcept;
          virtual ~RTCSessionDescriptionInit() noexcept;
          void wrapper_init_org_webRtc_RTCSessionDescriptionInit() noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

