

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_MediaElementMaker.h>

namespace Org {
  namespace WebRtc {



    [Windows::Foundation::Metadata::WebHostHiddenAttribute]
    public ref class MediaElementMaker sealed
    {
    internal:
      wrapper::org::webRtc::MediaElementMakerPtr native_;

      struct WrapperCreate {};
      MediaElementMaker(const WrapperCreate &) {}

      static MediaElementMaker^ ToCx(wrapper::org::webRtc::MediaElementMakerPtr value);
      static wrapper::org::webRtc::MediaElementMakerPtr FromCx(MediaElementMaker^ value);



    public:
      // ::org::webRtc::MediaElementMaker

      /// <summary>
      /// Creates an media element object from a media element type.
      /// </summary>
      static ::Org::WebRtc::MediaElement^ Bind(Windows::UI::Xaml::Controls::MediaElement^ element);
      /// <summary>
      /// Extracts a native media element from an media element object.
      /// </summary>
      static Windows::UI::Xaml::Controls::MediaElement^ Extract(::Org::WebRtc::MediaElement^ element);


    };

  } // namespace WebRtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER
