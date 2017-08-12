
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

          RTCStats();
          virtual ~RTCStats();

          // methods RTCStats
          virtual void wrapper_init_org_ortc_RTCStats() override;
          virtual void wrapper_init_org_ortc_RTCStats(wrapper::org::ortc::RTCStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCStats(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          static RTCStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

