
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCOfferAnswerOptions.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCOfferAnswerOptions : public wrapper::org::webRtc::RTCOfferAnswerOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCOfferAnswerOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCOfferAnswerOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCOfferAnswerOptions, NativeType);

          RTCOfferAnswerOptions() noexcept = delete;
          RTCOfferAnswerOptions(const RTCOfferAnswerOptions &) noexcept = delete;
          virtual ~RTCOfferAnswerOptions() noexcept;

          static void apply(const NativeType &from, WrapperType &to) noexcept;
          static void apply(const WrapperType &from, NativeType &to) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

