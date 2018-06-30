// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpTransceiver.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpTransceiver : public wrapper::org::webRtc::RTCRtpTransceiver
        {
          RTCRtpTransceiverWeakPtr thisWeak_;

          RTCRtpTransceiver() noexcept;
          virtual ~RTCRtpTransceiver() noexcept;

          // methods RTCRtpTransceiver
          void stop() noexcept override;
          void setCodecPreferences(shared_ptr< list< wrapper::org::webRtc::RTCRtpCodecCapabilityPtr > > codecs) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr

          // properties RTCRtpTransceiver
          String get_mid() noexcept override;
          wrapper::org::webRtc::RTCRtpSenderPtr get_sender() noexcept override;
          wrapper::org::webRtc::RTCRtpReceiverPtr get_receiver() noexcept override;
          bool get_stopped() noexcept override;
          wrapper::org::webRtc::RTCRtpTransceiverDirection get_direction() noexcept override;
          void set_direction(wrapper::org::webRtc::RTCRtpTransceiverDirection value) noexcept override;
          Optional< wrapper::org::webRtc::RTCRtpTransceiverDirection > get_currentDirection() noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

