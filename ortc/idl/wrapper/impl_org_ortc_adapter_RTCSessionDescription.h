
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

            RTCSessionDescription() noexcept;
            virtual ~RTCSessionDescription() noexcept;

            // methods RTCSessionDescription
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescription(
              wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type,
              String description
              ) noexcept override;
            virtual void wrapper_init_org_ortc_adapter_RTCSessionDescription(
              wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type,
              wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr description
              ) noexcept override;

            // properties RTCSessionDescription
            virtual uint64_t get_objectId() noexcept override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType get_type() noexcept override;
            virtual wrapper::org::ortc::adapter::RTCSdpType get_sdpType() noexcept override;
            virtual bool get_isJsonSignalling() noexcept override;
            virtual bool get_isSdpSignaling() noexcept override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr get_description() noexcept override;
            virtual String get_formattedDescription() noexcept override;
            virtual String get_sdp() noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

