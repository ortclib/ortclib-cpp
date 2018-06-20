
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
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCDtlsTransportStats
          virtual void wrapper_init_org_ortc_RTCDtlsTransportStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::RTCDtlsTransportStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCDtlsTransportStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCDtlsTransportStats
          virtual String get_localCertificateId() noexcept override;
          virtual String get_remoteCertificateId() noexcept override;

          static RTCDtlsTransportStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCDtlsTransportStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

