
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionRtpMediaLine.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionRtpMediaLine : public wrapper::org::ortc::adapter::RTCSessionDescriptionRtpMediaLine
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::RTPMediaLine, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionRtpMediaLine, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpMediaLine, WrapperType);
            RTCSessionDescriptionRtpMediaLineWeakPtr thisWeak_;

            RTCSessionDescriptionRtpMediaLine();
            virtual ~RTCSessionDescriptionRtpMediaLine();

            // methods RTCSessionDescriptionMediaLine
            virtual wrapper::org::ortc::JsonPtr toJson() override;
            virtual String hash() override;

            // methods RTCSessionDescriptionRtpMediaLine
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpMediaLinePtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine(wrapper::org::ortc::JsonPtr json) override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

