
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

          RTCStatsTypeSet() noexcept;
          virtual ~RTCStatsTypeSet() noexcept;

          // methods RTCStatsTypeSet
          void wrapper_init_org_ortc_RTCStatsTypeSet() noexcept override;
          void wrapper_init_org_ortc_RTCStatsTypeSet(shared_ptr< set< wrapper::org::ortc::RTCStatsType > > values) noexcept override;
          bool hasStatType(wrapper::org::ortc::RTCStatsType type) noexcept override;

          static RTCStatsTypeSetPtr toWrapper(NativeStatsTypeSetPtr native) noexcept;
          static NativeStatsTypeSetPtr toNative(wrapper::org::ortc::RTCStatsTypeSetPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

