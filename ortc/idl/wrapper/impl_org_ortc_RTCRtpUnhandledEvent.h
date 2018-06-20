
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpUnhandledEvent.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpUnhandledEvent : public wrapper::org::ortc::RTCRtpUnhandledEvent
        {
          typedef ::ortc::IRTPTypes::SSRCType SSRCType;
          typedef ::ortc::IRTPTypes::PayloadType PayloadType;
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpUnhandledEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpUnhandledEvent, WrapperType);
          RTCRtpUnhandledEventWeakPtr thisWeak_;
          SSRCType ssrc_;
          PayloadType payloadType_;
          String mid_;
          String rid_;

          RTCRtpUnhandledEvent() noexcept;
          virtual ~RTCRtpUnhandledEvent() noexcept;
          virtual void wrapper_init_org_ortc_RTCRtpUnhandledEvent() noexcept override;

          // properties RTCRtpUnhandledEvent
          virtual uint32_t get_ssrc() noexcept override;
          virtual uint8_t get_payloadType() noexcept override;
          virtual String get_muxId() noexcept override;
          virtual String get_rid() noexcept override;

          static WrapperImplTypePtr toWrapper(
            SSRCType ssrc,
            PayloadType payloadType,
            const String &mid,
            const String &rid
          ) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

