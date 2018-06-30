// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCDataChannelStats.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCDataChannelStats : public wrapper::org::webRtc::RTCDataChannelStats
        {
          RTCDataChannelStatsWeakPtr thisWeak_;

          RTCDataChannelStats() noexcept;
          virtual ~RTCDataChannelStats() noexcept;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::webRtc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCDataChannelStats
          void wrapper_init_org_webRtc_RTCDataChannelStats() noexcept override;
          void wrapper_init_org_webRtc_RTCDataChannelStats(wrapper::org::webRtc::RTCDataChannelStatsPtr source) noexcept override;

          // properties RTCDataChannelStats
          String get_label() noexcept override;
          String get_protocol() noexcept override;
          long get_dataChannelId() noexcept override;
          wrapper::org::webRtc::RTCDataChannelState get_state() noexcept override;
          unsigned long get_messagesSent() noexcept override;
          unsigned long long get_bytesSent() noexcept override;
          unsigned long get_messagesReceived() noexcept override;
          unsigned long long get_bytesReceived() noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

