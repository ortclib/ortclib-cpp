
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

            RTCSessionDescriptionDescriptionDetails() noexcept;
            virtual ~RTCSessionDescriptionDescriptionDetails() noexcept;

            // methods RTCSessionDescriptionDescriptionDetails
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetailsPtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails(wrapper::org::ortc::JsonPtr json) noexcept override;
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

