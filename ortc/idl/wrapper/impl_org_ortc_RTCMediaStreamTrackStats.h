
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
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() noexcept override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          virtual String get_statsTypeOther() noexcept override;
          virtual String get_id() noexcept override;

          // methods RTCMediaStreamTrackStats
          virtual void wrapper_init_org_ortc_RTCMediaStreamTrackStats() noexcept override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCMediaStreamTrackStats
          virtual String get_trackId() noexcept override;
          virtual bool get_remoteSource() noexcept override;
          virtual shared_ptr< list< uint32_t > > get_ssrcIds() noexcept override;
          virtual unsigned long get_frameWidth() noexcept override;
          virtual unsigned long get_frameHeight() noexcept override;
          virtual double get_framesPerSecond() noexcept override;
          virtual unsigned long get_framesSent() noexcept override;
          virtual unsigned long get_framesReceived() noexcept override;
          virtual unsigned long get_framesDecoded() noexcept override;
          virtual unsigned long get_framesDropped() noexcept override;
          virtual unsigned long get_framesCorrupted() noexcept override;
          virtual double get_audioLevel() noexcept override;
          virtual double get_echoReturnLoss() noexcept override;
          virtual double get_echoReturnLossEnhancement() noexcept override;

          static RTCMediaStreamTrackStatsPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

