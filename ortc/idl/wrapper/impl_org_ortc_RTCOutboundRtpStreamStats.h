
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

          RTCOutboundRtpStreamStats() noexcept;
          virtual ~RTCOutboundRtpStreamStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // properties RTCRtpStreamStats
          Optional< uint32_t > get_ssrc() noexcept override;
          String get_associatedStatId() noexcept override;
          bool get_isRemote() noexcept override;
          String get_mediaType() noexcept override;
          String get_mediaTrackId() noexcept override;
          String get_transportId() noexcept override;
          String get_codecId() noexcept override;
          unsigned long get_firCount() noexcept override;
          unsigned long get_pliCount() noexcept override;
          unsigned long get_nackCount() noexcept override;
          unsigned long get_sliCount() noexcept override;

          // methods RTCOutboundRtpStreamStats
          void wrapper_init_org_ortc_RTCOutboundRtpStreamStats() noexcept override;
          void wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCOutboundRtpStreamStats
          unsigned long get_packetsSent() noexcept override;
          unsigned long long get_bytesSent() noexcept override;
          double get_targetBitrate() noexcept override;
          double get_roundTripTime() noexcept override;

          static RTCOutboundRtpStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

