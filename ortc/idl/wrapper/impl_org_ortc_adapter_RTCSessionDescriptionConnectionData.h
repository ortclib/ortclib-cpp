
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

            RTCSessionDescriptionConnectionData();
            virtual ~RTCSessionDescriptionConnectionData();

            // methods RTCSessionDescriptionConnectionData
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionData() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionData(wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionDataPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionData(wrapper::org::ortc::JsonPtr json) override;
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

