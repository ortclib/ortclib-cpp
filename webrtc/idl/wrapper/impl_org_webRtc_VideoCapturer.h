
#pragma once

#include "types.h"
#include "generated/org_webRtc_VideoCapturer.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "media/base/videocapturer.h"
#include "impl_org_webRtc_post_include.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct VideoCapturer : public wrapper::org::webRtc::VideoCapturer
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::VideoCapturer, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoCapturer, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::cricket::VideoCapturer, NativeType);

          NativeTypeUniPtr native_;
          VideoCapturerWeakPtr thisWeak_;

          VideoCapturer() noexcept;
          virtual ~VideoCapturer() noexcept;

          // methods VideoCapturer
          shared_ptr< list< wrapper::org::webRtc::VideoFormatPtr > > getSupportedFormats() noexcept override;
          wrapper::org::webRtc::VideoFormatPtr getBestCaptureFormat(wrapper::org::webRtc::VideoFormatPtr desired) noexcept override;
          wrapper::org::webRtc::VideoCaptureState start(wrapper::org::webRtc::VideoFormatPtr captureFormat) noexcept override;
          wrapper::org::webRtc::VideoFormatPtr getCaptureFormat() noexcept override;
          void stop() noexcept override;
          void constrainSupportedFormats(wrapper::org::webRtc::VideoFormatPtr maxFormat) noexcept override;

          // properties VideoCapturer
          String get_id() noexcept override;
          bool get_enableCameraList() noexcept override;
          void set_enableCameraList(bool value) noexcept override;
          bool get_enableVideoAdapter() noexcept override;
          void set_enableVideoAdapter(bool value) noexcept override;
          bool get_isRunning() noexcept override;
          bool get_applyRotation() noexcept override;
          bool get_isScreencast() noexcept override;
          wrapper::org::webRtc::VideoCapturerInputSizePtr get_inputSize() noexcept override;
          wrapper::org::webRtc::VideoCaptureState get_state() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypeUniPtr native) noexcept;
          ZS_NO_DISCARD() static NativeTypeUniPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

