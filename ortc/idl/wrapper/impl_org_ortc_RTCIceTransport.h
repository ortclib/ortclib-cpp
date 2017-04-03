
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceTransport.h"

#include <ortc/IICETransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceTransport : public wrapper::org::ortc::RTCIceTransport,
                                 public ::ortc::IICETransportDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransport, IICETransport);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransportSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceTransport, WrapperType);
          RTCIceTransportWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCIceTransport();
          virtual ~RTCIceTransport();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCIceTransport
          virtual void wrapper_init_org_ortc_RTCIceTransport() override;
          virtual void wrapper_init_org_ortc_RTCIceTransport(wrapper::org::ortc::RTCIceGathererPtr gatherer) override;
          virtual void start(
            wrapper::org::ortc::RTCIceGathererPtr gatherer,
            wrapper::org::ortc::RTCIceParametersPtr remoteParameters
            ) override;
          virtual void start(
            wrapper::org::ortc::RTCIceGathererPtr gatherer,
            wrapper::org::ortc::RTCIceParametersPtr remoteParameters,
            wrapper::org::ortc::RTCIceRole role
            ) override;
          virtual void start(
            wrapper::org::ortc::RTCIceGathererPtr gatherer,
            wrapper::org::ortc::RTCIceParametersPtr remoteParameters,
            wrapper::org::ortc::RTCIceTransportOptionsPtr options
            ) override;
          virtual void stop() override;
          virtual wrapper::org::ortc::RTCIceTransportPtr createAssociatedTransport() override;
          virtual void addRemoteCandidate(wrapper::org::ortc::RTCIceGathererCandidatePtr remoteCandidate) override;
          virtual void setRemoteCandidates(shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > remoteCandidates) override;
          virtual void removeRemoteCandidate(wrapper::org::ortc::RTCIceGathererCandidatePtr remoteCandidate) override;
          virtual void keepWarm(wrapper::org::ortc::RTCIceCandidatePairPtr candidatePair) override;
          virtual void keepWarm(
            wrapper::org::ortc::RTCIceCandidatePairPtr candidatePair,
            bool keepWarm
            ) override;

          // properties RTCIceTransport
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCIceGathererPtr get_gatherer() override;
          virtual wrapper::org::ortc::RTCIceRole get_role() override;
          virtual wrapper::org::ortc::RTCIceComponent get_component() override;
          virtual wrapper::org::ortc::RTCIceTransportState get_state() override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > get_remoteCandidates() override;
          virtual wrapper::org::ortc::RTCIceCandidatePairPtr get_selectedCandidatePair() override;
          virtual wrapper::org::ortc::RTCIceParametersPtr get_remoteParameters() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onICETransportStateChange(
            IICETransportPtr transport,
            IICETransport::States state
          ) override;
          virtual void onICETransportCandidatePairAvailable(
            IICETransportPtr transport,
            CandidatePairPtr candidatePair
          ) override;
          virtual void onICETransportCandidatePairGone(
            IICETransportPtr transport,
            CandidatePairPtr candidatePair
          ) override;

          virtual void onICETransportCandidatePairChanged(
            IICETransportPtr transport,
            CandidatePairPtr candidatePair
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

