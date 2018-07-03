
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCDataChannelInit.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCDataChannelInit : public wrapper::org::webRtc::RTCDataChannelInit
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCDataChannelInit, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelInit, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::DataChannelInit, NativeType);

          RTCDataChannelInitWeakPtr thisWeak_;

          RTCDataChannelInit() noexcept;
          virtual ~RTCDataChannelInit() noexcept;

          // methods RTCDataChannelInit
          void wrapper_init_org_webRtc_RTCDataChannelInit() noexcept override;
          void wrapper_init_org_webRtc_RTCDataChannelInit(wrapper::org::webRtc::RTCDataChannelInitPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

