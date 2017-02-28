
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCStatsTypeSet.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCStatsTypeSet : public wrapper::org::ortc::RTCStatsTypeSet
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::StatsTypeSet, NativeStatsTypeSet);
          RTCStatsTypeSetWeakPtr thisWeak_;
          NativeStatsTypeSetPtr native_;

          RTCStatsTypeSet();
          virtual ~RTCStatsTypeSet();

          // methods RTCStatsTypeSet
          virtual void wrapper_init_org_ortc_RTCStatsTypeSet() override;
          virtual void wrapper_init_org_ortc_RTCStatsTypeSet(shared_ptr< set< wrapper::org::ortc::RTCStatsType > > values) override;
          virtual bool hasStatType(wrapper::org::ortc::RTCStatsType type) override;

          static RTCStatsTypeSetPtr toWrapper(NativeStatsTypeSetPtr native);
          static NativeStatsTypeSetPtr toNative(wrapper::org::ortc::RTCStatsTypeSetPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

