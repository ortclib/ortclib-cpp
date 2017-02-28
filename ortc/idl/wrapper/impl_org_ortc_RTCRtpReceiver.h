
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

          RTCRtpReceiver();
          virtual ~RTCRtpReceiver();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCRtpReceiver
          virtual void wrapper_init_org_ortc_RTCRtpReceiver(
            wrapper::org::ortc::MediaStreamTrackKind kind,
            wrapper::org::ortc::RTCRtpTransportPtr transport
            ) override;
          virtual void wrapper_init_org_ortc_RTCRtpReceiver(
            wrapper::org::ortc::MediaStreamTrackKind kind,
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
            ) override;
          virtual void setTransport(wrapper::org::ortc::RTCRtpTransportPtr transport) override;
          virtual void setTransport(
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
            ) override;
          virtual PromisePtr receive(wrapper::org::ortc::RTCRtpParametersPtr parameters) override;
          virtual void stop() override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCRtpContributingSourcePtr > > getContributingSources() override;
          virtual void requestSendCsrc(uint32_t csrc) override;

          // properties RTCRtpReceiver
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::MediaStreamTrackPtr get_track() override;
          virtual wrapper::org::ortc::RTCRtpTransportPtr get_transport() override;
          virtual wrapper::org::ortc::RTCRtcpTransportPtr get_rtcpTransport() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

