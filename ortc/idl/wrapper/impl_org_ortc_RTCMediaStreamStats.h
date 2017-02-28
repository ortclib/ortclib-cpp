
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCMediaStreamStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCMediaStreamStats : public wrapper::org::ortc::RTCMediaStreamStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::MediaStreamStats, NativeStats);
          RTCMediaStreamStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCMediaStreamStats();
          virtual ~RTCMediaStreamStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCMediaStreamStats
          virtual void wrapper_init_org_ortc_RTCMediaStreamStats() override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::RTCMediaStreamStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCMediaStreamStats
          virtual String get_streamId() override;
          virtual shared_ptr< list< String > > get_trackIds() override;

          static RTCMediaStreamStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

