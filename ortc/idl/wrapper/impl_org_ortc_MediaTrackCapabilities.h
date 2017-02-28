
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackCapabilities.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackCapabilities : public wrapper::org::ortc::MediaTrackCapabilities
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::Capabilities, NativeCapabilities);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICapabilities::CapabilityString, NativeCapabilityString);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICapabilities::CapabilityBoolean, NativeCapabilityBoolean);
          
          MediaTrackCapabilitiesWeakPtr thisWeak_;

          MediaTrackCapabilities();
          virtual ~MediaTrackCapabilities();

          // methods MediaTrackCapabilities
          virtual void wrapper_init_org_ortc_MediaTrackCapabilities() override;
          virtual void wrapper_init_org_ortc_MediaTrackCapabilities(wrapper::org::ortc::MediaTrackCapabilitiesPtr source) override;
          virtual void wrapper_init_org_ortc_MediaTrackCapabilities(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static MediaTrackCapabilitiesPtr toWrapper(NativeCapabilitiesPtr native);
          static MediaTrackCapabilitiesPtr toWrapper(const NativeCapabilities &native);
          static NativeCapabilitiesPtr toNative(wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

