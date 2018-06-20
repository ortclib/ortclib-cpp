
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

          RTCSrtpTransportStats() noexcept;
          virtual ~RTCSrtpTransportStats() noexcept;

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCSrtpTransportStats
          virtual void wrapper_init_org_ortc_RTCSrtpTransportStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCSrtpTransportStats(wrapper::org::ortc::RTCSrtpTransportStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCSrtpTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          static RTCSrtpTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCSrtpTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

