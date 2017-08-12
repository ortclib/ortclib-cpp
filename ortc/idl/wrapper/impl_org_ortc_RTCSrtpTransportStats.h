
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpTransportStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpTransportStats : public wrapper::org::ortc::RTCSrtpTransportStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::SRTPTransportStats, NativeStats);
          RTCSrtpTransportStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCSrtpTransportStats();
          virtual ~RTCSrtpTransportStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCSrtpTransportStats
          virtual void wrapper_init_org_ortc_RTCSrtpTransportStats() override;
          virtual void wrapper_init_org_ortc_RTCSrtpTransportStats(wrapper::org::ortc::RTCSrtpTransportStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCSrtpTransportStats(wrapper::org::ortc::JsonPtr json) override;

          static RTCSrtpTransportStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCSrtpTransportStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

