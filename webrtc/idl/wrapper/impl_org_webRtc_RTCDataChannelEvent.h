
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCDataChannelEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCDataChannelEvent : public wrapper::org::webRtc::RTCDataChannelEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCDataChannelEvent, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelEvent, WrapperImplType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel, UseDataChannel);

          UseDataChannelPtr dataChannel_;
          RTCDataChannelEventWeakPtr thisWeak_;

          RTCDataChannelEvent() noexcept;
          virtual ~RTCDataChannelEvent() noexcept;


          // properties RTCDataChannelEvent
          wrapper::org::webRtc::RTCDataChannelPtr get_channel() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseDataChannelPtr value) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

