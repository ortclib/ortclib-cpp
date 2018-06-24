
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

          RTCDtlsTransportStats() noexcept;
          virtual ~RTCDtlsTransportStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCDtlsTransportStats
          void wrapper_init_org_ortc_RTCDtlsTransportStats() noexcept override;
          void wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::RTCDtlsTransportStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCDtlsTransportStats
          String get_localCertificateId() noexcept override;
          String get_remoteCertificateId() noexcept override;

          static RTCDtlsTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCDtlsTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

