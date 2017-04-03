
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

          RTCMediaStreamTrackStats();
          virtual ~RTCMediaStreamTrackStats();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCMediaStreamTrackStats
          virtual void wrapper_init_org_ortc_RTCMediaStreamTrackStats() override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCMediaStreamTrackStats(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCMediaStreamTrackStats
          virtual String get_trackId() override;
          virtual bool get_remoteSource() override;
          virtual shared_ptr< list< uint32_t > > get_ssrcIds() override;
          virtual unsigned long get_frameWidth() override;
          virtual unsigned long get_frameHeight() override;
          virtual double get_framesPerSecond() override;
          virtual unsigned long get_framesSent() override;
          virtual unsigned long get_framesReceived() override;
          virtual unsigned long get_framesDecoded() override;
          virtual unsigned long get_framesDropped() override;
          virtual unsigned long get_framesCorrupted() override;
          virtual double get_audioLevel() override;
          virtual double get_echoReturnLoss() override;
          virtual double get_echoReturnLossEnhancement() override;

          static RTCMediaStreamTrackStatsPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCMediaStreamTrackStatsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

