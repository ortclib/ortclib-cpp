
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtcpFeedback.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtcpFeedback : public wrapper::org::ortc::RTCRtcpFeedback
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTCPFeedback, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtcpFeedback, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtcpFeedback, WrapperType);
          RTCRtcpFeedbackWeakPtr thisWeak_;

          RTCRtcpFeedback() noexcept;
          virtual ~RTCRtcpFeedback() noexcept;

          // methods RTCRtcpFeedback
          virtual void wrapper_init_org_ortc_RTCRtcpFeedback() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtcpFeedback(wrapper::org::ortc::RTCRtcpFeedbackPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtcpFeedback(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

