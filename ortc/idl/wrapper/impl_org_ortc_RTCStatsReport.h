
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCStatsReport.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCStatsReport : public wrapper::org::ortc::RTCStatsReport
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReport, NativeStatsReport);

          RTCStatsReportWeakPtr thisWeak_;
          NativeStatsReportPtr native_;

          RTCStatsReport();
          virtual ~RTCStatsReport();

          // methods RTCStatsReport
          virtual wrapper::org::ortc::RTCStatsPtr getStats(String id) override;

          // properties RTCStatsReport
          virtual uint64_t get_objectId() override;
          virtual shared_ptr< list< String > > get_statsIds() override;

          static RTCStatsReportPtr toWrapper(NativeStatsReportPtr native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

