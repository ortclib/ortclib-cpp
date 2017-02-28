
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

          RTCIceServer();
          virtual ~RTCIceServer();

          // methods RTCIceServer
          virtual void wrapper_init_org_ortc_RTCIceServer() override;
          virtual void wrapper_init_org_ortc_RTCIceServer(wrapper::org::ortc::RTCIceServerPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceServer(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

