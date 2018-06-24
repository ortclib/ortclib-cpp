
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpSender.h"

#include <ortc/IRTPSender.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpSender : public wrapper::org::ortc::RTCRtpSender,
                              public ::ortc::IRTPSenderDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPSender, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPSender, IRTPSender);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPSenderSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpSender, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpSender, WrapperType);

          RTCRtpSenderWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCRtpSender() noexcept;
          virtual ~RTCRtpSender() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override;

          // methods RTCRtpSender
          void wrapper_init_org_ortc_RTCRtpSender(
            wrapper::org::ortc::MediaStreamTrackPtr track,
            wrapper::org::ortc::RTCRtpTransportPtr transport
            ) noexcept(false) override;
          void wrapper_init_org_ortc_RTCRtpSender(
            wrapper::org::ortc::MediaStreamTrackPtr track,
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
            ) noexcept(false) override;
          void setTransport(wrapper::org::ortc::RTCRtpTransportPtr transport) noexcept(false) override;
          void setTransport(
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
            ) noexcept(false) override;
          PromisePtr setTrack(wrapper::org::ortc::MediaStreamTrackPtr track) noexcept(false) override;
          PromisePtr send(wrapper::org::ortc::RTCRtpParametersPtr parameters) noexcept(false) override;
          void stop() noexcept override;

          // properties RTCRtpSender
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::MediaStreamTrackPtr get_track() noexcept override;
          wrapper::org::ortc::RTCRtpTransportPtr get_transport() noexcept override;
          wrapper::org::ortc::RTCRtcpTransportPtr get_rtcpTransport() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onRTPSenderSSRCConflict(
            IRTPSenderPtr sender,
            SSRCType ssrc
          ) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

