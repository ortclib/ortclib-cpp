
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

          RTCDtlsTransport() noexcept;
          virtual ~RTCDtlsTransport() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept override;

          // methods RTCDtlsTransport
          void wrapper_init_org_ortc_RTCDtlsTransport(
            wrapper::org::ortc::RTCIceTransportPtr iceTransport,
            shared_ptr< list< wrapper::org::ortc::RTCCertificatePtr > > certificates
            ) noexcept override;
          wrapper::org::ortc::RTCDtlsParametersPtr remoteParameters() noexcept override;
          shared_ptr< list< wrapper::org::ortc::RTCDtlsCertificateBinaryPtr > > getRemoteCertificates() noexcept override;
          void start(wrapper::org::ortc::RTCDtlsParametersPtr remoteParameters) noexcept override;
          void stop() noexcept override;

          // properties RTCDtlsTransport
          uint64_t get_objectId() noexcept override;
          shared_ptr< list< wrapper::org::ortc::RTCCertificatePtr > > get_certificates() noexcept override;
          wrapper::org::ortc::RTCIceTransportPtr get_transport() noexcept override;
          wrapper::org::ortc::RTCDtlsTransportState get_state() noexcept override;
          wrapper::org::ortc::RTCDtlsParametersPtr get_localParameters() noexcept override;

          void wrapper_onObserverCountChanged(size_t count) noexcept override;

          // IDTLSTransportDelegate
          void onDTLSTransportStateChange(
            IDTLSTransportPtr transport,
            IDTLSTransport::States state
          ) noexcept override;

          void onDTLSTransportError(
            IDTLSTransportPtr transport,
            ::ortc::ErrorAnyPtr error
          ) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

