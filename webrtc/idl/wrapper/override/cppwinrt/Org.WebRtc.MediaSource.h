
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/WebRtc/MediaSource.g.h"
#include <wrapper/generated/org_webRtc_MediaSource.h>

namespace winrt {
  namespace Org {
    namespace WebRtc {
      namespace implementation {


        /// <summary>
        /// MediaSource represents an object holder for a platform specific
        /// media source.
        /// </summary>
        struct MediaSource : MediaSourceT<MediaSource>
        {
          // internal
          wrapper::org::webRtc::MediaSourcePtr native_;

          struct WrapperCreate {};
          MediaSource(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::WebRtc::implementation::MediaSource > ToCppWinrtImpl(wrapper::org::webRtc::MediaSourcePtr value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaSource > ToCppWinrtImpl(Org::WebRtc::MediaSource const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaSource > ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::MediaSource > ToCppWinrtImpl(Org::WebRtc::IMediaSource const & value);

          // ToCppWinrt
          static Org::WebRtc::MediaSource ToCppWinrt(wrapper::org::webRtc::MediaSourcePtr value);
          static Org::WebRtc::MediaSource ToCppWinrt(Org::WebRtc::MediaSource const & value);
          static Org::WebRtc::MediaSource ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value);
          static Org::WebRtc::MediaSource ToCppWinrt(Org::WebRtc::IMediaSource const & value);

          // ToCppWinrtInterface
          static Org::WebRtc::IMediaSource ToCppWinrtInterface(wrapper::org::webRtc::MediaSourcePtr value);
          static Org::WebRtc::IMediaSource ToCppWinrtInterface(Org::WebRtc::MediaSource const & value);
          static Org::WebRtc::IMediaSource ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value);
          static Org::WebRtc::IMediaSource ToCppWinrtInterface(Org::WebRtc::IMediaSource const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::MediaSourcePtr FromCppWinrt(wrapper::org::webRtc::MediaSourcePtr value);
          static wrapper::org::webRtc::MediaSourcePtr FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value);
          static wrapper::org::webRtc::MediaSourcePtr FromCppWinrt(Org::WebRtc::MediaSource const & value);
          static wrapper::org::webRtc::MediaSourcePtr FromCppWinrt(Org::WebRtc::IMediaSource const & value);




        public:
          MediaSource();
          /// <summary>
          /// Cast from Org::WebRtc::IMediaSource to MediaSource
          /// </summary>
          static Org::WebRtc::MediaSource CastFromIMediaSource(Org::WebRtc::IMediaSource const & value);

          /// <summary>
          /// Gets or sets the platform specific media source.
          /// </summary>
          Windows::Media::Core::IMediaSource Source();
          void Source(Windows::Media::Core::IMediaSource const & value);
          

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct MediaSource : MediaSourceT<MediaSource, implementation::MediaSource>
        {
        };

      } // namespace factory_implementation

    } // namespace WebRtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
