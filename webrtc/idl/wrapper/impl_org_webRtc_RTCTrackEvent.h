
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCTrackEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCTrackEvent : public wrapper::org::webRtc::RTCTrackEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCTrackEvent, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCTrackEvent, WrapperImplType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver, UseRtpReceiver);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaStreamTrack, UseMediaStreamTrack);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver, UseRtpTransceiver);

          wrapper::org::webRtc::RTCRtpReceiverPtr receiver_;
          wrapper::org::webRtc::MediaStreamTrackPtr track_;
          wrapper::org::webRtc::RTCRtpTransceiverPtr transceiver_;
          RTCTrackEventWeakPtr thisWeak_;

          RTCTrackEvent() noexcept;
          virtual ~RTCTrackEvent() noexcept;

          // properties RTCTrackEvent
          wrapper::org::webRtc::RTCRtpReceiverPtr get_receiver() noexcept override;
          wrapper::org::webRtc::MediaStreamTrackPtr get_track() noexcept override;
          wrapper::org::webRtc::RTCRtpTransceiverPtr get_transceiver() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseRtpReceiverPtr value) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseMediaStreamTrackPtr value) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(RTCRtpTransceiverPtr value) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

