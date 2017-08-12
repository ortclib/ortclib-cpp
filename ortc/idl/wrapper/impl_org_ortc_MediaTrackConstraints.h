
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackConstraints.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackConstraints : public wrapper::org::ortc::MediaTrackConstraints
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::TrackConstraints, NativeTrackConstraints);

          MediaTrackConstraintsWeakPtr thisWeak_;

          MediaTrackConstraints();
          virtual ~MediaTrackConstraints();

          // methods MediaTrackConstraintSet
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // methods MediaTrackConstraints
          virtual void wrapper_init_org_ortc_MediaTrackConstraints() override;
          virtual void wrapper_init_org_ortc_MediaTrackConstraints(wrapper::org::ortc::MediaTrackConstraintsPtr source) override;
          virtual void wrapper_init_org_ortc_MediaTrackConstraints(wrapper::org::ortc::JsonPtr json) override;

          static MediaTrackConstraintsPtr toWrapper(NativeTrackConstraintsPtr native);
          static MediaTrackConstraintsPtr toWrapper(const NativeTrackConstraints &native);
          static NativeTrackConstraintsPtr toNative(wrapper::org::ortc::MediaTrackConstraintsPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

