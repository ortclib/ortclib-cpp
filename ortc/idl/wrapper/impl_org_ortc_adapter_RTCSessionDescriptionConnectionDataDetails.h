
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

            RTCSessionDescriptionConnectionDataDetails();
            virtual ~RTCSessionDescriptionConnectionDataDetails();

            // methods RTCSessionDescriptionConnectionDataDetails
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionConnectionDataDetailsPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails(wrapper::org::ortc::JsonPtr json) override;
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

