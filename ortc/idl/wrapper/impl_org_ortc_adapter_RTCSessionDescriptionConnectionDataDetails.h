
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionConnectionDataDetails : public wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionDataDetails
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::ConnectionData::Details, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionConnectionDataDetails, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionDataDetails, WrapperType);
            RTCSessionDescriptionConnectionDataDetailsWeakPtr thisWeak_;

            RTCSessionDescriptionConnectionDataDetails() noexcept;
            virtual ~RTCSessionDescriptionConnectionDataDetails() noexcept;

            // methods RTCSessionDescriptionConnectionDataDetails
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails() noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionDataDetailsPtr source) noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails(wrapper::org::ortc::JsonPtr json) noexcept override;
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

