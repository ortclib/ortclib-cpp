
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceServer.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceServer : public wrapper::org::ortc::RTCIceServer
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGathererTypes::Server, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceServer, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceServer, WrapperType);
          RTCIceServerWeakPtr thisWeak_;

          RTCIceServer() noexcept;
          virtual ~RTCIceServer() noexcept;

          // methods RTCIceServer
          void wrapper_init_org_ortc_RTCIceServer() noexcept override;
          void wrapper_init_org_ortc_RTCIceServer(wrapper::org::ortc::RTCIceServerPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceServer(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

