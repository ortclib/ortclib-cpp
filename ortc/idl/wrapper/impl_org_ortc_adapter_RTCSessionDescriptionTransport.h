
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionTransport.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionTransport : public wrapper::org::ortc::adapter::RTCSessionDescriptionTransport
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::Transport, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionTransport, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionTransport, WrapperType);
            RTCSessionDescriptionTransportWeakPtr thisWeak_;

            RTCSessionDescriptionTransport();
            virtual ~RTCSessionDescriptionTransport();

            // methods RTCSessionDescriptionTransport
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransport() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransport(wrapper::org::ortc::adapter::RTCSessionDescriptionTransportPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransport(wrapper::org::ortc::JsonPtr json) override;
            virtual wrapper::org::ortc::JsonPtr toJson() override;
            virtual String hash() override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

