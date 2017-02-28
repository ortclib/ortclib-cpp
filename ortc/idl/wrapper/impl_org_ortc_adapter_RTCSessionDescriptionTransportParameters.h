
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

            RTCSessionDescriptionTransportParameters();
            virtual ~RTCSessionDescriptionTransportParameters();

            // methods RTCSessionDescriptionTransportParameters
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters(wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParametersPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters(wrapper::org::ortc::JsonPtr json) override;
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

