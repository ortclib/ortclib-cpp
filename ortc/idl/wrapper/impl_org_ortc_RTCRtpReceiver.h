
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpReceiver.h"

#include <ortc/IRTPReceiver.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpReceiver : public wrapper::org::ortc::RTCRtpReceiver,
                                public ::ortc::IRTPReceiverDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPReceiver, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPReceiver, IRTPReceiver);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPReceiverSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpReceiver, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpReceiver, WrapperType);
          RTCRtpReceiverWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCRtpReceiver() noexcept;
          virtual ~RTCRtpReceiver() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState

          // methods RTCRtpReceiver
          void wrapper_init_org_ortc_RTCRtpReceiver(
            wrapper::org::ortc::MediaStreamTrackKind kind,
            wrapper::org::ortc::RTCRtpTransportPtr transport
            ) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState
          void wrapper_init_org_ortc_RTCRtpReceiver(
            wrapper::org::ortc::MediaStreamTrackKind kind,
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
            ) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState
          void setTransport(wrapper::org::ortc::RTCRtpTransportPtr transport) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState
          void setTransport(
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
            ) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState
          PromisePtr receive(wrapper::org::ortc::RTCRtpParametersPtr parameters) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState
          void stop() noexcept override;
          shared_ptr< list< wrapper::org::ortc::RTCRtpContributingSourcePtr > > getContributingSources() noexcept override;
          void requestSendCsrc(uint32_t csrc) noexcept override;

          // properties RTCRtpReceiver
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::MediaStreamTrackPtr get_track() noexcept override;
          wrapper::org::ortc::RTCRtpTransportPtr get_transport() noexcept override;
          wrapper::org::ortc::RTCRtcpTransportPtr get_rtcpTransport() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

