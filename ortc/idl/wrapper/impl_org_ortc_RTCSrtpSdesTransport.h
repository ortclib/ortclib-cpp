
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

          RTCSrtpSdesTransport();
          virtual ~RTCSrtpSdesTransport();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCSrtpSdesTransport
          virtual void wrapper_init_org_ortc_RTCSrtpSdesTransport(
            wrapper::org::ortc::RTCIceTransportPtr iceTransport,
            wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr encryptParameters,
            wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr decryptParameters
            ) override;
          virtual void stop() override;

          // properties RTCSrtpSdesTransport
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCIceTransportPtr get_transport() override;
          virtual wrapper::org::ortc::RTCIceTransportPtr get_rtcpTransport() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onSRTPSDESTransportLifetimeRemaining(
            ISRTPSDESTransportPtr transport,
            ULONG leastLifetimeRemainingPercentageForAllKeys,
            ULONG overallLifetimeRemainingPercentage
          ) override;

          virtual void onSRTPSDESTransportError(
            ISRTPSDESTransportPtr transport,
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

