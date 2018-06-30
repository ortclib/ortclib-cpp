
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCOfferOptions.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCOfferOptions : public wrapper::org::webrtc::RTCOfferOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCOfferOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCOfferOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCOfferAnswerOptions, NativeType);

          RTCOfferOptionsWeakPtr thisWeak_;

          RTCOfferOptions() noexcept;
          virtual ~RTCOfferOptions() noexcept;

          // methods RTCOfferOptions
          void wrapper_init_org_webrtc_RTCOfferOptions() noexcept override;
          void wrapper_init_org_webrtc_RTCOfferOptions(wrapper::org::webrtc::RTCOfferOptionsPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

