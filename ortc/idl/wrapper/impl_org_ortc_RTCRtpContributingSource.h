
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpContributingSource.h"

#include <ortc/IRTPReceiver.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpContributingSource : public wrapper::org::ortc::RTCRtpContributingSource
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPReceiverTypes::ContributingSource, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpContributingSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpContributingSource, WrapperType);
          RTCRtpContributingSourceWeakPtr thisWeak_;

          RTCRtpContributingSource() noexcept;
          virtual ~RTCRtpContributingSource() noexcept;

          // methods RTCRtpContributingSource
          void wrapper_init_org_ortc_RTCRtpContributingSource() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

