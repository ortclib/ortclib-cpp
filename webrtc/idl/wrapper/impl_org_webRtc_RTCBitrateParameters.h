
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCBitrateParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCBitrateParameters : public wrapper::org::webRtc::RTCBitrateParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCBitrateParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCBitrateParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::BitrateParameters, NativeType);

          RTCBitrateParametersWeakPtr thisWeak_;

          RTCBitrateParameters() noexcept;
          virtual ~RTCBitrateParameters() noexcept;
          void wrapper_init_org_webRtc_RTCBitrateParameters() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

