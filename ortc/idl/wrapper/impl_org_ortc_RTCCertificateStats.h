
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

          RTCCertificateStats();
          virtual ~RTCCertificateStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCCertificateStats
          virtual void wrapper_init_org_ortc_RTCCertificateStats() override;
          virtual void wrapper_init_org_ortc_RTCCertificateStats(wrapper::org::ortc::RTCCertificateStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCCertificateStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCCertificateStats
          virtual String get_fingerprint() override;
          virtual String get_fingerprintAlgorithm() override;
          virtual String get_base64Certificate() override;
          virtual String get_issuerCertificateId() override;

          static RTCCertificateStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCCertificateStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

