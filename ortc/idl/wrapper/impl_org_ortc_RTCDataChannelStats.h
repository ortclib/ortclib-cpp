
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

          RTCDataChannelStats();
          virtual ~RTCDataChannelStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCDataChannelStats
          virtual void wrapper_init_org_ortc_RTCDataChannelStats() override;
          virtual void wrapper_init_org_ortc_RTCDataChannelStats(wrapper::org::ortc::RTCDataChannelStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCDataChannelStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCDataChannelStats
          virtual String get_label() override;
          virtual String get_protocol() override;
          virtual long get_dataChannelId() override;
          virtual wrapper::org::ortc::RTCDataChannelState get_state() override;
          virtual unsigned long get_messagesSent() override;
          virtual unsigned long long get_bytesSent() override;
          virtual unsigned long get_messagesReceived() override;
          virtual unsigned long long get_bytesReceived() override;

          static RTCDataChannelStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCDataChannelStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

