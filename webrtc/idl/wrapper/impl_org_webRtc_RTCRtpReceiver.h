
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpReceiver.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpReceiver : public wrapper::org::webRtc::RTCRtpReceiver
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpReceiver, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpReceiverInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;

          NativeTypeScopedPtr native_;
          RTCRtpReceiverWeakPtr thisWeak_;

          RTCRtpReceiver() noexcept;
          virtual ~RTCRtpReceiver() noexcept;

          // methods RTCRtpReceiver
          wrapper::org::webRtc::RTCRtpReceiveParametersPtr getParameters() noexcept override;
          shared_ptr< list< wrapper::org::webRtc::RTCRtpContributingSourcePtr > > getContributingSources() noexcept override;
          shared_ptr< list< wrapper::org::webRtc::RTCRtpSynchronizationSourcePtr > > getSynchronizationSources() noexcept override;

          // properties RTCRtpReceiver
          wrapper::org::webRtc::MediaStreamTrackPtr get_track() noexcept override;

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

