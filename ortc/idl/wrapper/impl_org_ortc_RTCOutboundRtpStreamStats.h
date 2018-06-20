
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

          // methods RTCOutboundRtpStreamStats
          virtual void wrapper_init_org_ortc_RTCOutboundRtpStreamStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCOutboundRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCOutboundRtpStreamStats
          virtual unsigned long get_packetsSent() noexcept override;
          virtual unsigned long long get_bytesSent() noexcept override;
          virtual double get_targetBitrate() noexcept override;
          virtual double get_roundTripTime() noexcept override;

          static RTCOutboundRtpStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCOutboundRtpStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

