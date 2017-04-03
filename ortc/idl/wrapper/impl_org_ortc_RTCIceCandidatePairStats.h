
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

          RTCIceCandidatePairStats();
          virtual ~RTCIceCandidatePairStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCIceCandidatePairStats
          virtual void wrapper_init_org_ortc_RTCIceCandidatePairStats() override;
          virtual void wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::RTCIceCandidatePairStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceCandidatePairStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCIceCandidatePairStats
          virtual String get_transportId() override;
          virtual String get_localCandidateId() override;
          virtual String get_remoteCandidateId() override;
          virtual wrapper::org::ortc::RTCIceCandidatePairState get_state() override;
          virtual unsigned long long get_priority() override;
          virtual bool get_nominated() override;
          virtual bool get_writable() override;
          virtual bool get_readable() override;
          virtual unsigned long long get_bytesSent() override;
          virtual unsigned long long get_bytesReceived() override;
          virtual double get_roundTripTime() override;
          virtual double get_availableOutgoingBitrate() override;
          virtual double get_availableIncomingBitrate() override;

          static RTCIceCandidatePairStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceCandidatePairStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

