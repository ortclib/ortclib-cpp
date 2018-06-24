
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCStats : public wrapper::org::ortc::RTCStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::Stats, NativeStats);

          RTCStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCStats() noexcept;
          virtual ~RTCStats() noexcept;

          // methods RTCStats
          void wrapper_init_org_ortc_RTCStats() noexcept override;
          void wrapper_init_org_ortc_RTCStats(wrapper::org::ortc::RTCStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCStats(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          static RTCStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

