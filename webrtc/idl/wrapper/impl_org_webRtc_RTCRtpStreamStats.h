// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpStreamStats.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpStreamStats : public wrapper::org::webRtc::RTCRtpStreamStats
        {
          RTCRtpStreamStatsWeakPtr thisWeak_;

          RTCRtpStreamStats() noexcept;
          virtual ~RTCRtpStreamStats() noexcept;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::webRtc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCRtpStreamStats
          void wrapper_init_org_webRtc_RTCRtpStreamStats() noexcept override;
          void wrapper_init_org_webRtc_RTCRtpStreamStats(wrapper::org::webRtc::RTCRtpStreamStatsPtr source) noexcept override;

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
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

