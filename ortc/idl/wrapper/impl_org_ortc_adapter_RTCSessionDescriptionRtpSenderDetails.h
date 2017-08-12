
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionRtpSenderDetails : public wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::RTPSender::Details, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionRtpSenderDetails, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails, WrapperType);
            RTCSessionDescriptionRtpSenderDetailsWeakPtr thisWeak_;

            RTCSessionDescriptionRtpSenderDetails();
            virtual ~RTCSessionDescriptionRtpSenderDetails();

            // methods RTCSessionDescriptionRtpSenderDetails
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetailsPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails(wrapper::org::ortc::JsonPtr json) override;
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

