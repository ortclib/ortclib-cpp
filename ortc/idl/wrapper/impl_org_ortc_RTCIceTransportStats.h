
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

          RTCIceTransportStats() noexcept;
          virtual ~RTCIceTransportStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCIceTransportStats
          void wrapper_init_org_ortc_RTCIceTransportStats() noexcept override;
          void wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::RTCIceTransportStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCIceTransportStats
          unsigned long long get_bytesSent() noexcept override;
          unsigned long long get_bytesReceived() noexcept override;
          String get_rtcpTransportStatsId() noexcept override;
          bool get_activeConnection() noexcept override;
          String get_selectedCandidatePairId() noexcept override;

          static RTCIceTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

