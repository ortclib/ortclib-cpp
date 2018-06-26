// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCMediaStreamStats.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCMediaStreamStats : public wrapper::org::webrtc::RTCMediaStreamStats
        {
          RTCMediaStreamStatsWeakPtr thisWeak_;

          RTCMediaStreamStats() noexcept;
          virtual ~RTCMediaStreamStats() noexcept;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::webrtc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCMediaStreamStats
          void wrapper_init_org_webrtc_RTCMediaStreamStats() noexcept override;
          void wrapper_init_org_webrtc_RTCMediaStreamStats(wrapper::org::webrtc::RTCMediaStreamStatsPtr source) noexcept override;

          // properties RTCMediaStreamStats
          String get_streamId() noexcept override;
          shared_ptr< list< String > > get_trackIds() noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper
