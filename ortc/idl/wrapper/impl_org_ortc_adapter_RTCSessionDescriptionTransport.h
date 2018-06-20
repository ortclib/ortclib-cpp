
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

            RTCSessionDescriptionTransport() noexcept;
            virtual ~RTCSessionDescriptionTransport() noexcept;

            // methods RTCSessionDescriptionTransport
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransport() noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransport(wrapper::org::ortc::adapter::RTCSessionDescriptionTransportPtr source) noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransport(wrapper::org::ortc::JsonPtr json) noexcept override;
            virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
            virtual String hash() noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

