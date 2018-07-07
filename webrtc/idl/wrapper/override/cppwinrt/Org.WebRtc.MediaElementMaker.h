
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/WebRtc/MediaElementMaker.g.h"
#include <wrapper/generated/org_webRtc_MediaElementMaker.h>

namespace winrt {
  namespace Org {
    namespace WebRtc {
      namespace implementation {


        struct MediaElementMaker : MediaElementMakerT<MediaElementMaker>
        {
          // internal
          wrapper::org::webRtc::MediaElementMakerPtr native_;

          struct WrapperCreate {};
          MediaElementMaker(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > ToCppWinrtImpl(wrapper::org::webRtc::MediaElementMakerPtr value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > ToCppWinrtImpl(Org::WebRtc::MediaElementMaker const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > ToCppWinrtImpl(Org::WebRtc::IMediaElementMaker const & value);

          // ToCppWinrt
          static Org::WebRtc::MediaElementMaker ToCppWinrt(wrapper::org::webRtc::MediaElementMakerPtr value);
          static Org::WebRtc::MediaElementMaker ToCppWinrt(Org::WebRtc::MediaElementMaker const & value);
          static Org::WebRtc::MediaElementMaker ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value);
          static Org::WebRtc::MediaElementMaker ToCppWinrt(Org::WebRtc::IMediaElementMaker const & value);

          // ToCppWinrtInterface
          static Org::WebRtc::IMediaElementMaker ToCppWinrtInterface(wrapper::org::webRtc::MediaElementMakerPtr value);
          static Org::WebRtc::IMediaElementMaker ToCppWinrtInterface(Org::WebRtc::MediaElementMaker const & value);
          static Org::WebRtc::IMediaElementMaker ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value);
          static Org::WebRtc::IMediaElementMaker ToCppWinrtInterface(Org::WebRtc::IMediaElementMaker const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::MediaElementMakerPtr FromCppWinrt(wrapper::org::webRtc::MediaElementMakerPtr value);
          static wrapper::org::webRtc::MediaElementMakerPtr FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value);
          static wrapper::org::webRtc::MediaElementMakerPtr FromCppWinrt(Org::WebRtc::MediaElementMaker const & value);
          static wrapper::org::webRtc::MediaElementMakerPtr FromCppWinrt(Org::WebRtc::IMediaElementMaker const & value);




        public:
          /// <summary>
          /// Cast from Org::WebRtc::IMediaElementMaker to MediaElementMaker
          /// </summary>
          static Org::WebRtc::MediaElementMaker CastFromIMediaElementMaker(Org::WebRtc::IMediaElementMaker const & value);

          // ::org::webRtc::MediaElementMaker

          /// <summary>
          /// Creates an media element object from a media element type.
          /// </summary>
          static Org::WebRtc::IMediaElement Bind(Windows::UI::Xaml::Controls::MediaElement const & element);
          /// <summary>
          /// Extracts a native media element from an media element object.
          /// </summary>
          static Windows::UI::Xaml::Controls::MediaElement Extract(Org::WebRtc::IMediaElement const & element);


        };

      } // namepsace implementation

      namespace factory_implementation {

        struct MediaElementMaker : MediaElementMakerT<MediaElementMaker, implementation::MediaElementMaker>
        {
        };

      } // namespace factory_implementation

    } // namespace WebRtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER
