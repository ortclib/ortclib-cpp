
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSctpTransportStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSctpTransportStats : public wrapper::org::ortc::RTCSctpTransportStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::SCTPTransportStats, NativeStats);
          RTCSctpTransportStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCSctpTransportStats();
          virtual ~RTCSctpTransportStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCSctpTransportStats
          virtual void wrapper_init_org_ortc_RTCSctpTransportStats() override;
          virtual void wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::RTCSctpTransportStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCSctpTransportStats
          virtual unsigned long get_dataChannelsOpened() override;
          virtual unsigned long get_dataChannelsClosed() override;

          static RTCSctpTransportStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCSctpTransportStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

