
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpSender.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpSender : public wrapper::org::webRtc::RTCRtpSender
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpSender, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSender, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpSenderInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;

          NativeTypeScopedPtr native_;
          RTCRtpSenderWeakPtr thisWeak_;

          RTCRtpSender() noexcept;
          virtual ~RTCRtpSender() noexcept;
          void wrapper_dispose() noexcept override;


          // methods RTCRtpSender
          wrapper::org::webRtc::RTCRtpSendParametersPtr getParameters() noexcept override;
          PromisePtr setParameters(wrapper::org::webRtc::RTCRtpSendParametersPtr parameters) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          PromisePtr replaceTrack(wrapper::org::webRtc::MediaStreamTrackPtr withTrack) noexcept override;

          // properties RTCRtpSender
          wrapper::org::webRtc::MediaStreamTrackPtr get_track() noexcept override;
          wrapper::org::webRtc::RTCDtmfSenderPtr get_dtmf() noexcept override;

          void setupObserver() noexcept;
          void teardownObserver() noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypeScopedPtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

