
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionConnectionData.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionConnectionData : public wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionData
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::ConnectionData, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionConnectionData, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionData, WrapperType);
            RTCSessionDescriptionConnectionDataWeakPtr thisWeak_;

            RTCSessionDescriptionConnectionData() noexcept;
            virtual ~RTCSessionDescriptionConnectionData() noexcept;

            // methods RTCSessionDescriptionConnectionData
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionData() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionData(wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionDataPtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionData(wrapper::org::ortc::JsonPtr json) noexcept override;
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

