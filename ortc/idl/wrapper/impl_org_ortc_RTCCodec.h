
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCCodec.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCCodec : public wrapper::org::ortc::RTCCodec
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::Codec, NativeStats);
          RTCCodecWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCCodec();
          virtual ~RTCCodec();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCCodec
          virtual void wrapper_init_org_ortc_RTCCodec() override;
          virtual void wrapper_init_org_ortc_RTCCodec(wrapper::org::ortc::RTCCodecPtr source) override;
          virtual void wrapper_init_org_ortc_RTCCodec(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCCodec
          virtual Optional< uint8_t > get_payloadType() override;
          virtual String get_codec() override;
          virtual unsigned long get_clockRate() override;
          virtual Optional< unsigned long > get_channels() override;
          virtual String get_parameters() override;

          static RTCCodecPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCCodecPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

