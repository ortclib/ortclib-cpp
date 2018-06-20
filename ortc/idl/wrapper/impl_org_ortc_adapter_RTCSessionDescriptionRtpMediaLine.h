
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

            RTCSessionDescriptionRtpMediaLine() noexcept;
            virtual ~RTCSessionDescriptionRtpMediaLine() noexcept;

            // methods RTCSessionDescriptionMediaLine
            wrapper::org::ortc::JsonPtr toJson() noexcept override;
            String hash() noexcept override;

            // methods RTCSessionDescriptionRtpMediaLine
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpMediaLinePtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine(wrapper::org::ortc::JsonPtr json) noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

