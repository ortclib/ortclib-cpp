
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescription.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescription : public wrapper::org::ortc::adapter::RTCSessionDescription
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescription, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescription, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescription, WrapperType);
            RTCSessionDescriptionWeakPtr thisWeak_;
            NativeTypePtr native_;

            RTCSessionDescription();
            virtual ~RTCSessionDescription();

            // methods RTCSessionDescription
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescription(
              wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type,
              String description
              ) override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescription(
              wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type,
              wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr description
              ) override;

            // properties RTCSessionDescription
            virtual uint64_t get_objectId() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType get_type() override;
            virtual wrapper::org::ortc::adapter::RTCSdpType get_sdpType() override;
            virtual bool get_isJsonSignalling() override;
            virtual bool get_isSdpSignaling() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr get_description() override;
            virtual String get_formattedDescription() override;
            virtual String get_sdp() override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

