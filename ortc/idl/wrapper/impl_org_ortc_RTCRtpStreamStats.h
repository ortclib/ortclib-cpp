
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpStreamStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpStreamStats : public wrapper::org::ortc::RTCRtpStreamStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::RTPStreamStats, NativeStats);
          RTCRtpStreamStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCRtpStreamStats() noexcept;
          virtual ~RTCRtpStreamStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCRtpStreamStats
          void wrapper_init_org_ortc_RTCRtpStreamStats() noexcept override;
          void wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::RTCRtpStreamStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

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

          static RTCRtpStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCRtpStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

