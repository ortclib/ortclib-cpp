
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/Webrtc/MediaElement.g.h"
#include <wrapper/generated/org_webrtc_MediaElement.h>

namespace winrt {
  namespace Org {
    namespace Webrtc {
      namespace implementation {


        /// <summary>
        /// MediaSource represents an object holder for a platform specific
        /// media element.
        /// </summary>
        struct MediaElement : MediaElementT<MediaElement>
        {
          // internal
          wrapper::org::webrtc::MediaElementPtr native_;

          struct WrapperCreate {};
          MediaElement(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Webrtc::implementation::MediaElement > ToCppWinrtImpl(wrapper::org::webrtc::MediaElementPtr value);
          static winrt::com_ptr< Org::Webrtc::implementation::MediaElement > ToCppWinrtImpl(Org::Webrtc::MediaElement const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::MediaElement > ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::MediaElement > ToCppWinrtImpl(Org::Webrtc::IMediaElement const & value);

          // ToCppWinrt
          static Org::Webrtc::MediaElement ToCppWinrt(wrapper::org::webrtc::MediaElementPtr value);
          static Org::Webrtc::MediaElement ToCppWinrt(Org::Webrtc::MediaElement const & value);
          static Org::Webrtc::MediaElement ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value);
          static Org::Webrtc::MediaElement ToCppWinrt(Org::Webrtc::IMediaElement const & value);

          // ToCppWinrtInterface
          static Org::Webrtc::IMediaElement ToCppWinrtInterface(wrapper::org::webrtc::MediaElementPtr value);
          static Org::Webrtc::IMediaElement ToCppWinrtInterface(Org::Webrtc::MediaElement const & value);
          static Org::Webrtc::IMediaElement ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value);
          static Org::Webrtc::IMediaElement ToCppWinrtInterface(Org::Webrtc::IMediaElement const & value);

          // FromCppWinrt
          static wrapper::org::webrtc::MediaElementPtr FromCppWinrt(wrapper::org::webrtc::MediaElementPtr value);
          static wrapper::org::webrtc::MediaElementPtr FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value);
          static wrapper::org::webrtc::MediaElementPtr FromCppWinrt(Org::Webrtc::MediaElement const & value);
          static wrapper::org::webrtc::MediaElementPtr FromCppWinrt(Org::Webrtc::IMediaElement const & value);




        public:
          MediaElement();
          /// <summary>
          /// Cast from Org::Webrtc::IMediaElement to MediaElement
          /// </summary>
          static Org::Webrtc::MediaElement CastFromIMediaElement(Org::Webrtc::IMediaElement const & value);

          /// <summary>
          /// Gets or sets the platform specific media element.
          /// </summary>
          Windows::Foundation::IInspectable Element();
          void Element(Windows::Foundation::IInspectable const & value);

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct MediaElement : MediaElementT<MediaElement, implementation::MediaElement>
        {
        };

      } // namespace factory_implementation

    } // namespace Webrtc
  } // namespace Org
} // namespace winrt

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
