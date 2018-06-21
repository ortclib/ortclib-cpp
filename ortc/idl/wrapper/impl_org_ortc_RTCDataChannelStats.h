
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDataChannelStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDataChannelStats : public wrapper::org::ortc::RTCDataChannelStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::DataChannelStats, NativeStats);
          RTCDataChannelStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCDataChannelStats() noexcept;
          virtual ~RTCDataChannelStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCDataChannelStats
          void wrapper_init_org_ortc_RTCDataChannelStats() noexcept override;
          void wrapper_init_org_ortc_RTCDataChannelStats(wrapper::org::ortc::RTCDataChannelStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCDataChannelStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCDataChannelStats
          String get_label() noexcept override;
          String get_protocol() noexcept override;
          long get_dataChannelId() noexcept override;
          wrapper::org::ortc::RTCDataChannelState get_state() noexcept override;
          unsigned long get_messagesSent() noexcept override;
          unsigned long long get_bytesSent() noexcept override;
          unsigned long get_messagesReceived() noexcept override;
          unsigned long long get_bytesReceived() noexcept override;

          static RTCDataChannelStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCDataChannelStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

