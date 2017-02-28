
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

          RTCRtpHeaderExtension();
          virtual ~RTCRtpHeaderExtension();

          // methods RTCRtpHeaderExtension
          virtual void wrapper_init_org_ortc_RTCRtpHeaderExtension() override;
          virtual void wrapper_init_org_ortc_RTCRtpHeaderExtension(wrapper::org::ortc::RTCRtpHeaderExtensionPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpHeaderExtension(wrapper::org::ortc::JsonPtr json) override;
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

