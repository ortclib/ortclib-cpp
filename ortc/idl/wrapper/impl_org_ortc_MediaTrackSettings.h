
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackSettings.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackSettings : public wrapper::org::ortc::MediaTrackSettings
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::Settings, Settings);

          MediaTrackSettingsWeakPtr thisWeak_;

          MediaTrackSettings();
          virtual ~MediaTrackSettings();

          // methods MediaTrackSettings
          virtual void wrapper_init_org_ortc_MediaTrackSettings() override;
          virtual void wrapper_init_org_ortc_MediaTrackSettings(wrapper::org::ortc::MediaTrackSettingsPtr source) override;
          virtual void wrapper_init_org_ortc_MediaTrackSettings(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static MediaTrackSettingsPtr toWrapper(SettingsPtr native);
          static SettingsPtr toNative(MediaTrackSettingsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

