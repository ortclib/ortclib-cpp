
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
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCMediaStreamStats
          void wrapper_init_org_ortc_RTCMediaStreamStats() noexcept override;
          void wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::RTCMediaStreamStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCMediaStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCMediaStreamStats
          String get_streamId() noexcept override;
          shared_ptr< list< String > > get_trackIds() noexcept override;

          static RTCMediaStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCMediaStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

