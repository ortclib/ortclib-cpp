
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCIceCandidate.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/jsep.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCIceCandidate : public wrapper::org::webRtc::RTCIceCandidate
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCIceCandidate, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidate, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::IceCandidateInterface, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::cricket::Candidate, CricketNativeType);
          
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidateInit, UseIceCandidateInit);

          String mid_;
          Optional<unsigned short> mLineIndex {};
          String serverUrl_;
          CricketNativeTypeUniPtr native_;
          zsLib::Lock lock_;
          RTCIceCandidateWeakPtr thisWeak_;

          RTCIceCandidate() noexcept;
          virtual ~RTCIceCandidate() noexcept;

          // methods RTCIceCandidate
          void wrapper_init_org_webRtc_RTCIceCandidate() noexcept override;
          void wrapper_init_org_webRtc_RTCIceCandidate(wrapper::org::webRtc::RTCIceCandidateInitPtr init) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          wrapper::org::webRtc::RTCIceCandidateInitPtr toJson() noexcept override;

          // properties RTCIceCandidate
          String get_candidate() noexcept override;
          String get_sdpMid() noexcept override;
          Optional< unsigned short > get_sdpMLineIndex() noexcept override;
          String get_foundation() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceComponent > get_component() noexcept override;
          Optional< unsigned long > get_priority() noexcept override;
          String get_ip() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceProtocol > get_protocol() noexcept override;
          Optional< uint16_t > get_port() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceCandidateType > get_type() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceTcpCandidateType > get_tcpType() noexcept override;
          String get_relatedAddress() noexcept override;
          Optional< uint16_t > get_relatedPort() noexcept override;
          String get_usernameFragment() noexcept override;

          String serverUrl() const noexcept { return serverUrl_; }

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const UseIceCandidateInit &candidate) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseIceCandidateInitPtr candidate) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const CricketNativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const CricketNativeType *native) noexcept;

          ZS_NO_DISCARD() static shared_ptr< list< WrapperImplTypePtr > > toWrapper(const std::vector<CricketNativeType> &native) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType *native) noexcept;

          ZS_NO_DISCARD() static NativeTypeUniPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

