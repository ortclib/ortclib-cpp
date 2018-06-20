
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

          RTCMediaStreamStats() noexcept;
          virtual ~RTCMediaStreamStats() noexcept;

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCMediaStreamStats
          virtual void wrapper_init_org_ortc_RTCMediaStreamStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::RTCMediaStreamStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCMediaStreamStats
          virtual String get_streamId() noexcept override;
          virtual shared_ptr< list< String > > get_trackIds() noexcept override;

          static RTCMediaStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

