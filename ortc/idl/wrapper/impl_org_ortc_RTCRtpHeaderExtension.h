
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpHeaderExtension.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpHeaderExtension : public wrapper::org::ortc::RTCRtpHeaderExtension
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::HeaderExtension, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpHeaderExtension, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpHeaderExtension, WrapperType);
          RTCRtpHeaderExtensionWeakPtr thisWeak_;

          RTCRtpHeaderExtension() noexcept;
          virtual ~RTCRtpHeaderExtension() noexcept;

          // methods RTCRtpHeaderExtension
          void wrapper_init_org_ortc_RTCRtpHeaderExtension() noexcept override;
          void wrapper_init_org_ortc_RTCRtpHeaderExtension(wrapper::org::ortc::RTCRtpHeaderExtensionPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpHeaderExtension(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

