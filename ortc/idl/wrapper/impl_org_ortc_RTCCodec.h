
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

          RTCCodec() noexcept;
          virtual ~RTCCodec() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCCodec
          void wrapper_init_org_ortc_RTCCodec() noexcept override;
          void wrapper_init_org_ortc_RTCCodec(wrapper::org::ortc::RTCCodecPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCCodec(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCCodec
          Optional< uint8_t > get_payloadType() noexcept override;
          String get_codec() noexcept override;
          unsigned long get_clockRate() noexcept override;
          Optional< unsigned long > get_channels() noexcept override;
          String get_parameters() noexcept override;

          static RTCCodecPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCCodecPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

