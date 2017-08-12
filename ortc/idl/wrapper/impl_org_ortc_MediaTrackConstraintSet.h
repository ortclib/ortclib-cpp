
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackConstraintSet.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackConstraintSet : public wrapper::org::ortc::MediaTrackConstraintSet
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::ConstraintSet, NativeConstraintSet);

          MediaTrackConstraintSetWeakPtr thisWeak_;

          MediaTrackConstraintSet();
          virtual ~MediaTrackConstraintSet();

          // methods MediaTrackConstraintSet
          virtual void wrapper_init_org_ortc_MediaTrackConstraintSet() override;
          virtual void wrapper_init_org_ortc_MediaTrackConstraintSet(wrapper::org::ortc::MediaTrackConstraintSetPtr source) override;
          virtual void wrapper_init_org_ortc_MediaTrackConstraintSet(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static MediaTrackConstraintSetPtr toWrapper(NativeConstraintSetPtr native);
          static MediaTrackConstraintSetPtr toWrapper(const NativeConstraintSet &native);
          static NativeConstraintSetPtr toNative(wrapper::org::ortc::MediaTrackConstraintSetPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

