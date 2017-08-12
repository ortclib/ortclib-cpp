
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpHeaderExtensionParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpHeaderExtensionParameters : public wrapper::org::ortc::RTCRtpHeaderExtensionParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::HeaderExtensionParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpHeaderExtensionParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpHeaderExtensionParameters, WrapperType);
          RTCRtpHeaderExtensionParametersWeakPtr thisWeak_;

          RTCRtpHeaderExtensionParameters();
          virtual ~RTCRtpHeaderExtensionParameters();

          // methods RTCRtpHeaderExtensionParameters
          virtual void wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters(wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters(wrapper::org::ortc::JsonPtr json) override;
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

