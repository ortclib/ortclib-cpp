
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCapabilities.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCapabilities : public wrapper::org::ortc::RTCRtpCapabilities
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::Capabilities, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCapabilities, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCapabilities, WrapperType);
          RTCRtpCapabilitiesWeakPtr thisWeak_;

          RTCRtpCapabilities();
          virtual ~RTCRtpCapabilities();

          // methods RTCRtpCapabilities
          virtual void wrapper_init_org_ortc_RTCRtpCapabilities() override;
          virtual void wrapper_init_org_ortc_RTCRtpCapabilities(wrapper::org::ortc::RTCRtpCapabilitiesPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpCapabilities(wrapper::org::ortc::JsonPtr json) override;
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

