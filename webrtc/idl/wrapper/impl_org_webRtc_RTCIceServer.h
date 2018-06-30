
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCIceServer.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCIceServer : public wrapper::org::webrtc::RTCIceServer
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCIceServer, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCIceServer, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::IceServer, NativeType);

          RTCIceServerWeakPtr thisWeak_;

          RTCIceServer() noexcept;
          virtual ~RTCIceServer() noexcept;

          // methods RTCIceServer
          void wrapper_init_org_webrtc_RTCIceServer() noexcept override;
          void wrapper_init_org_webrtc_RTCIceServer(wrapper::org::webrtc::RTCIceServerPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

