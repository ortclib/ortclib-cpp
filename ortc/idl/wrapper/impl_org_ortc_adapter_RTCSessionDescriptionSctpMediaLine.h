
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionSctpMediaLine.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionSctpMediaLine : public wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::SCTPMediaLine, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionSctpMediaLine, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine, WrapperType);
            RTCSessionDescriptionSctpMediaLineWeakPtr thisWeak_;

            RTCSessionDescriptionSctpMediaLine() noexcept;
            virtual ~RTCSessionDescriptionSctpMediaLine() noexcept;

            // methods RTCSessionDescriptionMediaLine
            virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
            virtual String hash() noexcept override;

            // methods RTCSessionDescriptionSctpMediaLine
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine() noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine(wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLinePtr source) noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine(wrapper::org::ortc::JsonPtr json) noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

