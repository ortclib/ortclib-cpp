
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpSdesTransportLifetimeRemainingEvent.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpSdesTransportLifetimeRemainingEvent : public wrapper::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent
        {
          typedef unsigned long ULONG;
          ZS_DECLARE_TYPEDEF_PTR(RTCSrtpSdesTransportLifetimeRemainingEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSrtpSdesTransportLifetimeRemainingEvent, WrapperType);
          RTCSrtpSdesTransportLifetimeRemainingEventWeakPtr thisWeak_;
          unsigned long leastLifetimeRemainingPercentageForAllKeys_{};
          unsigned long overallLifetimeRemainingPercentage_{};

          RTCSrtpSdesTransportLifetimeRemainingEvent() noexcept;
          virtual ~RTCSrtpSdesTransportLifetimeRemainingEvent() noexcept;

          // properties RTCSrtpSdesTransportLifetimeRemainingEvent
          unsigned long get_leastLifetimeRemainingPercentageForAllKeys() noexcept override;
          unsigned long get_overallLifetimeRemainingPercentage() noexcept override;

          static WrapperImplTypePtr toWrapper(
            ULONG leastLifetimeRemainingPercentageForAllKeys,
            ULONG overallLifetimeRemainingPercentage
          ) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

