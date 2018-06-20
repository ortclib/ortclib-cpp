
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

          RTCRtpCapabilities() noexcept;
          virtual ~RTCRtpCapabilities() noexcept;

          // methods RTCRtpCapabilities
          virtual void wrapper_init_org_ortc_RTCRtpCapabilities() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpCapabilities(wrapper::org::ortc::RTCRtpCapabilitiesPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpCapabilities(wrapper::org::ortc::JsonPtr json) noexcept override;
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

