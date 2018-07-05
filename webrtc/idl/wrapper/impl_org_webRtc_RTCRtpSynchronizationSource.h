
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpSynchronizationSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpSynchronizationSource : public wrapper::org::webRtc::RTCRtpSynchronizationSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpSynchronizationSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSynchronizationSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpSource, NativeType);

          RTCRtpSynchronizationSourceWeakPtr thisWeak_;

          RTCRtpSynchronizationSource() noexcept;
          virtual ~RTCRtpSynchronizationSource() noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

