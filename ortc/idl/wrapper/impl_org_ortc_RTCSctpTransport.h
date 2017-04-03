
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSctpTransport.h"

#include <ortc/ISCTPTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSctpTransport : public wrapper::org::ortc::RTCSctpTransport,
                                  public ::ortc::ISCTPTransportDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransport, ISCTPTransport);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransportSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCSctpTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSctpTransport, WrapperType);

          RTCSctpTransportWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCSctpTransport();
          virtual ~RTCSctpTransport();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCSctpTransport
          virtual void wrapper_init_org_ortc_RTCSctpTransport(wrapper::org::ortc::RTCDtlsTransportPtr transport) override;
          virtual void wrapper_init_org_ortc_RTCSctpTransport(
            wrapper::org::ortc::RTCDtlsTransportPtr transport,
            uint16_t localPort
            ) override;
          virtual void start(wrapper::org::ortc::RTCSctpCapabilitiesPtr remoteCapabilities) override;
          virtual void start(
            wrapper::org::ortc::RTCSctpCapabilitiesPtr remoteCapabilities,
            uint16_t remotePort
            ) override;
          virtual void stop() override;

          // properties RTCSctpTransport
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCDtlsTransportPtr get_transport() override;
          virtual wrapper::org::ortc::RTCSctpTransportState get_state() override;
          virtual uint16_t get_port() override;
          virtual uint16_t get_localPort() override;
          virtual Optional< uint16_t > get_remotePort() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onSCTPTransportStateChange(
            ISCTPTransportPtr transport,
            States state
          ) override;
          virtual void onSCTPTransportDataChannel(
            ISCTPTransportPtr transport,
            ::ortc::IDataChannelPtr channel
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

