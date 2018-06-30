
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCIceServer.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCIceServer : public wrapper::org::webRtc::RTCIceServer
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCIceServer, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceServer, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::IceServer, NativeType);

          RTCIceServerWeakPtr thisWeak_;

          RTCIceServer() noexcept;
          virtual ~RTCIceServer() noexcept;

          // methods RTCIceServer
          void wrapper_init_org_webRtc_RTCIceServer() noexcept override;
          void wrapper_init_org_webRtc_RTCIceServer(wrapper::org::webRtc::RTCIceServerPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

