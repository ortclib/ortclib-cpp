
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionRtpSender.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionRtpSender : public wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSender
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::RTPSender, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionRtpSender, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSender, WrapperType);
            RTCSessionDescriptionRtpSenderWeakPtr thisWeak_;

            RTCSessionDescriptionRtpSender();
            virtual ~RTCSessionDescriptionRtpSender();

            // methods RTCSessionDescriptionRtpSender
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSender() override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSender(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderPtr source) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSender(wrapper::org::ortc::JsonPtr json) override;
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

