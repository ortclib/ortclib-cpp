
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
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCIceTransportStats
          virtual void wrapper_init_org_ortc_RTCIceTransportStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::RTCIceTransportStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCIceTransportStats
          virtual unsigned long long get_bytesSent() noexcept override;
          virtual unsigned long long get_bytesReceived() noexcept override;
          virtual String get_rtcpTransportStatsId() noexcept override;
          virtual bool get_activeConnection() noexcept override;
          virtual String get_selectedCandidatePairId() noexcept override;

          static RTCIceTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

