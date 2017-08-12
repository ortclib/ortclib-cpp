
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

          RTCRtcpFeedback();
          virtual ~RTCRtcpFeedback();

          // methods RTCRtcpFeedback
          virtual void wrapper_init_org_ortc_RTCRtcpFeedback() override;
          virtual void wrapper_init_org_ortc_RTCRtcpFeedback(wrapper::org::ortc::RTCRtcpFeedbackPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtcpFeedback(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

