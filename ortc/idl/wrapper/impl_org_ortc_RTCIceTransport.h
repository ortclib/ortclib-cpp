
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

          RTCIceTransport() noexcept;
          virtual ~RTCIceTransport() noexcept;

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept override;

          // methods RTCIceTransport
          virtual void wrapper_init_org_ortc_RTCIceTransport() noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceTransport(wrapper::org::ortc::RTCIceGathererPtr gatherer) noexcept override;
          virtual void start(
            wrapper::org::ortc::RTCIceGathererPtr gatherer,
            wrapper::org::ortc::RTCIceParametersPtr remoteParameters
            ) noexcept override;
          virtual void start(
            wrapper::org::ortc::RTCIceGathererPtr gatherer,
            wrapper::org::ortc::RTCIceParametersPtr remoteParameters,
            wrapper::org::ortc::RTCIceRole role
            ) noexcept override;
          virtual void start(
            wrapper::org::ortc::RTCIceGathererPtr gatherer,
            wrapper::org::ortc::RTCIceParametersPtr remoteParameters,
            wrapper::org::ortc::RTCIceTransportOptionsPtr options
            ) noexcept override;
          virtual void stop() noexcept override;
          virtual wrapper::org::ortc::RTCIceTransportPtr createAssociatedTransport() noexcept override;
          virtual void addRemoteCandidate(wrapper::org::ortc::RTCIceGathererCandidatePtr remoteCandidate) noexcept override;
          virtual void setRemoteCandidates(shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > remoteCandidates) noexcept override;
          virtual void removeRemoteCandidate(wrapper::org::ortc::RTCIceGathererCandidatePtr remoteCandidate) noexcept override;
          virtual void keepWarm(wrapper::org::ortc::RTCIceCandidatePairPtr candidatePair) noexcept override;
          virtual void keepWarm(
            wrapper::org::ortc::RTCIceCandidatePairPtr candidatePair,
            bool keepWarm
            ) noexcept override;

          // properties RTCIceTransport
          virtual uint64_t get_objectId() noexcept override;
          virtual wrapper::org::ortc::RTCIceGathererPtr get_gatherer() noexcept override;
          virtual wrapper::org::ortc::RTCIceRole get_role() noexcept override;
          virtual wrapper::org::ortc::RTCIceComponent get_component() noexcept override;
          virtual wrapper::org::ortc::RTCIceTransportState get_state() noexcept override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > get_remoteCandidates() noexcept override;
          virtual wrapper::org::ortc::RTCIceCandidatePairPtr get_selectedCandidatePair() noexcept override;
          virtual wrapper::org::ortc::RTCIceParametersPtr get_remoteParameters() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onICETransportStateChange(
            IICETransportPtr transport,
            IICETransport::States state
          ) noexcept override;
          virtual void onICETransportCandidatePairAvailable(
            IICETransportPtr transport,
            CandidatePairPtr candidatePair
          ) noexcept override;
          virtual void onICETransportCandidatePairGone(
            IICETransportPtr transport,
            CandidatePairPtr candidatePair
          ) noexcept override;

          virtual void onICETransportCandidatePairChanged(
            IICETransportPtr transport,
            CandidatePairPtr candidatePair
          ) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

