
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCMediaStreamTrackStats.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCMediaStreamTrackStats : public wrapper::org::ortc::RTCMediaStreamTrackStats
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::MediaStreamTrackStats, NativeStats);
          RTCMediaStreamTrackStatsWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCMediaStreamTrackStats() noexcept;
          virtual ~RTCMediaStreamTrackStats() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCMediaStreamTrackStats
          void wrapper_init_org_ortc_RTCMediaStreamTrackStats() noexcept override;
          void wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCMediaStreamTrackStats
          String get_trackId() noexcept override;
          bool get_remoteSource() noexcept override;
          shared_ptr< list< uint32_t > > get_ssrcIds() noexcept override;
          unsigned long get_frameWidth() noexcept override;
          unsigned long get_frameHeight() noexcept override;
          double get_framesPerSecond() noexcept override;
          unsigned long get_framesSent() noexcept override;
          unsigned long get_framesReceived() noexcept override;
          unsigned long get_framesDecoded() noexcept override;
          unsigned long get_framesDropped() noexcept override;
          unsigned long get_framesCorrupted() noexcept override;
          double get_audioLevel() noexcept override;
          double get_echoReturnLoss() noexcept override;
          double get_echoReturnLossEnhancement() noexcept override;

          static RTCMediaStreamTrackStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

