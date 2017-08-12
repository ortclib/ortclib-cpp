
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpStreamStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpStreamStats : public wrapper::org::ortc::RTCRtpStreamStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::RTPStreamStats, NativeStats);
          RTCRtpStreamStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCRtpStreamStats();
          virtual ~RTCRtpStreamStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCRtpStreamStats
          virtual void wrapper_init_org_ortc_RTCRtpStreamStats() override;
          virtual void wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::RTCRtpStreamStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCRtpStreamStats
          virtual Optional< uint32_t > get_ssrc() override;
          virtual String get_associatedStatId() override;
          virtual bool get_isRemote() override;
          virtual String get_mediaType() override;
          virtual String get_mediaTrackId() override;
          virtual String get_transportId() override;
          virtual String get_codecId() override;
          virtual unsigned long get_firCount() override;
          virtual unsigned long get_pliCount() override;
          virtual unsigned long get_nackCount() override;
          virtual unsigned long get_sliCount() override;

          static RTCRtpStreamStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCRtpStreamStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

