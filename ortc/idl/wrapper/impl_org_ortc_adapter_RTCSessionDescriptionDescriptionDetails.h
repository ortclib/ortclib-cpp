
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionDescriptionDetails.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionDescriptionDetails : public wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::Description::Details, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionDescriptionDetails, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails, WrapperType);
            RTCSessionDescriptionDescriptionDetailsWeakPtr thisWeak_;

            RTCSessionDescriptionDescriptionDetails();
            virtual ~RTCSessionDescriptionDescriptionDetails();

            // methods RTCSessionDescriptionDescriptionDetails
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetailsPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails(wrapper::org::ortc::JsonPtr json) override;
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

