
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

          RTCSctpTransportStats() noexcept;
          virtual ~RTCSctpTransportStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCSctpTransportStats
          void wrapper_init_org_ortc_RTCSctpTransportStats() noexcept override;
          void wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::RTCSctpTransportStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCSctpTransportStats
          unsigned long get_dataChannelsOpened() noexcept override;
          unsigned long get_dataChannelsClosed() noexcept override;

          static RTCSctpTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCSctpTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

