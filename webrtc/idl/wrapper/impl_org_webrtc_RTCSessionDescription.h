
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCSessionDescription.h"

#include "impl_org_webrtc_pre_include.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCSessionDescription : public wrapper::org::webrtc::RTCSessionDescription
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCSessionDescription, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCSessionDescription, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::SessionDescriptionInterface, NativeType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCSessionDescriptionInit, UseSessionDescriptionInit);

          UseSessionDescriptionInitPtr init_;
          RTCSessionDescriptionWeakPtr thisWeak_;

          RTCSessionDescription() noexcept;
          virtual ~RTCSessionDescription() noexcept;

          // methods RTCSessionDescription
          void wrapper_init_org_webrtc_RTCSessionDescription(wrapper::org::webrtc::RTCSessionDescriptionInitPtr init) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr

          // properties RTCSessionDescription
          wrapper::org::webrtc::RTCSdpType get_sdpType() noexcept override;
          String get_sdp() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

