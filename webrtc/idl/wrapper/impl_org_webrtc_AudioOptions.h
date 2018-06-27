
#pragma once

#include "types.h"
#include "generated/org_webrtc_AudioOptions.h"

#include "api/audio_options.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct AudioOptions : public wrapper::org::webrtc::AudioOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::AudioOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::AudioOptions, WrapperImplType);

          AudioOptionsWeakPtr thisWeak_;

          /*

          Optional< bool > echoCancellation {};
          Optional< bool > iosForceSoftwareAec_HACK {};
          Optional< bool > autoGinControl {};
          Optional< bool > noiseSuppression {};
          Optional< bool > highpassFilter {};
          Optional< bool > stereoWwapping {};
          Optional< int > audioJitterBufferMaxPackets {};
          Optional< bool > audioJitterBufferFastAccelerate {};
          Optional< bool > typingDetection {};
          Optional< bool > aecmGenerateComfortNoise {};
          Optional< bool > experimentalAgc {};
          Optional< bool > extendedFilterAec {};
          Optional< bool > delayAgnosticAec {};
          Optional< bool > experimentalNs {};
          Optional< bool > intelligibilityEnhancer {};
          Optional< bool > levelControl {};
          Optional< float > levelControlInitialPeakLevelDbfs {};
          Optional< bool > residualEchoDetector {};
          Optional< uint16_t > txAgcTargetDbov {};
          Optional< uint16_t > txAgcDigitalCompressionGain {};
          Optional< bool > txAgcLimiter {};
          Optional< bool > combinedAudioVideoBwe {};
          Optional< bool > audioNetworkAdaptor {};
          String audioNetworkAdaptorConfig {};
          */

          AudioOptions() noexcept;
          virtual ~AudioOptions() noexcept;
          void wrapper_init_org_webrtc_AudioOptions() noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

