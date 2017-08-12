
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionDescription.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionDescription : public wrapper::org::ortc::adapter::RTCSessionDescriptionDescription
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::Description, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionDescription, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionDescription, WrapperType);
            RTCSessionDescriptionDescriptionWeakPtr thisWeak_;

            RTCSessionDescriptionDescription();
            virtual ~RTCSessionDescriptionDescription();

            // methods RTCSessionDescriptionDescription
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescription() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescription(wrapper::org::ortc::JsonPtr json) override;
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

