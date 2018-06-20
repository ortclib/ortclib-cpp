
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCStatsProvider.h"

#include <ortc/IStatsProvider.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCStatsProvider : public wrapper::org::ortc::RTCStatsProvider
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsProvider, NativeType);
          RTCStatsProviderWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCStatsProvider() noexcept;
          virtual ~RTCStatsProvider() noexcept;

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept override;

          static RTCStatsProviderPtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(wrapper::org::ortc::RTCStatsProviderPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

