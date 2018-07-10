

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_MediaElement.h>

namespace Org {
  namespace WebRtc {



    /// <summary>
    /// MediaSource represents an object holder for a platform specific media
    /// element.
    /// </summary>
    public ref class MediaElement sealed
    {
    internal:
      wrapper::org::webRtc::MediaElementPtr native_;

      struct WrapperCreate {};
      MediaElement(const WrapperCreate &) {}

      static MediaElement^ ToCx(wrapper::org::webRtc::MediaElementPtr value);
      static wrapper::org::webRtc::MediaElementPtr FromCx(MediaElement^ value);



    public:
      // ::org::webRtc::MediaElement

#if 0
      /// <summary>
      /// Constructs a media element object from a native media element type.
      /// </summary>
      [Windows::Foundation::Metadata::DefaultOverloadAttribute, Windows::Foundation::Metadata::OverloadAttribute("MediaElementWithNativeQueue")]
      MediaElement(Platform::Object^ element);

      /// <summary>
      /// Gets the platform specific media element.
      /// </summary>
      property Platform::Object^ Element { Platform::Object^ get(); }
#endif //0

    };

  } // namespace WebRtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
