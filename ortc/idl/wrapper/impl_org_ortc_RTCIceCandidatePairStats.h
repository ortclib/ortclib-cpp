
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidatePairStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidatePairStats : public wrapper::org::ortc::RTCIceCandidatePairStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::ICECandidatePairStats, NativeStats);
          RTCIceCandidatePairStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCIceCandidatePairStats() noexcept;
          virtual ~RTCIceCandidatePairStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCIceCandidatePairStats
          void wrapper_init_org_ortc_RTCIceCandidatePairStats() noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::RTCIceCandidatePairStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCIceCandidatePairStats
          String get_transportId() noexcept override;
          String get_localCandidateId() noexcept override;
          String get_remoteCandidateId() noexcept override;
          wrapper::org::ortc::RTCIceCandidatePairState get_state() noexcept override;
          unsigned long long get_priority() noexcept override;
          bool get_nominated() noexcept override;
          bool get_writable() noexcept override;
          bool get_readable() noexcept override;
          unsigned long long get_bytesSent() noexcept override;
          unsigned long long get_bytesReceived() noexcept override;
          double get_roundTripTime() noexcept override;
          double get_availableOutgoingBitrate() noexcept override;
          double get_availableIncomingBitrate() noexcept override;

          static RTCIceCandidatePairStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

