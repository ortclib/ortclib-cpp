
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

          RTCInboundRtpStreamStats() noexcept;
          virtual ~RTCInboundRtpStreamStats() noexcept;

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

          // methods RTCInboundRtpStreamStats
          void wrapper_init_org_ortc_RTCInboundRtpStreamStats() noexcept override;
          void wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCInboundRtpStreamStats
          unsigned long get_packetsReceived() noexcept override;
          unsigned long long get_bytesReceived() noexcept override;
          unsigned long get_packetsLost() noexcept override;
          double get_jitter() noexcept override;
          double get_fractionLost() noexcept override;
          ::zsLib::Milliseconds get_endToEndDelay() noexcept override;

          static RTCInboundRtpStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

