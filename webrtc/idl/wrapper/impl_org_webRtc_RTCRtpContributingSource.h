
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpContributingSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpContributingSource : public wrapper::org::webRtc::RTCRtpContributingSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpContributingSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpContributingSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpSource, NativeType);

          RTCRtpContributingSourceWeakPtr thisWeak_;

          RTCRtpContributingSource() noexcept;
          virtual ~RTCRtpContributingSource() noexcept;

          static void apply(const NativeType &from, WrapperType &to);

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

