
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpSdesTransportLifetimeRemainingEvent.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpSdesTransportLifetimeRemainingEvent : public wrapper::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCSrtpSdesTransportLifetimeRemainingEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent, WrapperType);
          RTCSrtpSdesTransportLifetimeRemainingEventWeakPtr thisWeak_;
          unsigned long leastLifetimeRemainingPercentageForAllKeys_{};
          unsigned long overallLifetimeRemainingPercentage_{};

          RTCSrtpSdesTransportLifetimeRemainingEvent();
          virtual ~RTCSrtpSdesTransportLifetimeRemainingEvent();

          // properties RTCSrtpSdesTransportLifetimeRemainingEvent
          virtual unsigned long get_leastLifetimeRemainingPercentageForAllKeys() override;
          virtual unsigned long get_overallLifetimeRemainingPercentage() override;

          static WrapperImplTypePtr toWrapper(
            ULONG leastLifetimeRemainingPercentageForAllKeys,
            ULONG overallLifetimeRemainingPercentage
          );
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

