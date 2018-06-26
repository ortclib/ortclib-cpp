
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/Webrtc/MediaSource.g.h"
#include <wrapper/generated/org_webrtc_MediaSource.h>

namespace winrt {
  namespace Org {
    namespace Webrtc {
      namespace implementation {


        /// <summary>
        /// MediaSource represents an object holder for a platform specific
        /// media source.
        /// </summary>
        struct MediaSource : MediaSourceT<MediaSource>
        {
          // internal
          wrapper::org::webrtc::MediaSourcePtr native_;

          struct WrapperCreate {};
          MediaSource(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Webrtc::implementation::MediaSource > ToCppWinrtImpl(wrapper::org::webrtc::MediaSourcePtr value);
          static winrt::com_ptr< Org::Webrtc::implementation::MediaSource > ToCppWinrtImpl(Org::Webrtc::MediaSource const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::MediaSource > ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::MediaSource > ToCppWinrtImpl(Org::Webrtc::IMediaSource const & value);

          // ToCppWinrt
          static Org::Webrtc::MediaSource ToCppWinrt(wrapper::org::webrtc::MediaSourcePtr value);
          static Org::Webrtc::MediaSource ToCppWinrt(Org::Webrtc::MediaSource const & value);
          static Org::Webrtc::MediaSource ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value);
          static Org::Webrtc::MediaSource ToCppWinrt(Org::Webrtc::IMediaSource const & value);

          // ToCppWinrtInterface
          static Org::Webrtc::IMediaSource ToCppWinrtInterface(wrapper::org::webrtc::MediaSourcePtr value);
          static Org::Webrtc::IMediaSource ToCppWinrtInterface(Org::Webrtc::MediaSource const & value);
          static Org::Webrtc::IMediaSource ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value);
          static Org::Webrtc::IMediaSource ToCppWinrtInterface(Org::Webrtc::IMediaSource const & value);

          // FromCppWinrt
          static wrapper::org::webrtc::MediaSourcePtr FromCppWinrt(wrapper::org::webrtc::MediaSourcePtr value);
          static wrapper::org::webrtc::MediaSourcePtr FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value);
          static wrapper::org::webrtc::MediaSourcePtr FromCppWinrt(Org::Webrtc::MediaSource const & value);
          static wrapper::org::webrtc::MediaSourcePtr FromCppWinrt(Org::Webrtc::IMediaSource const & value);




        public:
          MediaSource();
          /// <summary>
          /// Cast from Org::Webrtc::IMediaSource to MediaSource
          /// </summary>
          static Org::Webrtc::MediaSource CastFromIMediaSource(Org::Webrtc::IMediaSource const & value);

          /// <summary>
          /// Gets or sets the platform specific media source.
          /// </summary>
          Windows::Foundation::IInspectable Source();
          void Source(Windows::Foundation::IInspectable const & value);
          /// <summary>
          /// Gets or sets the media track associated to the media source.
          /// </summary>
          Windows::Foundation::IInspectable Track();

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct MediaSource : MediaSourceT<MediaSource, implementation::MediaSource>
        {
        };

      } // namespace factory_implementation

    } // namespace Webrtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
