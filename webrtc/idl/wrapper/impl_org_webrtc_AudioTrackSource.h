
#pragma once

#include "types.h"
#include "generated/org_webrtc_AudioTrackSource.h"


#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/video/video_frame.h"
#include "api/mediastreaminterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct AudioTrackSource : public wrapper::org::webrtc::AudioTrackSource
        {
          AudioTrackSourceWeakPtr thisWeak_;

          AudioTrackSource() noexcept;
          virtual ~AudioTrackSource() noexcept;

          // properties MediaTrackSource
          wrapper::org::webrtc::MediaSourceState get_state() noexcept override;
          bool get_remote() noexcept override;
          void wrapper_init_org_webrtc_AudioTrackSource() noexcept override;

          // properties AudioTrackSource
          void set_volume(double value) noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

