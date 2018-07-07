
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/WebRtc/MediaElement.g.h"
#include <wrapper/generated/org_webRtc_MediaElement.h>

namespace winrt {
  namespace Org {
    namespace WebRtc {
      namespace implementation {


        /// <summary>
        /// MediaSource represents an object holder for a platform specific
        /// media element.
        /// </summary>
        struct MediaElement : MediaElementT<MediaElement>
        {
          // internal
          wrapper::org::webRtc::MediaElementPtr native_;

          struct WrapperCreate {};
          MediaElement(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElement > ToCppWinrtImpl(wrapper::org::webRtc::MediaElementPtr value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElement > ToCppWinrtImpl(Org::WebRtc::MediaElement const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElement > ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaElement > ToCppWinrtImpl(Org::WebRtc::IMediaElement const & value);

          // ToCppWinrt
          static Org::WebRtc::MediaElement ToCppWinrt(wrapper::org::webRtc::MediaElementPtr value);
          static Org::WebRtc::MediaElement ToCppWinrt(Org::WebRtc::MediaElement const & value);
          static Org::WebRtc::MediaElement ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value);
          static Org::WebRtc::MediaElement ToCppWinrt(Org::WebRtc::IMediaElement const & value);

          // ToCppWinrtInterface
          static Org::WebRtc::IMediaElement ToCppWinrtInterface(wrapper::org::webRtc::MediaElementPtr value);
          static Org::WebRtc::IMediaElement ToCppWinrtInterface(Org::WebRtc::MediaElement const & value);
          static Org::WebRtc::IMediaElement ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value);
          static Org::WebRtc::IMediaElement ToCppWinrtInterface(Org::WebRtc::IMediaElement const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::MediaElementPtr FromCppWinrt(wrapper::org::webRtc::MediaElementPtr value);
          static wrapper::org::webRtc::MediaElementPtr FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value);
          static wrapper::org::webRtc::MediaElementPtr FromCppWinrt(Org::WebRtc::MediaElement const & value);
          static wrapper::org::webRtc::MediaElementPtr FromCppWinrt(Org::WebRtc::IMediaElement const & value);




        public:
          /// <summary>
          /// Cast from Org::WebRtc::IMediaElement to MediaElement
          /// </summary>
          static Org::WebRtc::MediaElement CastFromIMediaElement(Org::WebRtc::IMediaElement const & value);

          // ::org::webRtc::MediaElement

#if 0
          /// <summary>
          /// Constructs a media element object from a native media element
          /// type.
          /// </summary>
          MediaElement(Windows::UI::Xaml::Controls::MediaElement const & element);

          /// <summary>
          /// Gets the platform specific media element.
          /// </summary>
          Windows::UI::Xaml::Controls::MediaElement Element();
#endif //0

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct MediaElement : MediaElementT<MediaElement, implementation::MediaElement>
        {
        };

      } // namespace factory_implementation

    } // namespace WebRtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
