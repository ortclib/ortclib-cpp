
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

          RTCIceGatherer();
          virtual ~RTCIceGatherer();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCIceGatherer
          virtual void wrapper_init_org_ortc_RTCIceGatherer(wrapper::org::ortc::RTCIceGatherOptionsPtr options) override;
          virtual wrapper::org::ortc::RTCIceGathererPtr createAssociatedGatherer() override;
          virtual void gather() override;
          virtual void gather(wrapper::org::ortc::RTCIceGatherOptionsPtr options) override;
          virtual void close() override;

          // properties RTCIceGatherer
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCIceComponent get_component() override;
          virtual wrapper::org::ortc::RTCIceGathererState get_state() override;
          virtual wrapper::org::ortc::RTCIceParametersPtr get_localParameters() override;
          virtual shared_ptr< list< wrapper::org::ortc::RTCIceCandidatePtr > > get_localCandidates() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

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

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

