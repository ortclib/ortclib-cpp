
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

            RTCSessionDescriptionMediaLineDetails() noexcept;
            virtual ~RTCSessionDescriptionMediaLineDetails() noexcept;

            // methods RTCSessionDescriptionMediaLineDetails
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetailsPtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails(wrapper::org::ortc::JsonPtr json) noexcept override;
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

