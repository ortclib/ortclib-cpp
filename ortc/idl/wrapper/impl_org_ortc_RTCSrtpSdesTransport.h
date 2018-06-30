
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpSdesTransport.h"

#include <ortc/ISRTPSDESTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpSdesTransport : public wrapper::org::ortc::RTCSrtpSdesTransport,
                                      public ::ortc::ISRTPSDESTransportDelegate
        {
          typedef unsigned long ULONG;
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISRTPSDESTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISRTPSDESTransport, ISRTPSDESTransport);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISRTPSDESTransportSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCSrtpSdesTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSrtpSdesTransport, WrapperType);
          RTCSrtpSdesTransportWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCSrtpSdesTransport() noexcept;
          virtual ~RTCSrtpSdesTransport() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState

          // methods RTCSrtpSdesTransport
          void wrapper_init_org_ortc_RTCSrtpSdesTransport(
            wrapper::org::ortc::RTCIceTransportPtr iceTransport,
            wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr encryptParameters,
            wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr decryptParameters
            ) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState
          void stop() noexcept override;

          // properties RTCSrtpSdesTransport
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::RTCIceTransportPtr get_transport() noexcept override;
          wrapper::org::ortc::RTCIceTransportPtr get_rtcpTransport() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onSRTPSDESTransportLifetimeRemaining(
            ISRTPSDESTransportPtr transport,
            ULONG leastLifetimeRemainingPercentageForAllKeys,
            ULONG overallLifetimeRemainingPercentage
          ) noexcept override;

          virtual void onSRTPSDESTransportError(
            ISRTPSDESTransportPtr transport,
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

