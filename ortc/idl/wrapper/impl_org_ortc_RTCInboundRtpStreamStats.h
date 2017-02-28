
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCInboundRtpStreamStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCInboundRtpStreamStats : public wrapper::org::ortc::RTCInboundRtpStreamStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::InboundRTPStreamStats, NativeStats);
          RTCInboundRtpStreamStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCInboundRtpStreamStats();
          virtual ~RTCInboundRtpStreamStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // properties RTCRtpStreamStats
          virtual Optional< uint32_t > get_ssrc() override;
          virtual String get_associatedStatId() override;
          virtual bool get_isRemote() override;
          virtual String get_mediaType() override;
          virtual String get_mediaTrackId() override;
          virtual String get_transportId() override;
          virtual String get_codecId() override;
          virtual unsigned long get_firCount() override;
          virtual unsigned long get_pliCount() override;
          virtual unsigned long get_nackCount() override;
          virtual unsigned long get_sliCount() override;

          // methods RTCInboundRtpStreamStats
          virtual void wrapper_init_org_ortc_RTCInboundRtpStreamStats() override;
          virtual void wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCInboundRtpStreamStats
          virtual unsigned long get_packetsReceived() override;
          virtual unsigned long long get_bytesReceived() override;
          virtual unsigned long get_packetsLost() override;
          virtual double get_jitter() override;
          virtual double get_fractionLost() override;
          virtual ::zsLib::Milliseconds get_endToEndDelay() override;

          static RTCInboundRtpStreamStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

