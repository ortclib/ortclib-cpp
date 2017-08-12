
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

            RTCSessionDescriptionSctpMediaLine();
            virtual ~RTCSessionDescriptionSctpMediaLine();

            // methods RTCSessionDescriptionMediaLine
            virtual wrapper::org::ortc::JsonPtr toJson() override;
            virtual String hash() override;

            // methods RTCSessionDescriptionSctpMediaLine
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine(wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLinePtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine(wrapper::org::ortc::JsonPtr json) override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

