
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionTransportParameters.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionTransportParameters : public wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParameters
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::Transport::Parameters, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionTransportParameters, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParameters, WrapperType);
            RTCSessionDescriptionTransportParametersWeakPtr thisWeak_;

            RTCSessionDescriptionTransportParameters() noexcept;
            virtual ~RTCSessionDescriptionTransportParameters() noexcept;

            // methods RTCSessionDescriptionTransportParameters
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters(wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParametersPtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
            wrapper::org::ortc::JsonPtr toJson() noexcept override;
            String hash() noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

