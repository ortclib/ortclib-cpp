
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCAnswerOptions.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCAnswerOptions : public wrapper::org::webRtc::RTCAnswerOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCAnswerOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCAnswerOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCOfferAnswerOptions, NativeType);
          
          RTCAnswerOptionsWeakPtr thisWeak_;

          RTCAnswerOptions() noexcept;
          virtual ~RTCAnswerOptions() noexcept;


          // methods RTCAnswerOptions
          void wrapper_init_org_webRtc_RTCAnswerOptions() noexcept override;
          void wrapper_init_org_webRtc_RTCAnswerOptions(wrapper::org::webRtc::RTCAnswerOptionsPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

