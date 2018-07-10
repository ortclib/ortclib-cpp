

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_MediaSource.h>

namespace Org {
  namespace WebRtc {



    /// <summary>
    /// MediaSource represents an object holder for a platform specific media
    /// source.
    /// </summary>
    public ref class MediaSource sealed
    {
    internal:
      wrapper::org::webRtc::MediaSourcePtr native_;

      struct WrapperCreate {};
      MediaSource(const WrapperCreate &) {}

      static MediaSource^ ToCx(wrapper::org::webRtc::MediaSourcePtr value);
      static wrapper::org::webRtc::MediaSourcePtr FromCx(MediaSource^ value);



    public:
      // ::org::webRtc::MediaSource

      /// <summary>
      /// Constructs a media source object from a native media source type.
      /// </summary>
      [Windows::Foundation::Metadata::DefaultOverloadAttribute, Windows::Foundation::Metadata::OverloadAttribute("MediaSourceWithNativeSource")]
      MediaSource(Windows::Media::Core::IMediaSource^ source);

      /// <summary>
      /// Gets the platform specific media source.
      /// </summary>
      property Windows::Media::Core::IMediaSource^ Source { Windows::Media::Core::IMediaSource^ get(); }

    };

  } // namespace WebRtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
