
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

          RTCRtpStreamStats() noexcept;
          virtual ~RTCRtpStreamStats() noexcept;

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCRtpStreamStats
          virtual void wrapper_init_org_ortc_RTCRtpStreamStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::RTCRtpStreamStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpStreamStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCRtpStreamStats
          virtual Optional< uint32_t > get_ssrc() noexcept override;
          virtual String get_associatedStatId() noexcept override;
          virtual bool get_isRemote() noexcept override;
          virtual String get_mediaType() noexcept override;
          virtual String get_mediaTrackId() noexcept override;
          virtual String get_transportId() noexcept override;
          virtual String get_codecId() noexcept override;
          virtual unsigned long get_firCount() noexcept override;
          virtual unsigned long get_pliCount() noexcept override;
          virtual unsigned long get_nackCount() noexcept override;
          virtual unsigned long get_sliCount() noexcept override;

          static RTCRtpStreamStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCRtpStreamStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

