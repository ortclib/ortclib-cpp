
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

          RTCIceTransportController();
          virtual ~RTCIceTransportController();

          // methods RTCIceTransportController
          virtual void wrapper_init_org_ortc_RTCIceTransportController() override;
          virtual void addTransport(wrapper::org::ortc::RTCIceTransportPtr transport) override;
          virtual void addTransport(
            wrapper::org::ortc::RTCIceTransportPtr transport,
            uint64_t index
            ) override;

          // properties RTCIceTransportController
          virtual uint64_t get_objectId() override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCIceTransportPtr > > get_transports() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

