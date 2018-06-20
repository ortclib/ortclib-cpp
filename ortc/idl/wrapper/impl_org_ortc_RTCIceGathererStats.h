
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGathererStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGathererStats : public wrapper::org::ortc::RTCIceGathererStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::ICEGathererStats, NativeStats);
          RTCIceGathererStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCIceGathererStats() noexcept;
          virtual ~RTCIceGathererStats() noexcept;

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCIceGathererStats
          virtual void wrapper_init_org_ortc_RTCIceGathererStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceGathererStats(wrapper::org::ortc::RTCIceGathererStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceGathererStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCIceGathererStats
          virtual unsigned long long get_bytesSent() noexcept override;
          virtual unsigned long long get_bytesReceived() noexcept override;
          virtual String get_rtcpGathererStatsId() noexcept override;

          static RTCIceGathererStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceGathererStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

