
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
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // properties RTCRtpStreamStats
          virtual Optional< uint32_t > get_ssrc() noexcept override;
          virtual String get_associatedStatId() noexcept override;
          virtual bool get_isRemote() noexcept override;
          virtual String get_mediaType() noexcept override;
          virtual String get_mediaTrackId() noexcept override;
          virtual String get_transportId() noexcept override;
          virtual String get_codecId() noexcept override;
          virtual unsigned long get_firCount() noexcept override;
          virtual unsigned long get_pliCount() noexcept override;
          virtual unsigned long get_nackCount() noexcept override;
          virtual unsigned long get_sliCount() noexcept override;

          // methods RTCInboundRtpStreamStats
          virtual void wrapper_init_org_ortc_RTCInboundRtpStreamStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCInboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCInboundRtpStreamStats
          virtual unsigned long get_packetsReceived() noexcept override;
          virtual unsigned long long get_bytesReceived() noexcept override;
          virtual unsigned long get_packetsLost() noexcept override;
          virtual double get_jitter() noexcept override;
          virtual double get_fractionLost() noexcept override;
          virtual ::zsLib::Milliseconds get_endToEndDelay() noexcept override;

          static RTCInboundRtpStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCInboundRtpStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

