
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtlsTransportStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtlsTransportStats : public wrapper::org::ortc::RTCDtlsTransportStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::DTLSTransportStats, NativeStats);
          RTCDtlsTransportStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCDtlsTransportStats();
          virtual ~RTCDtlsTransportStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCDtlsTransportStats
          virtual void wrapper_init_org_ortc_RTCDtlsTransportStats() override;
          virtual void wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::RTCDtlsTransportStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCDtlsTransportStats
          virtual String get_localCertificateId() override;
          virtual String get_remoteCertificateId() override;

          static RTCDtlsTransportStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCDtlsTransportStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

