
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGatherer.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGatherer : public wrapper::org::ortc::RTCIceGatherer,
                                public ::ortc::IICEGathererDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGatherer, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGatherer, IICEGatherer);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICEGathererSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGatherer, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGatherer, WrapperType);
          RTCIceGathererWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCIceGatherer() noexcept;
          virtual ~RTCIceGatherer() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState

          // methods RTCIceGatherer
          void wrapper_init_org_ortc_RTCIceGatherer(wrapper::org::ortc::RTCIceGatherOptionsPtr options) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument
          wrapper::org::ortc::RTCIceGathererPtr createAssociatedGatherer() noexcept(false) override; // throws ::zsLib::Exceptions::BadState
          void gather() noexcept override;
          void gather(wrapper::org::ortc::RTCIceGatherOptionsPtr options) noexcept override;
          void close() noexcept override;

          // properties RTCIceGatherer
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::RTCIceComponent get_component() noexcept override;
          wrapper::org::ortc::RTCIceGathererState get_state() noexcept override;
          wrapper::org::ortc::RTCIceParametersPtr get_localParameters() noexcept override;
          shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > get_localCandidates() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onICEGathererStateChange(
            IICEGathererPtr gatherer,
            IICEGatherer::States state
          ) override;

          virtual void onICEGathererLocalCandidate(
            IICEGathererPtr gatherer,
            CandidatePtr candidate
          ) override;

          virtual void onICEGathererLocalCandidateComplete(
            IICEGathererPtr gatherer,
            CandidateCompletePtr candidate
          ) override;

          virtual void onICEGathererLocalCandidateGone(
            IICEGathererPtr gatherer,
            CandidatePtr candidate
          ) override;

          virtual void onICEGathererError(
            IICEGathererPtr gatherer,
            ErrorEventPtr errorEvent
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

