
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceTransportController.h"

#include <ortc/IICETransportController.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceTransportController : public wrapper::org::ortc::RTCIceTransportController
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransportController, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransportController, IICETransportController);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceTransportController, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceTransportController, WrapperType);

          RTCIceTransportControllerWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCIceTransportController() noexcept;
          virtual ~RTCIceTransportController() noexcept;

          // methods RTCIceTransportController
          void wrapper_init_org_ortc_RTCIceTransportController() noexcept override;
          void addTransport(wrapper::org::ortc::RTCIceTransportPtr transport) noexcept(false) override;
          void addTransport(
            wrapper::org::ortc::RTCIceTransportPtr transport,
            uint64_t index
            ) noexcept(false) override;

          // properties RTCIceTransportController
          uint64_t get_objectId() noexcept override;
          shared_ptr< list< wrapper::org::ortc::RTCIceTransportPtr > > get_transports() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

