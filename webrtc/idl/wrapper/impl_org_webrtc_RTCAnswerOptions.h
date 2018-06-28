
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCAnswerOptions.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCAnswerOptions : public wrapper::org::webrtc::RTCAnswerOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCAnswerOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCAnswerOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCOfferAnswerOptions, NativeType);
          
          RTCAnswerOptionsWeakPtr thisWeak_;

          RTCAnswerOptions() noexcept;
          virtual ~RTCAnswerOptions() noexcept;

          // methods RTCAnswerOptions
          void wrapper_init_org_webrtc_RTCAnswerOptions() noexcept override;
          void wrapper_init_org_webrtc_RTCAnswerOptions(wrapper::org::webrtc::RTCAnswerOptionsPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

