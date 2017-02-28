
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtlsTransport.h"

#include <ortc/IDTLSTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtlsTransport : public wrapper::org::ortc::RTCDtlsTransport,
                                  public ::ortc::IDTLSTransportDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTLSTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTLSTransport, IDTLSTransport);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTLSTransportSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCDtlsTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtlsTransport, WrapperType);
          RTCDtlsTransportWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCDtlsTransport();
          virtual ~RTCDtlsTransport();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCDtlsTransport
          virtual void wrapper_init_org_ortc_RTCDtlsTransport(
            wrapper::org::ortc::RTCIceTransportPtr iceTransport,
            shared_ptr< list< wrapper::org::ortc::RTCCertificatePtr > > certificates
            ) override;
          virtual wrapper::org::ortc::RTCDtlsParametersPtr remoteParameters() override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCDtlsCertificateBinaryPtr > > getRemoteCertificates() override;
          virtual void start(wrapper::org::ortc::RTCDtlsParametersPtr remoteParameters) override;
          virtual void stop() override;

          // properties RTCDtlsTransport
          virtual uint64_t get_objectId() override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCCertificatePtr > > get_certificates() override;
          virtual wrapper::org::ortc::RTCIceTransportPtr get_transport() override;
          virtual wrapper::org::ortc::RTCDtlsTransportState get_state() override;
          virtual wrapper::org::ortc::RTCDtlsParametersPtr get_localParameters() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          // IDTLSTransportDelegate
          virtual void onDTLSTransportStateChange(
            IDTLSTransportPtr transport,
            IDTLSTransport::States state
          ) override;

          virtual void onDTLSTransportError(
            IDTLSTransportPtr transport,
            ::ortc::ErrorAnyPtr error
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

