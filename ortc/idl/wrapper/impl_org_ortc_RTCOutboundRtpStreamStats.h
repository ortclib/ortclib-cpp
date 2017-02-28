
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCOutboundRtpStreamStats.h"

#include <ortc/IStatsReport.h>


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCOutboundRtpStreamStats : public wrapper::org::ortc::RTCOutboundRtpStreamStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::OutboundRTPStreamStats, NativeStats);
          RTCOutboundRtpStreamStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCOutboundRtpStreamStats();
          virtual ~RTCOutboundRtpStreamStats();

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

          // methods RTCOutboundRtpStreamStats
          virtual void wrapper_init_org_ortc_RTCOutboundRtpStreamStats() override;
          virtual void wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCOutboundRtpStreamStats
          virtual unsigned long get_packetsSent() override;
          virtual unsigned long long get_bytesSent() override;
          virtual double get_targetBitrate() override;
          virtual double get_roundTripTime() override;

          static RTCOutboundRtpStreamStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

