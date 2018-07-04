
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpTransceiver.h"


#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpTransceiver : public wrapper::org::webRtc::RTCRtpTransceiver
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpTransceiver, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpTransceiverInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;

          NativeTypeScopedPtr native_;
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

