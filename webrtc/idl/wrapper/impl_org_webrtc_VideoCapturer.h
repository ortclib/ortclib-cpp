
#pragma once

#include "types.h"
#include "generated/org_webrtc_VideoCapturer.h"

#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "media/base/videocapturer.h"
#include "impl_org_webrtc_post_include.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct VideoCapturer : public wrapper::org::webrtc::VideoCapturer
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::VideoCapturer, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::VideoCapturer, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::cricket::VideoCapturer, NativeType);

          NativeTypeUniPtr native_;
          VideoCapturerWeakPtr thisWeak_;

          VideoCapturer() noexcept;
          virtual ~VideoCapturer() noexcept;

          // methods VideoCapturer
          void wrapper_init_org_webrtc_VideoCapturer() noexcept override;
          shared_ptr< list< wrapper::org::webrtc::VideoFormatPtr > > getSupportedFormats() noexcept override;
          wrapper::org::webrtc::VideoFormatPtr getBestCaptureFormat(wrapper::org::webrtc::VideoFormatPtr desired) noexcept override;
          wrapper::org::webrtc::VideoCaptureState start(wrapper::org::webrtc::VideoFormatPtr captureFormat) noexcept override;
          wrapper::org::webrtc::VideoFormatPtr getCaptureFormat() noexcept override;
          void stop() noexcept override;
          void constrainSupportedFormats(wrapper::org::webrtc::VideoFormatPtr maxFormat) noexcept override;

          // properties VideoCapturer
          String get_id() noexcept override;
          bool get_enableCameraList() noexcept override;
          void set_enableCameraList(bool value) noexcept override;
          bool get_enableVideoAdapter() noexcept override;
          void set_enableVideoAdapter(bool value) noexcept override;
          bool get_isRunning() noexcept override;
          bool get_applyRotation() noexcept override;
          bool get_isScreencast() noexcept override;
          wrapper::org::webrtc::VideoCapturerInputSizePtr get_inputSize() noexcept override;
          wrapper::org::webrtc::VideoCaptureState get_state() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypeUniPtr native) noexcept;
          ZS_NO_DISCARD() static NativeTypeUniPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

