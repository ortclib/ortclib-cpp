
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCSessionDescription.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCSessionDescription : public wrapper::org::webRtc::RTCSessionDescription
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCSessionDescription, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCSessionDescription, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::SessionDescriptionInterface, NativeType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCSessionDescriptionInit, UseSessionDescriptionInit);

          NativeTypeUniPtr native_;
          RTCSessionDescriptionWeakPtr thisWeak_;

          RTCSessionDescription() noexcept;
          virtual ~RTCSessionDescription() noexcept;


          // methods RTCSessionDescription
          void wrapper_init_org_webRtc_RTCSessionDescription(wrapper::org::webRtc::RTCSessionDescriptionInitPtr init) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr

          // properties RTCSessionDescription
          wrapper::org::webRtc::RTCSdpType get_sdpType() noexcept override;
          String get_sdp() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType *native) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const UseSessionDescriptionInit &value) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseSessionDescriptionInitPtr value) noexcept;

          ZS_NO_DISCARD() static NativeTypeUniPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

