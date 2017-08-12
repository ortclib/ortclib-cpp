
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

          RTCIceGathererStats();
          virtual ~RTCIceGathererStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCIceGathererStats
          virtual void wrapper_init_org_ortc_RTCIceGathererStats() override;
          virtual void wrapper_init_org_ortc_RTCIceGathererStats(wrapper::org::ortc::RTCIceGathererStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceGathererStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCIceGathererStats
          virtual unsigned long long get_bytesSent() override;
          virtual unsigned long long get_bytesReceived() override;
          virtual String get_rtcpGathererStatsId() override;

          static RTCIceGathererStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceGathererStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

