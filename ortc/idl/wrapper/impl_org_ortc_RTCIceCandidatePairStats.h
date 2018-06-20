
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
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCIceCandidatePairStats
          virtual void wrapper_init_org_ortc_RTCIceCandidatePairStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::RTCIceCandidatePairStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCIceCandidatePairStats
          virtual String get_transportId() noexcept override;
          virtual String get_localCandidateId() noexcept override;
          virtual String get_remoteCandidateId() noexcept override;
          virtual wrapper::org::ortc::RTCIceCandidatePairState get_state() noexcept override;
          virtual unsigned long long get_priority() noexcept override;
          virtual bool get_nominated() noexcept override;
          virtual bool get_writable() noexcept override;
          virtual bool get_readable() noexcept override;
          virtual unsigned long long get_bytesSent() noexcept override;
          virtual unsigned long long get_bytesReceived() noexcept override;
          virtual double get_roundTripTime() noexcept override;
          virtual double get_availableOutgoingBitrate() noexcept override;
          virtual double get_availableIncomingBitrate() noexcept override;

          static RTCIceCandidatePairStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

