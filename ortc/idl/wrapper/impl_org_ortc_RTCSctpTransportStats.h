
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
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCSctpTransportStats
          virtual void wrapper_init_org_ortc_RTCSctpTransportStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::RTCSctpTransportStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCSctpTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCSctpTransportStats
          virtual unsigned long get_dataChannelsOpened() noexcept override;
          virtual unsigned long get_dataChannelsClosed() noexcept override;

          static RTCSctpTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCSctpTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

