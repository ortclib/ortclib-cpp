
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionMediaLineDetails.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionMediaLineDetails : public wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::MediaLine::Details, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionMediaLineDetails, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails, WrapperType);
            RTCSessionDescriptionMediaLineDetailsWeakPtr thisWeak_;

            RTCSessionDescriptionMediaLineDetails();
            virtual ~RTCSessionDescriptionMediaLineDetails();

            // methods RTCSessionDescriptionMediaLineDetails
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetailsPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails(wrapper::org::ortc::JsonPtr json) override;
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

