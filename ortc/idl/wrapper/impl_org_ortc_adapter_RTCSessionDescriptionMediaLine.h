
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCSessionDescriptionMediaLine.h"

#include <ortc/adapter/ISessionDescription.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCSessionDescriptionMediaLine : public wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLine
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::ISessionDescriptionTypes::MediaLine, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCSessionDescriptionMediaLine, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLine, WrapperType);
            RTCSessionDescriptionMediaLineWeakPtr thisWeak_;
            NativeTypePtr native_;

            RTCSessionDescriptionMediaLine();
            virtual ~RTCSessionDescriptionMediaLine();

            // methods RTCSessionDescriptionMediaLine
            virtual wrapper::org::ortc::JsonPtr toJson() override;
            virtual String hash() override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

