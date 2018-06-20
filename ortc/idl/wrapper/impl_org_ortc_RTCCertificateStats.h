
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCCertificateStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCCertificateStats : public wrapper::org::ortc::RTCCertificateStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::CertificateStats, NativeStats);
          RTCCertificateStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCCertificateStats() noexcept;
          virtual ~RTCCertificateStats() noexcept;

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCCertificateStats
          virtual void wrapper_init_org_ortc_RTCCertificateStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCCertificateStats(wrapper::org::ortc::RTCCertificateStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCCertificateStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCCertificateStats
          virtual String get_fingerprint() noexcept override;
          virtual String get_fingerprintAlgorithm() noexcept override;
          virtual String get_base64Certificate() noexcept override;
          virtual String get_issuerCertificateId() noexcept override;

          static RTCCertificateStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCCertificateStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

