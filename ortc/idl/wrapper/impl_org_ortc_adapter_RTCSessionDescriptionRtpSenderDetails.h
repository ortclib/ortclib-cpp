
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

            RTCSessionDescriptionRtpSenderDetails() noexcept;
            virtual ~RTCSessionDescriptionRtpSenderDetails() noexcept;

            // methods RTCSessionDescriptionRtpSenderDetails
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetailsPtr source) noexcept override;
            void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails(wrapper::org::ortc::JsonPtr json) noexcept override;
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

