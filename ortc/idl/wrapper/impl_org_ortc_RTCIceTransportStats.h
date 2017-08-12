
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceTransportStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceTransportStats : public wrapper::org::ortc::RTCIceTransportStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::ICETransportStats, NativeStats);
          RTCIceTransportStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCIceTransportStats();
          virtual ~RTCIceTransportStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCIceTransportStats
          virtual void wrapper_init_org_ortc_RTCIceTransportStats() override;
          virtual void wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::RTCIceTransportStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCIceTransportStats
          virtual unsigned long long get_bytesSent() override;
          virtual unsigned long long get_bytesReceived() override;
          virtual String get_rtcpTransportStatsId() override;
          virtual bool get_activeConnection() override;
          virtual String get_selectedCandidatePairId() override;

          static RTCIceTransportStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceTransportStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

