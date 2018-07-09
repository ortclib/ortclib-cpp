
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCOfferOptions.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCOfferOptions : public wrapper::org::webRtc::RTCOfferOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCOfferOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCOfferOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCOfferAnswerOptions, NativeType);

          RTCOfferOptionsWeakPtr thisWeak_;

          RTCOfferOptions() noexcept;
          virtual ~RTCOfferOptions() noexcept;


          // methods RTCOfferOptions
          void wrapper_init_org_webRtc_RTCOfferOptions() noexcept override;
          void wrapper_init_org_webRtc_RTCOfferOptions(wrapper::org::webRtc::RTCOfferOptionsPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

